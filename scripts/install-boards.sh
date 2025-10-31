#!/bin/bash

# Ensure arduino-cli is installed
if ! command -v arduino-cli &> /dev/null; then
    echo "arduino-cli is not installed. Please install it first."
    exit 1
fi

# ESP32 board package details
PACKAGE_URL="https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json"
PACKAGE_NAME="esp32:esp32"

# Add ESP32 board package URL to Arduino CLI configuration
echo "Configuring ESP32 package URL..."
arduino-cli config add board_manager.additional_urls "$PACKAGE_URL"

# Update the index of available boards
echo "Updating board index..."
arduino-cli core update-index

# Install the ESP32 board package
echo "Installing ESP32 board package..."
arduino-cli core install "$PACKAGE_NAME"

echo "ESP32 board package has been installed!"
