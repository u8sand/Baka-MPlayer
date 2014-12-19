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

TODO

### Linux

If your distribution does not provide a package, you can compile from source.

	git clone -b release https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer"
	./make.sh

The executable will be built into the `build` folder. You can then install it to your system by copying it to the `/usr/bin/` folder. The configuration file will be created on first run and will be written to `~/.config/bakamplayer.ini`.

## Bug reports

Please use the [issues tracker](https://github.com/u8sand/Baka-MPlayer/issues) provided by GitHub to send us bug reports or feature requests.

## Contact

**IRC Channel**: `#baka-mplayer` on `irc.freenode.net`

You can ask us questions about using Baka MPlayer, give feedback, or discuss its development.
However, if possible, please avoid posting bugs there and use the [issue tracker](https://github.com/u8sand/Baka-MPlayer/issues) instead.
