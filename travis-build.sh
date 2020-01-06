#!/bin/sh
cd
mkdir -p build
cd build
cmake ../canorus -D CMAKE_INSTALL_PREFIX=~/.local
make -j4 
make tr
make install

