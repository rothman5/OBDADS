#!/bin/bash

source .venv/bin/activate
uv sync
uv run main.py start /dev/ttyRPMSG0
