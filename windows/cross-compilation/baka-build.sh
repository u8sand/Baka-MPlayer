#!/bin/bash

# usage:
#  just delete the directory of what you want rebuilt.
# this script keeps an untouched copy of the source tree which it updates with git pull
#  this is useful for preserving bandwidth (no need to do full clones for rebuilding anymore).

# for systems with python pointing to python3, many packages in mxe use python and expect python2
#  the easy way to get around this issue is by uncommenting out the line below.
#alias python=python2

if [[ $1 == 'x86_64' ]]; then
	ARCH=x86_64
elif [[ $1 == 'i686' ]]; then
	ARCH=i686
else
	echo "Please specify either x86_64, i686 architecture.";
	exit;
fi

DIR=`pwd`
SRC=$DIR/src
BUILD=$DIR/build
RELEASE=$DIR/release.$ARCH
JOBS=`grep -c ^processor /proc/cpuinfo`

function download_mxe {
  cd "$SRC" && git clone https://github.com/mxe/mxe.git mxe || return 1
  return 0
}

function download_mpv {
  cd "$SRC" && git clone https://github.com/mpv-player/mpv.git mpv || return 1
  return 0
}

function download_baka {
  cd "$SRC" && git clone https://github.com/u8sand/Baka-MPlayer.git Baka-MPlayer || return 1
  return 0
}

function download_patches { # todo
  mkdir -p "$SRC/patches" || return 1
  return 0
}

function download_extras {
  mkdir -p "$SRC/release"
  cd "$SRC/release"
  wget https://yt-dl.org/downloads/2015.01.25/youtube-dl.exe || return 1
  mkdir -p mpv
  cd mpv && wget https://raw.githubusercontent.com/lachs0r/mingw-w64-cmake/master/packages/mpv/mpv/fonts.conf || return 1
  mkdir -p fonts
  # todo: download fonts
}

function update {
  echo "Updating $1..."
  cd "$SRC/$1" && git pull || return 1
  return 0
}

function copy {
  cp -r "$SRC/$1" "$BUILD/$1.$ARCH" || return 1
  return 0
}

function do_patch {
  echo "Patching $1..."
  cd "$BUILD/$1.$ARCH"
  for p in "$SRC/patches/$1"*; do
    if [ -f "$p" ]; then
      ( patch -p1 < "$p" ) || return 1
    fi
  done
  return 0
}

function build_mxe {
  cd "$BUILD/mxe.$ARCH"
  echo "Building mxe..."
  echo "JOBS := $JOBS" > settings.mk
  echo "MXE_TARGETS := $ARCH-w64-mingw32.static" >> settings.mk
  make gcc ffmpeg libass lua jpeg pthreads qt5 libzip || return 1
  return 0
}

function prepare_mxe_env {
  echo "Preparing mxe environment variables..."
  MXEROOT=$BUILD/mxe.$ARCH
  export PATH="$MXEROOT/usr/bin:$PATH"
  export PKG_CONFIG_PATH="$MXEROOT/usr/$ARCH-w64-mingw32.static/lib/pkgconfig"
  unset `env | \
      grep -vi '^EDITOR=\|^HOME=\|^LANG=\|MXE\|^PATH=' | \
      grep -vi 'PKG_CONFIG\|PROXY\|^PS1=\|^TERM=' | \
      cut -d '=' -f1 | tr '\n' ' '`
  return 0
}

function build_mpv {
  cd "$BUILD/mpv.$ARCH"
  echo "Building mpv..."
  ./bootstrap.py
  DEST_OS=win32 TARGET=$ARCH-w64-mingw32.static \
    ./waf configure \
      --enable-libmpv-static || return 1
  ./waf build -j $JOBS || return 1

  INSTROOT=$MXEROOT/usr/$ARCH-w64-mingw32.static
  cp build/libmpv.a "$INSTROOT/lib"
  mkdir -p "$INSTROOT/include/mpv"
  cp libmpv/client.h "$INSTROOT/include/mpv/"
  cat build/libmpv/mpv.pc |
	  sed "s,^prefix=.*$,prefix=${INSTROOT},g" |
	  sed "s,^exec_prefix=.*$,exec_prefix=\${prefix},g" |
	  sed "s,^libdir=.*$,libdir=\${prefix}/lib,g" |
	  sed "s,^includedir=.*$,includedir=\${prefix}/include,g" > "$INSTROOT/lib/pkgconfig/mpv.pc"
	return 0
}

function build_baka {
  cd "$BUILD/Baka-MPlayer.$ARCH"
  echo "Building Baka-MPlayer..."
  QTROOT=$MXEROOT/usr/$ARCH-w64-mingw32.static/qt5/bin
  QMAKE="$QTROOT/qmake" \
    ./configure \
    "CONFIG+=embed_translations" \
    "lupdate=$QTROOT/lupdate" \
    "lrelease=$QTROOT/lrelease"
  make -j $JOBS || return 1
  return 0
}

function build_release {
  echo "Creating release..."
  mkdir -p "$RELEASE"
  upx "$BUILD/Baka-MPlayer.$ARCH/build/baka-mplayer.exe" -o "$RELEASE/Baka MPlayer.exe" ||
    cp "$BUILD/Baka-MPlayer.$ARCH/build/baka-mplayer.exe" "$RELEASE/Baka MPlayer.exe"
  # add release stuff
  cp -r "$SRC/release/"* "$RELEASE"
  # compress
  cd "$RELEASE" && zip "../Baka-MPlayer.$ARCH.zip" -r *
  rm -r "$RELEASE"
  return 0
}


# check sources

if [ ! -d "$SRC" ]; then
  mkdir -p "$SRC" "$SRC/release"
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

if [ ! -d "$SRC/patches" ]; then
  echo "Downloading patches..."
  download_patches &
  patches_pid=$!
fi

if [ ! -d "$SRC/release" ]; then
  download_extras
  extras_pid=$!
fi

# check builds

if [ ! -d "$BUILD" ]; then
  mkdir -p "$BUILD"
fi

if [ ! -d "$BUILD/mxe.$ARCH" ] &&
   [ wait $mxe_pid ] &&
   [ wait $patches_pid ]; then
  update mxe && copy mxe && do_patch mxe && build_mxe || exit 1
fi

wait $mxe_pid && prepare_mxe_env || exit 1

if [ ! -d "$BUILD/mpv.$ARCH" ] &&
   [ wait $mpv_pid ] &&
   [ wait $patches_pid ]; then
  update mpv && copy mpv && do_patch mpv && build_mpv || exit 1
fi

if [ ! -d "$BUILD/Baka-MPlayer.$ARCH" ] &&
   [ wait $baka_pid ] &&
   [ wait $patches_pid ]; then
  update "Baka-MPlayer" && copy "Baka-MPlayer" && do_patch "Baka-MPlayer" && build_baka || exit 1
fi

# check releases

if [ ! -f "$DIR/Baka-MPlayer.$ARCH.zip" ] &&
   [ wait $extras_pid ]; then
  build_release || exit 1
fi
