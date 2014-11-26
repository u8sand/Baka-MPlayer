#!/bin/sh

qmake src/Baka-MPlayer.pro
if [ -z "$1" ]; then
	make -j `grep -c ^processor /proc/cpuinfo`;
else
	make $1;
fi
