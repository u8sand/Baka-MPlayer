#!/bin/sh

JOBS=4

# install dependencies
sudo zypper install git gcc gcc-g++ make autoconf automake libtool yasm freetype2-devel fribidi-devel fontconfig-devel libqt5-qtbase-devel libqt5-qttools-devel libqt5-qtdeclarative-devel libqt5-qtsvg-devel libqt5-qtx11extras-devel

# build mpv
git clone https://github.com/mpv-player/mpv-build.git
cd mpv-build
echo --prefix=/usr --libdir=/usr/lib64 --enable-libmpv-shared > mpv_options
./rebuild -j${JOBS}
# install mpv
sudo ./install
sudo mv /usr/lib64/pkgconfig/mpv.pc /usr/share/pkgconfig/
cd ..

# build baka-mplayer
git clone https://github.com/u8sand/Baka-MPlayer.git
cd Baka-MPlayer
QMAKE=/usr/lib64/x86_64-linux-gnu/qt5/bin/qmake ./configure
make -j${JOBS}
# install baka-mplayer
sudo make install
cd ..
