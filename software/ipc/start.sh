#!/bin/bash

source .venv/bin/activate
uv sync
uv run main.py /dev/ttyS0 ../grafana/OBD.db
