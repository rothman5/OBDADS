#!/bin/bash

PROV_DIR="/etc/grafana/provisioning"
DATASOURCE_DIR="$PROV_DIR/datasources"
DASHBOARD_YAML_DIR="$PROV_DIR/dashboards"
DASHBOARD_JSON_DIR="$PROV_DIR/dashboards/sqlite"
SQLITE_PLUGIN="frser-sqlite-datasource"

echo "Creating default database and populating schema..."
sqlite3 OBDADS_DK2_CM4.db < schema.sql
echo "Done!"

echo "Installing SQLite plugin for Grafana..."
grafana-cli plugins install $SQLITE_PLUGIN

echo "Creating SQLite datasource provisioning..."
cat > "$DATASOURCE_DIR/sqlite.yaml" <<EOF
apiVersion: 1

datasources:
  - name: SQLite
    type: $SQLITE_PLUGIN
    access: proxy
    editable: true
    isDefault: true
    jsonData:
      path: $(realpath OBDADS_DK2_CM4.db)
EOF

echo "Setting up dashboard provisioning..."
mkdir -p "$DASHBOARD_JSON_DIR"

cat > "$DASHBOARD_YAML_DIR/sqlite_dashboards.yaml" <<EOF
apiVersion: 1

providers:
  - name: 'SQLite Dashboards'
    orgId: 1
    folder: ''
    type: file
    disableDeletion: false
    editable: true
    options:
      path: $DASHBOARD_JSON_DIR
EOF

if [ -f dashboard/dashboard.json ]; then
    cp dashboard/dashboard.json "$DASHBOARD_JSON_DIR"
else
    echo "Dashboard JSON not found — skipping copy."
fi

echo "Restarting Grafana to apply changes..."
sudo systemctl restart grafana-server
