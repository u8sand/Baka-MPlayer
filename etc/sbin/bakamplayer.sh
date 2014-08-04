#!/bin/bash

# add built mxe directory to the path
export PATH=$(pwd)/mxe/usr/bin/:$PATH

# get bakamplayer
git clone https://github.com/u8sand/Baka-MPlayer.git

# if you're mxe toolchain is only there for one architecture,
#  comment out this block, the other architecture's block, and
#	  the cd command of your architecture
# copy and make two folders: .x86_64, .i686
cp Baka-MPlayer{,.i686} -r
mv Baka-MPlayer{,.x86_64}

# compile i686
cd "Baka-MPlayer.i68/Baka MPlayer"
i686-w64-mingw32-qmake-qt5 .
make
cd ..

# compile x86_64
x86_64-w64-mingw32-qmake-qt5 .
make
cd ..
