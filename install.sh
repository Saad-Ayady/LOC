#!/bin/bash

echo "*=============================*"
echo "|  ██       ██████   ██████   |"
echo "|  ██      ██    ██ ██        |"
echo "|  ██      ██    ██ ██        |"
echo "|  ██      ██    ██ ██        |"
echo "|  ███████  ██████   ██████   |"
echo "|                             |"
echo "*=============================*"
echo "|         coded by 0xdy       |"
echo "*=============================*"
                         

# Detect distro
if [ -f /etc/debian_version ]; then
    DISTRO="debian"
elif [ -f /etc/arch-release ]; then
    DISTRO="arch"
elif [ -f /etc/fedora-release ]; then
    DISTRO="fedora"
else
    echo "[ERROR] Unsupported Linux distribution."
    exit 1
fi

echo "[INFO] Detected distro: $DISTRO"
echo "[INFO] Installing mingw-w64 ..."

# Install mingw based on distro
case $DISTRO in
    debian)
        sudo apt update
        sudo apt install -y mingw-w64 make
        ;;
    arch)
        sudo pacman -Sy --noconfirm mingw-w64-gcc make
        ;;
    fedora)
        sudo dnf install -y mingw64-gcc mingw64-gcc-c++ make
        ;;
esac

echo "[INFO] Toolchain successfully installed."

# Ensure Makefile exists
if [ ! -f Makefile ]; then
    echo "[ERROR] Makefile not found! Put this script beside your Makefile."
    exit 1
fi
