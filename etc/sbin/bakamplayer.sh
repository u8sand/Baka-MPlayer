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
mkdir -p build
cd build
# generate toolchain cmake
cmake -DCMAKE_TOOLCHAIN_FILE=$mxeroot/usr/$arch-w64-mingw32.static/share/cmake/mxe-conf.cmake ..
make
cd ..
