#!/usr/bin/env python3
"""Integration test: start a forwarder, send to it in a loop, read responses.

Usage: send_hello.py <config.json>

Parses the config to find the local UDP endpoint, launches the forwarder,
then enters a loop that sends "hello" to the forwarder and prints any
received data.  Ctrl-C to stop.
"""

import json
import socket
import subprocess
import sys
import time

FORWARDER_BIN = "./build/forwarder"
PAYLOAD = b"hello"
READ_TIMEOUT = 1.0  # seconds
STARTUP_DELAY = 1.0  # seconds to wait for forwarder to bind


def main() -> None:
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <config.json>", file=sys.stderr)
        sys.exit(1)

    config_path = sys.argv[1]

    # Parse config to find the local UDP endpoint
    with open(config_path) as f:
        cfg = json.load(f)

    ch = cfg["connections"][0]["udp_channel"]
    fwd_addr = (ch["local_ip"], ch["local_port"])
    print(f"[test] Forwarder local endpoint: {fwd_addr[0]}:{fwd_addr[1]}")

    # Start forwarder
    proc = subprocess.Popen([FORWARDER_BIN, config_path])
    print(f"[test] Forwarder started (pid {proc.pid})")
    time.sleep(STARTUP_DELAY)

    # Open local UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(READ_TIMEOUT)

    seq = 0
    try:
        while True:
            seq += 1
            print(f"\n--- cycle {seq} ---")

            # Send
            sock.sendto(PAYLOAD, fwd_addr)
            print(f"[test] sent {PAYLOAD!r}")

            # Try to read (fire-and-forget on timeout)
            try:
                data, addr = sock.recvfrom(4096)
                print(f"[test] recv {data!r} from {addr}")
            except socket.timeout:
                print("[test] recv timeout (no data)")

    except KeyboardInterrupt:
        print("\n[test] Interrupted")
    finally:
        sock.close()
        proc.terminate()
        proc.wait()
        print("[test] Forwarder stopped")


if __name__ == "__main__":
    main()
