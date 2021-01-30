#!/bin/bash
# This script is run from the CI service and does:
# 1) checks, if code is correctly formatted.
# 2) compiles canorus
# 3) installs canorus locally

set -euxo pipefail

mkdir -p ../build
cd ../build
cmake -D CMAKE_INSTALL_PREFIX=~/.local ../canorus

echo "Check, if code is formatted with \"make fmt\"..."
make fmt
cd ../canorus
# Output diff, if any.
git diff
# Abort, if there are any differences.
git diff --quiet
cd ../build

make -j4 
make tr
make install

