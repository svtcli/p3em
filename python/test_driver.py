#!/usr/bin/env python3
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
