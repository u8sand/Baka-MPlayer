#!/bin/bash

ninja=1
arch=0

hash cmake 2>/dev/null || { echo >&2 "Cannot find cmake."; exit 1; }
hash ninja 2>/dev/null || { echo >&2 "Cannot find ninja."; ninja=0; }
if [[ $ninja == 0 ]]; then
	hash make 2>/dev/null || { echo >&2 "Cannot find make."; exit 1; }
fi

if [[ $1 == "i686" ]]; then
	arch=32
elif [[ $1 == "x86_64" ]]; then
	arch=64
fi

if [[ $ninja == 1  ]]; then
	if [[ $arch -ne 0 ]]; then
		mkdir -p build-$arch
		cd build-$arch
		cmake -G Ninja CMAKE_C_FLAGS=-m$arch -DCMAKE_CXX_FLAGS=-m$arch ..
	else
		mkdir -p build
		cd build
		cmake -G Ninja ..
	fi
	ninja
else
	if [[ $arch -ne 0 ]]; then
		mkdir -p build-$arch
		cd build-$arch
		cmake CMAKE_C_FLAGS=-m$arch -DCMAKE_CXX_FLAGS=-m$arch ..
	else
		mkdir -p build
		cd build
		cmake ..
	fi
	make
fi
