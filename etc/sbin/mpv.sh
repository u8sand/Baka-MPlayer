#!/bin/bash

# mpv: https://github.com/mpv-player/mpv

# add built mxe directory to the path
export PATH=$(pwd)/mxe/usr/bin/:$PATH
# get mpv/waf
git clone https://github.com/mpv-player/mpv.git
cd mpv
./bootstrap.py

# if you're mxe toolchain is only there for one architecture,
#  comment out this block, the other architecture's block, and
#	  the cd command of your architecture
cd ..
# copy and make two folders: mpv.x86_64, mpv.i686
cp mpv{,.i686} -r
mv mpv{,.x86_64}

# compile i686
cd mpv.i686
DEST_OS=win32 TARGET=i686-w64-mingw32 ./waf configure --enable-libmpv-shared --enable-static-build --disable-client-api-examples
# todo: patch waftools/syms.py with nm=i686-w64-mingw32-nm
./waf build
cd ..

# compile x86_64
cd mpv.x86_64
DEST_OS=win32 TARGET=x86_64-w64-mingw32 ./waf configure --enable-libmpv-shared --enable-static-build --disable-client-api-examples
# todo: patch waftools/syms.py with nm=x86_64-w64-mingw32-nm
./waf build
cd ..
