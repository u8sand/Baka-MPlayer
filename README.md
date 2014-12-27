# [Baka MPlayer](http://bakamplayer.u8sand.net)

## Overview

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.

## Requirements

* gcc
* pkg-config
* libmpv-dev
* qtbase5-dev (>= 5.2.0)
  * qt5-qmake
  * qtdeclarative5-dev
  * libqt5svg5-dev
  * libqt5x11extras5-dev
  * libqt5network5

## Compilation

### Windows

These instructions are for cross-compiling for Windows on a Linux system. (Note: the architecture can be either `x86_64` or `i686` depending on which platform you're compiling for)

	arch=x86_64
	git clone -b release https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer"
	mkdir build
	cp etc/sbin/* build/
	cd build
	./mxe.sh $arch
	./mpv.sh $arch
	./bakamplayer.sh $arch

This is a very long process because you'll need to build the mingw32 toolchain `mxe` and all dependent libraries, `libmpv.a`, and finally `baka-mplayer.exe`. If everything succeeded without error, `baka-mplayer.exe` will be built under `Baka-MPlayer.x86_64/build/`

The next time you build the executable it will be as simple as

	arch=x86_64
	./mpv.sh $arch
	./bakamplayer.sh $arch

These scripts will pull the new version and compile them again using the existing mxe toolchain. To update your mxe toolchain, use `./mxe.sh $arch`.

### Linux

If your distribution does not provide a package, you can compile it from source.

	git clone -b release https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer"
	./configure
	make -j `grep -c ^processor /proc/cpuinfo`
	make install

The configuration file will be created on first run and will be written to `~/.config/bakamplayer.ini`.

### Other Languages

By default, Baka MPlayer will compile in English if no language is specified during compilation. To compile a multi-lingual version of baka-mplayer, configure it like so:

	./configure CONFIG+=embed_translations

For more configuration options see the `configure` source file or read the manual.

You can check out which languages we currently support by checking out `Baka-MPlayer/src/translations/`.

## Bug reports

Please use the [issues tracker](https://github.com/u8sand/Baka-MPlayer/issues) provided by GitHub to send us bug reports or feature requests.

## Contact

**IRC Channel**: `#baka-mplayer` on `irc.freenode.net`

You can ask us questions about using Baka MPlayer, give feedback, or discuss its development.
However, if possible, please avoid posting bugs there and use the [issue tracker](https://github.com/u8sand/Baka-MPlayer/issues) instead.
