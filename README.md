# [Baka MPlayer](http://bakamplayer.u8sand.net)

## Note

This program is in its alpha stages. Until this notice is removed, expect bugs.

## Overview

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching anime.
It's also portable and supports opening Youtube videos (through **libmpv**).

## Installation

### Windows

#### Official Releases

Download archive with the binary for your system (64 bit or 32 bit) from
http://bakamplayer.u8sand.net/downloads.php

The downloaded file will be a `.7zip` file. You can use either 7zip or WinRAR to extract the files wherever you like--it works as a stand-alone executable. It's configuration file will be named `bakamplayer.ini`.

To treat it as a standard Windows program--extract it to `C:/Program Files/Baka MPlayer/` then make a shortcut on your desktop.

#### Development Release

See Linux instructions. If compilation goes successfully you'll have built `Baka-MPlayer.exe`.

### Linux

If your distribution does not provide a package--you must compile baka mplayer for your system. This requires libmpv (see https://github.com/mpv-player/mpv), qt5 (https://qt-project.org/), pkg-config, make, gcc, and git.

	git clone https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer"
	./make.sh
	
Copy the resulting executable (build/Baka MPlayer) wherever you like; (eg. `sudo cp "build/Baka MPlayer" /usr/bin/bakamplayer`) your configuration file will be written to `bakamplayer.ini`.

## Bug reports

Please use the [issues tracker](https://github.com/u8sand/Baka-MPlayer/issues) provided by GitHub to send us bug reports or
feature requests.
