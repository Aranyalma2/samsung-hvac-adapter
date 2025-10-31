#!/bin/bash

# Ensure arduino-cli is installed
if ! command -v arduino-cli &> /dev/null; then
    echo "arduino-cli is not installed. Please install it first."
    exit 1
fi

# Update the index of available libraries
arduino-cli lib update-index

# List of GitHub repositories to install
REPOS=(
    "https://github.com/ESP32Async/ESPAsyncWebServer"
    "https://github.com/ESP32Async/AsyncTCP"
    "https://github.com/adafruit/Adafruit-GFX-Library"
    "https://github.com/adafruit/Adafruit_BusIO"
    "https://github.com/adafruit/Adafruit_SSD1306"
    "https://github.com/bblanchon/ArduinoJson"
    "https://github.com/eModbus/eModbus"
)

# Install each library from the GitHub URL
for REPO in "${REPOS[@]}"; do
    echo "Installing from $REPO..."
    arduino-cli lib install --git-url "$REPO"
done

echo "All libraries have been installed!"
