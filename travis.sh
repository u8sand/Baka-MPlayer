#!/bin/bash

mkdir -p build
cd build
qmake-qt5 ../../src/Baka-MPlayer.pro CONFIG+=release CONFIG+=install_translations -spec linux-g++
make -j $(grep -c ^processor /proc/cpuinfo)
