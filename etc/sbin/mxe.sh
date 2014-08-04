#!/bin/bash

# for mxe related information and dependencies and such see http://mxe.cc/
# dependencies: http://mxe.cc/#requirements
# this is big and takes a long time; recommend doing it in RAM if you have enough
#  I have 16GB RAM, intel core i7-980x
#   doing it with 12 threads took about 4.5gb ram at most + ~3gb
#   for the mxe folder so 8gb is probably just a little bit too little
#		though with less threads might be just enough.

git clone https://github.com/mxe/mxe.git
cd mxe
# set jobs optimal for your computer
echo "JOBS := 12" >> settings.mk
# i recommend you only specify the mxe target for the architecture you want to compile for
# took 1 hour each on my system. (remove the one you don't want in the line bellow
echo "MXE_TARGETS := x86_64-w64-mingw32 i686-w64-mingw32" >> settings.mk
make gcc ffmpeg libass jpeg pthreads qt5
cd ..
