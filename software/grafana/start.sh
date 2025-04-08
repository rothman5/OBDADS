#!/bin/bash

GRAFANA_HOME_DIR="$HOME/grafana-v11.6.0"

grafana server --homepath $GRAFANA_HOME_DIR
