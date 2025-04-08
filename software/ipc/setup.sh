#!/bin/bash

# Activate the virtual environment
source .venv/bin/activate

# Sync the virtual environment dependencies
uv sync

# Set the firmware file path
uv run main.py firmware /usr/local/projects/OBDADS_DK2_CM4/lib/firmware/OBDADS_DK2_CM4.elf

# Set the script file path
uv run main.py script /usr/local/projects/OBDADS_DK2_CM4/fw_cortex_m4.sh

# Set the database file path
uv run main.py database /usr/local/projects/grafana/OBDADS_DK2_CM4.db
