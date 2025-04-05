#!/bin/bash

TARGET="${1:-ubuntu}"
echo "Running install for target: $TARGET"

update_system() {
    echo "Updating system..."
    sudo apt-get update -y
    sudo apt-get upgrade -y
    sudo apt-get autoremove -y
}

install_ubuntu() {
    pushd $HOME
    echo "Installing for Ubuntu..."
    update_system
    sudo apt-get install -y adduser libfontconfig1 musl
    wget https://dl.grafana.com/enterprise/release/grafana-enterprise_11.6.0_amd64.deb
    sudo dpkg -i grafana-enterprise_11.6.0_amd64.deb
    popd
}

install_ubuntu_standalone() {
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

if [ "$TARGET" = "ubuntu" ]; then
  if [ "$MODE" = "standalone" ]; then
    install_ubuntu_standalone
  else
    install_ubuntu
  fi

elif [ "$TARGET" = "armv7" ]; then
  if [ "$MODE" = "standalone" ]; then
    install_armv7_standalone
  else
    install_armv7
  fi

else
  echo "Unknown target: $TARGET"
  echo "Usage: $0 [ubuntu|armv7|windows] [standalone]"
  exit 1
fi
