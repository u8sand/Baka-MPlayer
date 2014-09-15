#!/bin/bash

ninja=1

hash cmake 2>/dev/null || { echo >&2 "Cannot find cmake."; exit 1; }
hash ninja 2>/dev/null || { echo >&2 "Cannot find ninja."; ninja=0; }
if [[ $ninja == 0 ]]; then
	hash make 2>/dev/null || { echo >&2 "Cannot find make."; exit 1; }
fi

mkdir -p build
cd build
if [[ $ninja == 1  ]]; then
	cmake -G Ninja ..
	ninja
else
	cmake ..
	make
fi
