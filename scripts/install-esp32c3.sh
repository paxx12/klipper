#!/usr/bin/env bash

set -xeo pipefail

git clone --recursive https://github.com/espressif/esp-idf.git esp-idf
cd esp-idf
./install.sh esp32c3
echo "Run now: source ./export.sh"
