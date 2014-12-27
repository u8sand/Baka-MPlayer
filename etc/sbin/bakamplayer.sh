#!/bin/bash

arch=x86_64
if [[ $1 == 'x86_64' ]]; then
	arch=x86_64
elif [[ $1 == 'i686' ]]; then
	arch=i686
else
	echo "Please specify either x86_64 or i686 architecture.";
	exit;
fi

# add built mxe directory to the path
mxeroot=$(pwd)/mxe.$arch

# get bakamplayer
git clone https://github.com/u8sand/Baka-MPlayer.git Baka-MPlayer.$arch
cd Baka-MPlayer.$arch
git pull

# setup mxe environment
export PATH=$mxeroot/usr/bin:$PATH
export PKG_CONFIG_PATH=$mreroot/usr/$arch-w64-mingw32.static/lib/pkgconfig
unset `env | \
    grep -vi '^EDITOR=\|^HOME=\|^LANG=\|MXE\|^PATH=' | \
    grep -vi 'PKG_CONFIG\|PROXY\|^PS1=\|^TERM=' | \
    cut -d '=' -f1 | tr '\n' ' '`

# build baka-mplayer
QMAKE="$mxeroot/usr/$arch-w64-mingw32.static/qt5/bin/qmake -win32" ./configure "CONFIG+=embed_translations"
make -j`grep -c ^processor /proc/cpuinfo`
cd ..
