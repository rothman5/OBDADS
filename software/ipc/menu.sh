#!/bin/bash

PY_SCRIPT="main.py"
IPC_PORT="/dev/ttyRPMSG0"

if [[ ! -f "$PY_SCRIPT" ]]; then
    echo "Error: $PY_SCRIPT not found!"
    exit 1
fi

if [[ -d "$PYTHON_ENV" ]]; then
    source "$PYTHON_ENV/bin/activate"
else
    echo "Error: Python virtual environment not found at $PYTHON_ENV"
    exit 1
fi

if [[ "$1" == "set-firmware" && -n "$2" ]]; then
    uv run "$PY_SCRIPT" firmware "$2"

elif [[ "$1" == "set-script" && -n "$2" ]]; then
    uv run "$PY_SCRIPT" script "$2"

elif [[ "$1" == "set-database" && -n "$2" ]]; then
    uv run "$PY_SCRIPT" database "$2"

elif [[ "$1" == "start" ]]; then
    uv run "$PY_SCRIPT" start "$IPC_PORT"

elif [[ "$1" == "stop" ]]; then
    uv run "$PY_SCRIPT" stop

elif [[ "$1" == "state" ]]; then
    uv run "$PY_SCRIPT" state

else
    echo "Usage: $0 [set-firmware <fw-path> | set-script <script-path> | set-database <database-path> | start | stop | state]"
    exit 1
fi
