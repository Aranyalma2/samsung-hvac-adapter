#!/bin/bash

CONFIG_FILE="../src/config.h"

# Extract the current version string
version=$(grep -oP '(?<=#define FIRMWARE_VERSION ")[^"]+' "$CONFIG_FILE")

if [[ -z "$version" ]]; then
    echo "Error: Could not find the version number in $CONFIG_FILE"
    exit 1
fi

# Extract the base version (e.g., 1.0.0-prealpha) and build number
base_version=$(echo "$version" | grep -oP '^[^-]+(-[a-zA-Z]+)?')
build_number=$(echo "$version" | grep -oP '(?<=\.)\d+$')

if [[ -z "$build_number" ]]; then
    build_number=0
fi

# Increment the build number
new_build_number=$((build_number + 1))

# Construct the new version string
new_version="$base_version.$new_build_number"

# Replace the old version with the new one
sed -i "s/#define FIRMWARE_VERSION \"$version\"/#define FIRMWARE_VERSION \"$new_version\"/" "$CONFIG_FILE"

echo "Updated version to $new_version"

# Run the build script
./build.sh