#!/bin/sh

BAKA_LANG="en"

if [[ $1 != "" ]]; then
  BAKA_LANG=$1
  # make sure we rebuild main
  touch src/main.cpp
fi

echo "Configuring..."
if [[ $BAKA_LANG == "en" ]]; then
  qmake src/Baka-MPlayer.pro CONFIG+=release
else
  qmake src/Baka-MPlayer.pro CONFIG+=release "DEFINES+=\'BAKA_MPLAYER_LANG=\\\"baka-mplayer_$BAKA_LANG\\\"\'"

  echo "Updating translations..."
  lupdate src/Baka-MPlayer.pro

  echo "Compiling translations..."
  lrelease src/Baka-MPlayer.pro
fi

echo "Compiling baka-mplayer..."
make -j `grep -c ^processor /proc/cpuinfo`;

if [[ $BAKA_LANG != "en" ]]; then
  echo "Copying translation to build..."
  cp src/translations/baka-mplayer_$BAKA_LANG.qm build/
fi
