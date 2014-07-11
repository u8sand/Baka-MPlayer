#!/bin/bash

# Todo: Detect existing libraries

# Download MXE. Note that compiling the required packages requires about 1.4 GB
# or more!

mkdir -p /tmp/build
cd /tmp/build
git clone https://github.com/mxe/mxe mingw
cd mingw

# Set build options.

# The JOBS environment variable controls threads to use when building. DO NOT
# use the regular `make -j4` option with MXE as it will slow down the build.
# Alternatively, you can set this in the make command by appending "JOBS=4"
# to the end of command:
echo "JOBS := 12" >> settings.mk

# The MXE_TARGET environment variable builds MinGW-w64 for 32 bit targets.
# Alternatively, you can specify this in the make command by appending
# "MXE_TARGETS=i686-w64-mingw32" to the end of command:
# echo "MXE_TARGETS := i686-w64-mingw32" >> settings.mk

# If you want to build 64 bit version, use this:
echo "MXE_TARGETS := x86_64-w64-mingw32.static" >> settings.mk

# Build required packages. The following provide a minimum required to build
# mpv and Baka MPlayer.  Todo: Add more libraries for all mpv features.

make gcc ffmpeg libass jpeg pthreads-w32 qt qtsvg

# Add MXE binaries to $PATH
export PATH=/tmp/build/mingw/usr/bin/:$PATH

# Build mpv. The target will be used to automatically select the name of the
# build tools involved (e.g. it will use i686-w64-mingw32-gcc).

cd ..
git clone https://github.com/mpv-player/mpv.git
cd mpv
#DEST_OS=win32 TARGET=i686-w64-mingw32 ./waf configure
# Or, if 64 bit version,
DEST_OS=win32 TARGET=x86_64-w64-mingw32.static ./waf configure --enable-libmpv-static --enable-static-build --disable-client-api-examples
./waf build

cp build/libmpv.a ../mingw/usr/lib/
mkdir -p ../mingw/usr/include/mpv/
cp build/libmpv/client.h ../mingw/usr/include/mpv/
cp build/libmpv/mpv.pc ../mingw/usr/lib/pkgconfig/
export PATH=/tmp/build/mpv/build/:$PATH

cd ..
git clone https://github.com/u8sand/Baka-MPlayer.git
cd Baka-MPlayer/Baka-MPlayer
DEST_OS=win32 TARGET=x86_64-w64-mingw32.static qemu .
DEST_OS=win32 TARGET=x86_64-w64-mingw32.static make

