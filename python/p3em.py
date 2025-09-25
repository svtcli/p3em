#   Copyright(C) 2025 Alexander Pöppl, Intel Corporation
#   Copyright(C) 2025 Salvatore Cielo, LRZ
#   Copyright(C) 2025 Ivan Pribec, LRZ
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the
#  License. You may obtain a copy of the License at    http:#www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
#  language governing permissions and limitations under the License.

import subprocess
import threading
from typing import Optional

class ScriptMonitor:
    """
    Launches a script and monitors its output, extracting the latest integer value from each line.
    """

    def __init__(self, script_path: str = "../p3em.sh"):
        self._script_path = script_path
        self._process: Optional[subprocess.Popen] = None
        self._latest_value: Optional[int] = None
        self._lock = threading.Lock()
        self._thread: Optional[threading.Thread] = None

    def _monitor_output(self):
        assert self._process is not None
        with self._process.stdout:
            for line in iter(self._process.stdout.readline, b''):
                try:
                    words = line.decode().strip().split()
                    if words:
                        value = int(words[-1])
                        with self._lock:
                            self._latest_value = value
                except (ValueError, UnicodeDecodeError):
                    # Ignore lines that don't end with an integer or that fail to decode
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

    def get_latest_value(self) -> Optional[int]:
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
