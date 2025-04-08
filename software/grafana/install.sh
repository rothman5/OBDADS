#!/bin/bash

TARGET="${1:-ubuntu}"
echo "Running install for target: $TARGET"

update_system() {
  echo "Updating system..."
  sudo apt-get update -y
  sudo apt-get upgrade -y
  sudo apt-get autoremove -y
}

install_amd64() {
  pushd $HOME
  echo "Installing for Ubuntu..."
  update_system
  sudo apt-get install -y adduser libfontconfig1 musl
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise_11.6.0_amd64.deb
  sudo dpkg -i grafana-enterprise_11.6.0_amd64.deb
  popd
}

install_amd64_standalone() {
  pushd $HOME
  echo "Installing standalone for Ubuntu..."
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise-11.6.0.linux-amd64.tar.gz
  tar -zxvf grafana-enterprise-11.6.0.linux-amd64.tar.gz
  popd
}

install_armv7() {
  pushd $HOME
  echo "Installing for ARMv7..."
  update_system
  sudo apt-get install -y adduser libfontconfig1 musl
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise_11.6.0_armhf.deb
  sudo dpkg -i grafana-enterprise_11.6.0_armhf.deb
  popd
}

install_armv7_standalone() {
  pushd $HOME
  echo "Installing standalone for ARMv7..."
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise-11.6.0.linux-armv7.tar.gz
  tar -zxvf grafana-enterprise-11.6.0.linux-armv7.tar.gz
  popd
}

install_arm64() {
  pushd $HOME
  echo "Installing for ARMv7..."
  update_system
  sudo apt-get install -y adduser libfontconfig1 musl
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise_11.6.0_arm64.deb
  sudo dpkg -i grafana-enterprise_11.6.0_arm64.deb
  popd
}

install_arm64_standalone() {
  pushd $HOME
  echo "Installing standalone for ARMv7..."
  wget https://dl.grafana.com/enterprise/release/grafana-enterprise-11.6.0.linux-arm64.tar.gz
  tar -zxvf grafana-enterprise-11.6.0.linux-arm64.tar.gz
  popd
}

if [ "$TARGET" = "amd64" ]; then
  install_amd64
elif [ "$TARGET" = "amd64-standalone" ]; then
  install_amd64_standalone
elif [ "$TARGET" = "armv7" ]; then
  install_armv7
elif [ "$TARGET" = "armv7-standalone" ]; then
  install_armv7_standalone
elif [ "$TARGET" = "arm64" ]; then
  install_arm64
elif [ "$TARGET" = "arm64-standalone" ]; then
  install_arm64_standalone
else
  echo "Unknown target: $TARGET"
  echo "Usage: $0 [ubuntu|armv7] [standalone]"
  exit 1
fi
