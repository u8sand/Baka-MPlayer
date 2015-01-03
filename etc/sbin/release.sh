#!/bin/bash

function build_arch {
  log=build-$1.log
  echo `date` > $log
  echo "Building mpv.$1..." | tee -a $log
  ./mpv.sh $1 >> $log 2>> $log;
  echo "Building baka-mplayer.$1..." | tee -a $log
  ./bakamplayer.sh $1 >> $log 2>> $log;
  echo "Compressing baka-mplayer.$1 with upx..." | tee -a $log
  upx Baka-MPlayer.$1/build/baka-mplayer.exe -o baka-mplayer-$1.exe >> $log 2>> $log
}

build_arch i686 &
i686_pid=$!
build_arch x86_64 &
x86_64_pid=$!

wait $i686_pid
wait $x86_64_pid

echo "Completed"
