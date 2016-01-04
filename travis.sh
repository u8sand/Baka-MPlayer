#!/bin/bash

mkdir -p build/mpv
cd build/mpv

git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build
echo --disable-pdf-build --enable-libmpv-shared > mpv_options
./update
# install mpv-dependencies
mk-build-deps -s sudo -i
dpkg-buildpackage -uc -us -b -j${JOBS}
# install mpv
sudo dpkg -i ../mpv*.deb
cd ..

mkdir -p baka-mplayer
cd baka-mplayer
qmake-qt5 ../../src/Baka-MPlayer.pro CONFIG+=release CONFIG+=install_translations -spec linux-g++
make -j $(grep -c ^processor /proc/cpuinfo)
