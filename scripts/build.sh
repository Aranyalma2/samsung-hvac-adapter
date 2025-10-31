#!/bin/bash
# This script compiles the Arduino project in the parent directory
# for the device with FQBN:
# esp32:esp32:wt32-eth01:UploadSpeed=115200,FlashFreq=40,FlashMode=dio,PartitionScheme=min_spiffs

# Do not exit immediately on error so we can handle it manually.
set +e

# Define the Fully Qualified Board Name (FQBN)
FQBN="esp32:esp32:wt32-eth01:UploadSpeed=115200,FlashFreq=40,FlashMode=dio,PartitionScheme=min_spiffs,DebugLevel=verbose"

# Move to the project directory (assumed to be one level up from the script's directory)
cd "$(dirname "$0")/../firmware/"

# Print current working directory
echo "Compiling project in directory: $(pwd)"

# Compile the project using arduino-cli and the FQBN specified.
echo "Starting compilation..."
arduino-cli compile --fqbn "$FQBN" -e .
exit_code=$?

# Handle error or success and keep terminal open.
if [ $exit_code -eq 0 ]; then
    echo "Compilation completed successfully."
else
    echo "Compilation failed with exit code $exit_code."
fi

# Wait for user input before closing to allow inspection of the output.
read -n1 -r -p "Press any key to exit..."
exit $exit_code