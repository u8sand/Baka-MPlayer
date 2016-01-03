#!/bin/bash

sudo apt-get install libqt5x11extras5-dev libmpv-dev
mkdir build
cd build
qmake-qt5 ../src/Baka-MPlayer.pro CONFIG+=release CONFIG+=install_translations -spec linux-g++
make -j $(grep -c ^processor /proc/cpuinfo)

