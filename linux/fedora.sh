#!/bin/sh

JOBS=4

# install dependencies
sudo yum install git gcc gcc-c++ autoconf automake libtool yasm freetype-devel fribidi-devel fontconfig-devel pkgconfig qt5-qtbase-devel qt5-qttools-devel qt5-qtdeclarative-devel qt5-qtsvg-devel  qt5-qtx11extras-devel

# build mpv
git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build
echo --prefix=/usr --enable-libmpv-shared > mpv_options
./rebuild -j${JOBS}
# install mpv
sudo ./install
sudo mv /usr/lib/pkgconfig/mpv.pc /usr/share/pkgconfig/
cd ..

# build baka-mplayer
git clone https://github.com/u8sand/Baka-MPlayer.git
cd Baka-MPlayer
./configure
make -j${JOBS}
# install baka-mplayer
sudo make install
cd ..
