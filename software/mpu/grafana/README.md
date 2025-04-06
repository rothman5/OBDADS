# Grafana Module

## Overview
This module is responsible for the installation, start, and stop of the Grafana server.

## Setup
There exists an `install.sh` that installs Grafana for either `Ubuntu` or `ARMv7`.
There also exists a `start.sh` that will start the module and a `stop.sh` that will stop the module.

## Dashboard
The dashboard is a JSON file used by Grafana to load the charts, tables, graphs, and statistic trackers configured on the Grafana UI.
These elements have queries specific to the database schema we are using and must be configured to load data from the specified database.
