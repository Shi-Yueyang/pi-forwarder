#!/usr/bin/env bash
# Run two integration test instances side by side.
# Logs overwrite on each run.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

cd "$PROJECT_DIR"

LOG_A="$SCRIPT_DIR/log_a.txt"
LOG_B="$SCRIPT_DIR/log_b.txt"

cleanup() {
    echo "[run_demo] Shutting down..."
    kill "$PID_A" "$PID_B" 2>/dev/null || true
    wait "$PID_A" "$PID_B" 2>/dev/null || true
    echo "[run_demo] Done"
}

trap cleanup EXIT INT TERM

echo "[run_demo] Starting App A (config_a) → $LOG_A"
python3 "$SCRIPT_DIR/send_hello.py" "$SCRIPT_DIR/config_a.json" > "$LOG_A" 2>&1 &
PID_A=$!

echo "[run_demo] Starting App B (config_b) → $LOG_B"
python3 "$SCRIPT_DIR/send_hello.py" "$SCRIPT_DIR/config_b.json" > "$LOG_B" 2>&1 &
PID_B=$!

echo "[run_demo] Both running (pids $PID_A, $PID_B). Ctrl-C to stop."
wait
