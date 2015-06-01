#!/bin/sh

JOBS=4

# for debian-based systems (debian, linux mint, ubuntu)

# install dependencies
sudo apt-get install git gcc g++ autoconf automake libtool yasm devscripts libfreetype6-dev libfribidi-dev libfontconfig1-dev pkg-config qtbase5-dev qt5-qmake qttools5-dev-tools qtdeclarative5-dev libqt5svg5-dev libqt5x11extras5-dev libqt5network5

# build mpv
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

# build baka-mplayer
git clone https://github.com/u8sand/Baka-MPlayer.git
cd Baka-MPlayer
QMAKE=/usr/lib/x86_64-linux-gnu/qt5/bin/qmake ./configure CONFIG+=install_translations
make -j${JOBS}
#dpkg-buildpackage -uc -us -b -j${JOBS}
# install baka-mplayer
sudo make install
#sudo dpkg -i ../baka-mplayer*.deb
cd ..
