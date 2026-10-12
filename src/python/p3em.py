#   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
#   Copyright(C) 2025 Salvatore Cielo, Leibniz-Rechenzentrum
#   Copyright(C) 2025 Ivan Pribec, Leibniz-Rechenzentrum
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
#  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
#  language governing permissions and limitations under the License.

"""
Drop-in replacement for time.time(), controlled by the same environment variables as the C/C++/Fortran versions:

    P3EM_ENABLED=1   activates monitoring; otherwise now() returns wall-clock seconds.
    P3EM_CPU         path to a CPU meter script (optional)
    P3EM_GPU         path to a GPU meter script (optional)

Usage:
    import p3em
    p3em.init()
    t0 = p3em.now()   # energy if enabled, seconds otherwise
"""

import atexit
import os
import signal
import subprocess
import sys
import threading
import time
from typing import Optional


def _mpi_local_rank_on_node() -> int:
    for env_var in ("OMPI_COMM_WORLD_LOCAL_RANK",
                    "MV2_COMM_WORLD_LOCAL_RANK",
                    "MPI_LOCALRANKID",
                    "SLURM_LOCALID",
                    "PMI_LOCAL_RANK"):
        value = os.environ.get(env_var)
        if value is not None:
            return int(value)
    return 0


def _check_enabled() -> bool:
    return os.environ.get("P3EM_ENABLED") == "1"


class ScriptMonitor:
    """
    Launches a meter script and keeps the last number printed on its latest output line.
    Only node-local rank 0 runs the script; other ranks always read 0.
    """

    def __init__(self, script_path: str, local_rank: int = 0):
        self._script_path = script_path
        self._shm_rank = local_rank
        self._process = None  # type: Optional[subprocess.Popen]
        self._latest_value = -42.0
        self._thread = None  # type: Optional[threading.Thread]
        if not self._shm_rank:
            self._launch_and_monitor()

    def _monitor_output(self):
        with self._process.stdout:
            for line in iter(self._process.stdout.readline, b''):
                try:
                    words = line.decode().split()
                    if words:
                        self._latest_value = float(words[-1])
                except (ValueError, UnicodeDecodeError):
                    continue

    def _launch_and_monitor(self):
        self._process = subprocess.Popen(
            ["stdbuf", "-oL", "-eL", self._script_path],
            stdout=subprocess.PIPE,
            start_new_session=True,
        )
        self._thread = threading.Thread(target=self._monitor_output, daemon=True)
        self._thread.start()
        while self._latest_value < 0:
            if self._process.poll() is not None:
                print("[p3em] ERROR: meter script exited before producing a value: " + self._script_path, file=sys.stderr)
                sys.exit(1)
            time.sleep(0.01)

    def get_latest_value(self) -> float:
        return 0.0 if self._shm_rank else self._latest_value

    def stop(self):
        if self._process:
            try:
                os.killpg(self._process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
            self._process.wait()
            self._process = None
        if self._thread:
            self._thread.join(timeout=1)
            self._thread = None


enabled = False
cpu_reader = None  # type: Optional[ScriptMonitor]
gpu_reader = None  # type: Optional[ScriptMonitor]


def init():
    global enabled, cpu_reader, gpu_reader
    stop()
    enabled = _check_enabled()
    if not enabled:
        return
    rank = _mpi_local_rank_on_node()
    str_gpu = os.environ.get("P3EM_GPU")
    str_cpu = os.environ.get("P3EM_CPU")

    # Abort if enabled but no device specified
    if not str_cpu and not str_gpu:
        print("[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined.", file=sys.stderr)
        sys.exit(1)

    if str_gpu:
        print("[p3em] GPU monitoring enabled: " + str_gpu, file=sys.stderr)
        gpu_reader = ScriptMonitor(str_gpu, rank)

    if str_cpu:
        print("[p3em] CPU monitoring enabled: " + str_cpu, file=sys.stderr)
        cpu_reader = ScriptMonitor(str_cpu, rank)


def now() -> float:
    if not enabled:
        return time.time()
    total = 0.0
    if cpu_reader is not None: total += cpu_reader.get_latest_value()
    if gpu_reader is not None: total += gpu_reader.get_latest_value()
    return total


def stop():
    global cpu_reader, gpu_reader
    for reader in (cpu_reader, gpu_reader):
        if reader is not None:
            reader.stop()
    cpu_reader = gpu_reader = None


atexit.register(stop)
