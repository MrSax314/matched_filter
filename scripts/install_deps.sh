#!/usr/bin/env bash

set -e

# Install nlohmann json for config file parsing

REPO_URL="https://github.com/nlohmann/json"
TAG_VERSION="v3.11.3"
SRC_DIR="/tmp/nlohmann_json_source"

echo "=== Starting nlohmann/json Installation ==="

if [ "$EUID" -ne 0 ]; then
    echo "Error: Please run this script with sudo or as root." >&2
    exit 1
fi

echo "Cloning repo..."
rm -rf "${SRC_DIR}"
git clone --branch "${TAG_VERSION}" --depth 1 "${REPO_URL}" "${SRC_DIR}"
cd "${SRC_DIR}"

echo "Configuring build layout..."
cmake -B build -S . \
  -DJSON_BuildTests=OFF \
  -DCMAKE_BUILD_TYPE=Release

echo "Installing library to system directories..."
cmake --install build

echo "Cleaning up temporary files..."
rm -rf "${SRC_DIR}"

echo "=== Installation Successful! ==="
echo "You can now use 'find_package(nlohmann_json CONFIG REQUIRED)' in CMake."
