#!/bin/bash

DATABASE_NAME="OBD.db"
DATABASE_SCHEMA="schema.sql"
SQLITE_PLUGIN="frser-sqlite-datasource"

GRAFANA_HOME_DIR="$HOME/grafana-v11.6.0"
GRAFANA_PLUGIN_DIR="$GRAFANA_HOME_DIR/data/plugins"
GRAFANA_PROV_DIR="$GRAFANA_HOME_DIR/conf/provisioning"
GRAFANA_DATASOURCE_DIR="$GRAFANA_PROV_DIR/datasources"
GRAFANA_DASHBOARD_YAML_DIR="$GRAFANA_PROV_DIR/dashboards"
GRAFANA_DASHBOARD_JSON_DIR="$GRAFANA_DASHBOARD_YAML_DIR/sqlite"
GRAFANA_DASHBOARD="dashboard/dashboard.json"

echo "Creating default database and populating schema..."
sqlite3 $DATABASE_NAME < $DATABASE_SCHEMA

echo "Installing SQLite plugin for Grafana..."
sudo $HOME/grafana-v11.6.0/bin/grafana cli --homepath $GRAFANA_HOME_DIR --pluginsDir $GRAFANA_PLUGIN_DIR plugins install $SQLITE_PLUGIN

echo "Setting up datasource provisioning..."
cat > "$GRAFANA_DATASOURCE_DIR/sqlite_datasources.yaml" <<EOF
apiVersion: 1
datasources:
  - name: 'OBDADS'
    type: $SQLITE_PLUGIN
    access: proxy
    editable: true
    isDefault: true
    jsonData:
      path: $(realpath $DATABASE_NAME)
EOF

echo "Setting up dashboard provisioning..."
mkdir -p "$GRAFANA_DASHBOARD_JSON_DIR"
cat > "$GRAFANA_DASHBOARD_YAML_DIR/sqlite_dashboards.yaml" <<EOF
apiVersion: 1
providers:
  - name: 'OBDADS Dashboards'
    orgId: 1
    folder: ''
    type: file
    disableDeletion: false
    editable: true
    options:
      path: $GRAFANA_DASHBOARD_JSON_DIR
EOF

if [ -f dashboard/dashboard.json ]; then
    cp dashboard/dashboard.json "$GRAFANA_DASHBOARD_JSON_DIR"
else
    echo "Dashboard JSON not found — skipping copy."
fi
