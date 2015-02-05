#!/bin/bash

# usage:
#  just delete the directory of what you want rebuilt.

# this script keeps an untouched copy of the source tree which it updates with git pull
#  this is useful for preserving bandwidth (no need to do full clones for rebuilding anymore).

if [[ $1 == 'x86_64' ]]; then
	ARCH=x86_64
elif [[ $1 == 'i686' ]]; then
	ARCH=i686
else
	echo "Please specify either x86_64, i686 architecture.";
	exit;
fi

SRC=$(pwd)/src
BUILD=$(pwd)/build
RELEASE=$(pwd)/release.$ARCH
JOBS=`grep -c ^processor /proc/cpuinfo`

function download_mxe {
  cd "$SRC" && git clone https://github.com/mxe/mxe.git mxe
}

function download_mpv {
  cd "$SRC" && git clone https://github.com/mpv-player/mpv.git mpv
}

function download_baka {
  cd "$SRC" && git clone https://github.com/u8sand/Baka-MPlayer.git Baka-MPlayer
}

function download_youtube_dl {
  cd "$SRC" && wget https://yt-dl.org/downloads/2015.01.25/youtube-dl.exe
}

function update {
  echo "Updating $1..."
  cd "$SRC/$1" && git pull
}

function copy {
  cp -r "$SRC/$1" "$BUILD/$1.$ARCH"
}

function build_mxe {
  cd "$BUILD/mxe.$ARCH"
  echo "Building mxe..."
  echo "JOBS := $JOBS" > settings.mk
  echo "MXE_TARGETS := $ARCH-w64-mingw32.static" >> settings.mk
  make gcc ffmpeg libass lua jpeg pthreads qt5 libzip
}

function prepare_mxe_env {
  echo "Preparing mxe environment variables..."
  MXEROOT="$BUILD/mxe.$ARCH"
  export PATH="$MXEROOT/usr/bin:$PATH"
  export PKG_CONFIG_PATH="$MXEROOT/usr/$ARCH-w64-mingw32.static/lib/pkgconfig"
  unset `env | \
      grep -vi '^EDITOR=\|^HOME=\|^LANG=\|MXE\|^PATH=' | \
      grep -vi 'PKG_CONFIG\|PROXY\|^PS1=\|^TERM=' | \
      cut -d '=' -f1 | tr '\n' ' '`
}

function build_mpv {
  cd "$BUILD/mpv.$ARCH"
  echo "Building mpv..."
  ./bootstrap.py
  DEST_OS=win32 TARGET=$ARCH-w64-mingw32.static ./waf configure --enable-libmpv-static
  ./waf build -j $JOBS

  INSTROOT="$MXEROOT/usr/$ARCH-w64-mingw32.static"
  cp build/libmpv.a "$INSTROOT/lib"
  mkdir -p "$INSTROOT/include/mpv"
  cp libmpv/client.h "$INSTROOT/include/mpv/"
  cat build/libmpv/mpv.pc |
	  sed "s,^prefix=.*$,prefix=${INSTROOT},g" |
	  sed "s,^exec_prefix=.*$,exec_prefix=\${prefix},g" |
	  sed "s,^libdir=.*$,libdir=\${prefix}/lib,g" |
	  sed "s,^includedir=.*$,includedir=\${prefix}/include,g" > "$INSTROOT/lib/pkgconfig/mpv.pc"
}

function build_baka {
  cd "$BUILD/Baka-MPlayer.$ARCH"
  echo "Building Baka-MPlayer..."
  QTROOT="$MXEROOT/usr/$ARCH-w64-mingw32.static/qt5/bin"
  QMAKE="$QTROOT/qmake" \
    ./configure \
    "CONFIG+=embed_translations" \
    "lupdate=$QTROOT/lupdate" \
    "lrelease=$QTROOT/lrelease"
  make -j $JOBS
}

function build_release {
  echo "Creating release..."
  upx "$BUILD/Baka-MPlayer.$ARCH/build/baka-mplayer.exe" -o "$RELEASE/Baka MPlayer.exe"
  cp "$SRC/youtube-dl.exe" "$RELEASE"
  cd "$RELEASE" && zip "../Baka-MPlayer.$ARCH.zip" -r *
  rm -r "$RELEASE"
}


# check sources

if [ ! -d "$SRC" ]; then
  mkdir -p "$SRC"
fi

if [ ! -d "$SRC/mxe" ]; then
  echo "Downloading mxe..."
  download_mxe &
  mxe_pid=$!
fi

if [ ! -d "$SRC/mpv" ]; then
  echo "Downloading mpv..."
  download_mpv &
  mpv_pid=$!
fi

if [ ! -d "$SRC/Baka-MPlayer" ]; then
  echo "Downloading Baka-MPlayer..."
  download_baka &
  baka_pid=$!
fi

if [ ! -f "$SRC/youtube-dl.exe" ]; then
  echo "Downloading youtube-dl.exe..."
  download_youtube_dl &
  youtube_dl_pid=$!
fi


# check builds

if [ ! -d "$BUILD" ]; then
  mkdir -p "$BUILD"
fi

if [ ! -d "$BUILD/mxe.$ARCH" ]; then
  wait $mxe_pid
  update mxe
  copy mxe
  build_mxe
fi

wait $mxe_pid
prepare_mxe_env

if [ ! -d "$BUILD/mpv.$ARCH" ]; then
  wait $mpv_pid
  update mpv
  copy mpv
  build_mpv
fi

if [ ! -d "$BUILD/Baka-MPlayer.$ARCH" ]; then
  wait $baka_pid
  update "Baka-MPlayer"
  copy "Baka-MPlayer"
  build_baka
fi


# check releases

if [ ! -f "$PWD/Baka-MPlayer.$ARCH.zip" ]; then
  wait $youtube_dl_pid
  mkdir -p "$RELEASE"
  build_release
fi
