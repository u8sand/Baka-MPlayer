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
export PATH=$(pwd)/mxe.$arch/usr/bin/:$PATH

# get bakamplayer
git clone https://github.com/u8sand/Baka-MPlayer.git Baka-MPlayer.$arch
cd Baka-MPlayer.$arch
$arch-w64-mingw32.static-qmake-qt5 src/Baka-MPlayer.pro
make
cd ..
