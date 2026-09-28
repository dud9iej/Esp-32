#!/bin/bash

echo "[SETUP] Updating package lists and installing prerequisites..."
sudo apt-get update && sudo apt-get install -y python3-pip python3-venv git

echo "[SETUP] Installing PlatformIO Core..."
pip3 install --upgrade platformio

echo "[SETUP] Initializing and building PlatformIO project environment..."
# This will automatically trigger platform downloads and pull dependencies from platformio.ini
pio run

echo "[SETUP] Done! Silver Parakeet is ready to compile."