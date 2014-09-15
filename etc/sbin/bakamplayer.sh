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
mxeroot=$(pwd)/mxe.$arch/usr/bin/

# get bakamplayer
git clone https://github.com/u8sand/Baka-MPlayer.git Baka-MPlayer.$arch
cd Baka-MPlayer.$arch
mkdir -p build
cd build
# generate toolchain cmake
cat > $arch.cmake << EOF
SET(CMAKE_SYSTEM_NAME win32)
SET(CMAKE_C_COMPILER   ${mxeroot}${arch}-w64-mingw32.static-gcc)
SET(CMAKE_CXX_COMPILER ${mxeroot}${arch}-w64-mingw32.static-g++)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${mxeroot}/${arch}-w64-mingw32.static)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=$arch.cmake ..
make
cd ..
