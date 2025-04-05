#!/bin/bash

source .venv/bin/activate
uv sync
uv run main.py firmware /usr/local/projects/OBDADS_DK2_CM4/lib/firmware/OBDADS_DK2_CM4.elf
uv run main.py script /usr/local/projects/OBDADS_DK2_CM4/fw_cortex_m4.sh
uv run main.py database /usr/local/projects/ipc/OBDADS_DK2_CM4.db
