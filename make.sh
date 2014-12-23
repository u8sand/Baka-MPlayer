#!/bin/sh

if [ "$1" != "" ]; then
  BAKA_LANG=$1
  # make sure we rebuild main
  touch src/main.cpp

  echo "Updating translations..."
  lupdate src/Baka-MPlayer.pro

  echo "Compiling translations..."
  lrelease src/Baka-MPlayer.pro

  echo "Preparing resource file..."
  cat src/rsclist.qrc.in | sed "s/\$lang/$BAKA_LANG.qm/g" > src/rsclist.qrc.out

  echo "Configuring..."
  qmake src/Baka-MPlayer.pro CONFIG+=release "DEFINES+=\'BAKA_MPLAYER_LANG=\\\":/translations/baka-mplayer_$BAKA_LANG\\\"\'" RESOURCES+=rsclist.qrc.out

else
  echo "Configuring..."
  qmake src/Baka-MPlayer.pro CONFIG+=release RESOURCES+=rsclist.qrc
fi

echo "Compiling baka-mplayer..."
make -j `grep -c ^processor /proc/cpuinfo`;
