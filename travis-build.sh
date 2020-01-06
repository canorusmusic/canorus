#!/bin/sh
cd
mkdir -p build
cd build
cmake -D CMAKE_INSTALL_PREFIX=~/.local ../canorus
make -j4 
make tr
make install

