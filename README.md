# [Baka MPlayer](http://bakamplayer.u8sand.net)

## Note

This program is in its alpha stages. Until this notice is removed, expect bugs.

## Overview

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching anime.

## Installation

### Windows

#### Official Releases

The official Windows builds will be available soon.

#### Development Release

See Linux instructions. If compilation goes successfully you'll have built `Baka-MPlayer.exe`.

### Linux

If your distribution does not provide a package--you must compile baka mplayer for your system. This requires libmpv (see https://github.com/mpv-player/mpv), qt5 (https://qt-project.org/), pkg-config, make, gcc, and git.

	git clone -b release https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer"
	./make.sh
	
Copy the resulting executable (build/Baka MPlayer) wherever you like; (eg. `sudo cp "build/Baka MPlayer" /usr/bin/bakamplayer`) your configuration file will be written to `bakamplayer.ini`.

## Bug reports

Please use the [issues tracker](https://github.com/u8sand/Baka-MPlayer/issues) provided by GitHub to send us bug reports or
feature requests.

## Contact

**IRC Channel**: `#baka-mplayer` on `irc.freenode.net`

You can ask us questions about using Baka MPlayer, give feedback, or discuss its development.
However, if possible, please avoid posting bugs there and use the [issue tracker](https://github.com/u8sand/Baka-MPlayer/issues) instead.
