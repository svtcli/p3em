#!/usr/bin/env python3
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

import time
from p3em import ScriptMonitor

def main():
    monitor = ScriptMonitor("../p3em.sh")
    monitor.launch_and_monitor()
    time.sleep(10)
    try:
        for i in range(10):
            value = monitor.get_latest_value()
            print(f"Latest value: {value}")
            time.sleep(0.03)
    finally:
        monitor.stop()

if __name__ == "__main__":
    main()
