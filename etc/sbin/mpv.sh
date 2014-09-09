#!/bin/bash

# mpv: https://github.com/mpv-player/mpv

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
export PATH=$(pwd)/mxe.$arch/usr/bin/:$PATH

# get mpv/waf
git clone https://github.com/mpv-player/mpv.git mpv.$arch
cd mpv.$arch
./bootstrap.py
DEST_OS=win32 TARGET=$arch-w64-mingw32.static ./waf configure --enable-libmpv-shared --enable-static-build --disable-client-api-examples
./waf build
cd ..
