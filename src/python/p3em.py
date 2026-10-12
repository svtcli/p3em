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

"""Environment-variable driven Python interface to p3em.

The module mirrors the C/C++ interface: :func:`init` reads the ``P3EM_*``
environment variables and starts the requested meter scripts, while
:func:`now` returns either the sum of the latest meter values (when enabled)
or wall-clock seconds (when disabled).  Instrumenting an existing timer is
therefore just a matter of calling :func:`now` instead of ``time.time``.
"""

import os
import subprocess
import sys
import threading
import time
from typing import Dict, Optional

_LOCAL_RANK_VARS = (
    "OMPI_COMM_WORLD_LOCAL_RANK",
    "MV2_COMM_WORLD_LOCAL_RANK",
    "MPI_LOCALRANKID",
    "SLURM_LOCALID",
    "PMI_LOCAL_RANK",
)

enabled: bool = False
_readers: Dict[str, "ScriptMonitor"] = {}
_initialized: bool = False


def _local_rank() -> int:
    """Return the rank of this process on its node (0 if unknown)."""
    for var in _LOCAL_RANK_VARS:
        value = os.environ.get(var)
        if value is not None:
            try:
                return int(value)
            except ValueError:
                return 0
    return 0


class ScriptMonitor:
    """Launch a meter script and track the latest value it prints.

    Only the value (last whitespace-separated token) of each line is used;
    lines that do not end in a number are ignored.
    """

    def __init__(self, script_path: str):
        self._script_path = script_path
        self._process: Optional[subprocess.Popen] = None
        self._latest_value: Optional[float] = None
        self._lock = threading.Lock()
        self._thread: Optional[threading.Thread] = None

    def _monitor_output(self):
        assert self._process is not None
        with self._process.stdout:
            for line in iter(self._process.stdout.readline, b''):
                try:
                    words = line.decode().strip().split()
                    if words:
                        value = float(words[-1])
                        with self._lock:
                            self._latest_value = value
                except (ValueError, UnicodeDecodeError):
                    # Ignore lines that don't end with a number or that fail to decode
                    continue

    def launch_and_monitor(self):
        if self._process is not None:
            raise RuntimeError("Script is already running.")
        self._process = subprocess.Popen(
            ["stdbuf", "-oL", "-eL", self._script_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            preexec_fn=lambda: subprocess.os.setpgid(0, 0)
        )
        self._thread = threading.Thread(target=self._monitor_output, daemon=True)
        self._thread.start()

    def get_latest_value(self) -> Optional[float]:
        with self._lock:
            return self._latest_value

    def stop(self):
        if self._process:
            self._process.terminate()
            self._process.wait()
            self._process = None
        if self._thread:
            self._thread.join(timeout=1)
            self._thread = None


def _start_reader(name: str, script_path: str, rank: int):
    if rank != 0:
        return
    monitor = ScriptMonitor(script_path)
    monitor.launch_and_monitor()
    while monitor.get_latest_value() is None:
        time.sleep(0.01)
    _readers[name] = monitor


def init():
    """Read the ``P3EM_*`` environment variables and start the meters.

    ``P3EM_ENABLED=1`` activates monitoring.  ``P3EM_CPU`` and/or
    ``P3EM_GPU`` name the meter script(s) to run.  If monitoring is enabled
    but neither is set the program aborts, matching the C/C++ interfaces.
    """
    global enabled, _initialized
    if _initialized:
        return
    _initialized = True

    enabled = os.environ.get("P3EM_ENABLED") == "1"
    if not enabled:
        return

    rank = _local_rank()
    cpu_path = os.environ.get("P3EM_CPU")
    gpu_path = os.environ.get("P3EM_GPU")

    if not cpu_path and not gpu_path:
        sys.stderr.write(
            "[p3em] ERROR: P3EM_ENABLED set but neither P3EM_CPU nor P3EM_GPU defined.\n"
        )
        sys.exit(1)

    if gpu_path:
        sys.stderr.write(f"[p3em] GPU monitoring enabled: {gpu_path}\n")
        _start_reader("gpu", gpu_path, rank)

    if cpu_path:
        sys.stderr.write(f"[p3em] CPU monitoring enabled: {cpu_path}\n")
        _start_reader("cpu", cpu_path, rank)


def now() -> float:
    """Return the current energy counter or wall-clock seconds.

    When monitoring is disabled this is a drop-in replacement for
    ``time.time()``.  When enabled it is the sum of the latest values of the
    configured CPU and GPU meters (0 on non-zero local ranks).
    """
    if not enabled:
        return time.time()
    total = 0.0
    for monitor in _readers.values():
        value = monitor.get_latest_value()
        if value is not None:
            total += value
    return total


def cleanup():
    """Stop all meter scripts and reset the module state."""
    global enabled, _initialized
    for monitor in _readers.values():
        monitor.stop()
    _readers.clear()
    enabled = False
    _initialized = False


class Timer:
    """Minimal timer analogous to the C ``my_timer`` / C++ ``P3emTimer``."""

    def __init__(self):
        init()
        self._start_sec = now()

    def start(self):
        self._start_sec = now()

    def elapsed(self) -> float:
        return now() - self._start_sec
