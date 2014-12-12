#!/bin/bash

# for mxe related information and dependencies and such see http://mxe.cc/
# dependencies: http://mxe.cc/#requirements
# this is big and takes a long time; recommend doing it in RAM if you have enough
#  I have 16GB RAM, intel core i7-980x
#   doing it with 12 threads took about 4.5gb ram at most + ~3gb
#   for the mxe folder so 8gb is probably just a little bit too little
#		though with less threads might be just enough.

arch=x86_64
jobs=12
if [[ $1 == 'x86_64' ]]; then
	arch=x86_64
elif [[ $1 == 'i686' ]]; then
	arch=i686
else
	echo "Please specify either x86_64 or i686 architecture.";
	exit;
fi

if [[ $2 != '' ]]; then
	jobs=$2
fi

git clone https://github.com/mxe/mxe.git mxe.$arch
cd mxe.$arch
git pull
# set jobs optimal for your computer
echo "JOBS := $jobs" > settings.mk
# this took 1 hour each on my system.
echo "MXE_TARGETS := $arch-w64-mingw32.static" >> settings.mk

make gcc ffmpeg libass lua jpeg pthreads qt5
cd ..
