# Baka MPlayer User Manual
Version 1.0

## Introduction

Baka MPlayer is a free and open source, cross-platform, mpv based multimedia player. It's simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching anime. It also supports opening Youtube videos (through mpv), is portable, and has support for voice commands (in beta).

This manual seeks to cover all related topics.

## Installation

### Windows

Download archive with the binary for your system (64 bit or 32 bit) from
http://bakamplayer.u8sand.net/downloads.php

The downloaded file is a simple `.zip` file; extract it wherever you like--it works as a stand-alone executable. It's configuration will be written as `bakamplayer.ini`

To treat it as the rest of your programs--extract it to `C:/Program Files/Baka MPlayer/` then make a shortcut on your desktop.

### Linux

If your distribution does not provide a package--you must compile baka mplayer for your system. This requires libmpv (see https://github.com/mpv-player/mpv) and qt5 (https://qt-project.org/).

	git clone -b release https://github.com/u8sand/Baka-MPlayer.git
	cd "Baka-MPlayer/Baka MPlayer"
	qmake .
	make
	
Copy the resulting executable (Baka MPlayer) wherever you like; (eg. `sudo cp "Baka MPlayer" /usr/bin/bakamplayer`) your configuration file will be written to `~/.bakamplayer.ini`

## Hotkeys

## Settings

