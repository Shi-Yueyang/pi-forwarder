#!/usr/bin/env python3
"""Run two integration test instances side by side, wait 5s, then terminate.

Logs overwrite on each run.
"""

import subprocess
import sys
import time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_DIR = SCRIPT_DIR.parent.parent

WAIT_SEC = 5


def main() -> None:
    log_a = open(SCRIPT_DIR / "log_a.txt", "w")
    log_b = open(SCRIPT_DIR / "log_b.txt", "w")

    proc_a = subprocess.Popen(
        [sys.executable, str(SCRIPT_DIR / "send_hello.py"), str(SCRIPT_DIR / "config_a.json")],
        stdout=log_a, stderr=subprocess.STDOUT, cwd=PROJECT_DIR,
    )
    print(f"[run_demo] App A started (pid {proc_a.pid})")

    proc_b = subprocess.Popen(
        [sys.executable, str(SCRIPT_DIR / "send_hello.py"), str(SCRIPT_DIR / "config_b.json")],
        stdout=log_b, stderr=subprocess.STDOUT, cwd=PROJECT_DIR,
    )
    print(f"[run_demo] App B started (pid {proc_b.pid})")

    try:
        print(f"[run_demo] Waiting {WAIT_SEC}s ...")
        time.sleep(WAIT_SEC)
    except KeyboardInterrupt:
        print("\n[run_demo] Interrupted")
    finally:
        print("[run_demo] Shutting down ...")
        proc_a.terminate()
        proc_b.terminate()
        proc_a.wait()
        proc_b.wait()
        log_a.close()
        log_b.close()
        print("[run_demo] Done")


if __name__ == "__main__":
    main()
