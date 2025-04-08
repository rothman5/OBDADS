# Grafana Module

## Overview
This module is responsible for the installation, start, and stop of the Grafana server.

## Setup
There exists an `install.sh` that installs Grafana for either `Ubuntu` or `ARMv7`.
There also exists a `start.sh` that will start the module and a `stop.sh` that will stop the module.

## Commands
There are a few scripts in this module, namely `install.sh`, `setup.sh`, `start.sh`, and `stop.sh`.
The `install.sh` script will install Grafana.
The `setup.sh` script will create the default database and setup provisioning for the dashboard and datasource.
The `start.sh` script will start the Grafana server.
Lastly, the `stop.sh` will stop the Grafana server.

### `install <arch>`
Installs Grafana for the specificed architecture, `arch` can be `ubuntu` or `armv7` and uses `ubuntu` by default.
```bash
./install.sh armv7
```

### `setup`
Creates the default database.
Installs the SQLite plugin for Grafana.
Creates provisioning for dashboard and datasource.
This imports the `dashboard.json` into Grafana and links the database to it.
```bash
./setup.sh
```

### `start`
Start the Grafana server.
```bash
./start.sh
```

### `stop`
Stop the Grafana server.
```bash
./stop.sh
```

## Dashboard
The dashboard is a JSON file used by Grafana to load the charts, tables, graphs, and statistic trackers configured on the Grafana UI.
These elements have queries specific to the database schema we are using and must be configured to load data from the specified database.
