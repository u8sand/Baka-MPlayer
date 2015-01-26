#!/bin/bash

function build_arch {
  log=build-$1.log
  echo `date` > $log
#  echo "Building mxe.$1..." | tee -a $log
#  ./mxe.sh $1 >> $log 2>> $log
  echo "Building mpv.$1..." | tee -a $log
  ./mpv.sh $1 >> $log 2>> $log
  echo "Building baka-mplayer.$1..." | tee -a $log
  ./bakamplayer.sh $1 >> $log 2>> $log
  echo "Preparing build directories..." | tee -a $log
  mkdir -p build/$1
  echo "Compressing baka-mplayer.$1 with upx..." | tee -a $log
  upx "Baka-MPlayer.$1/build/baka-mplayer.exe" -o "build/$1/Baka MPlayer.exe" >> $log 2>> $log
  echo "Copying youtube-dl..."
  cp build/{youtube-dl.exe,$1/}
  cd "build/$1"
  echo "Compressing..."
  zip "../Baka-MPlayer.$1.zip" -r *
  echo "Completed baka-mplayer.$1." | tee -a $log
}

echo "Creating releases..."
mkdir -p build

build_arch i686 &
i686_pid=$!
build_arch x86_64 &
x86_64_pid=$!

if [ ! -e build/youtube-dl.exe ]; then
  echo "Downloading youtube-dl..."
  cd build && wget https://yt-dl.org/downloads/2015.01.25/youtube-dl.exe
fi

wait $i686_pid
wait $x86_64_pid

echo "Done"
