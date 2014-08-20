#!/bin/bash

hash qmake 2>/dev/null || { echo >&2 "Cannot find qmake."; exit 1; }
hash make 2>/dev/null || { echo >&2 "Cannot find make."; exit 1; }

qmake_ver=`qmake --version | awk '/Using Qt version.*/{print $4}'`
echo "Found qt version $qmake_ver...";
if [[ $qmake_ver =~ (5.*) ]]; then
	qmake source/Baka-MPlayer.pro && make $@
else
	echo "You need qt version 5.x.x to compile Baka MPlayer.";
fi
