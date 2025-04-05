#!/bin/bash

# NOTE: This requires the grafana-import tool to be installed through PIP.
grafana-import -i dashboard.json -u http://admin:obdads@localhost:3000 import
