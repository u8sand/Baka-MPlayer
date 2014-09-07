# Baka MPlayer User Manual
Version 1.0

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching anime.
It's also portable and supports opening Youtube videos (through **libmpv**).

This manual seeks to cover all related topics.


## Portability

Baka MPlayer is made to be cross-platform and portable--simple to put on your usb and use anywhere to play those video formats that no one else seems to be able to play. Though the linux version only works on the distribution it was compiled for, the windows version is compiled with mingw and should work in both all windows versions and on linux with wine.

Configuration files on the windows version are saved with the executable in an ini file `bakamplayer.ini`. On the linux version they are saved in the qt location `~/.config/bakamplayer.ini`


## Hotkeys

Most hotkeys are labeled on the file menu. Other hotkeys are described here.

	Left                    Seek backwards 5 seconds
	Right                   Seek forwards 5 seconds
	Esc                     Exit fullscreen mode or Boss Key (pause and minimize)
	MediaPlay               Triggers Play/Pause
	MediaStop               Triggers Stop
	MediaNext               Triggers Play Next File
	MediaPrevious           Triggers Play Previous File


## Settings

	# Format: [category] key=value
	# Below are the keys and comments specify setting details
	
	[mpv]                   # Mpv Related Options
	last-file=              # The full path of the last file opened
	screenshotDir=          # The directory where screenshots will be saved
	screenshotTemplate=     # The template of screenshot files see mpv's man
	screenshotFormat=       # The screenshot file format see mpv's man
	showAll=                # Should we load files of different extensions
	speed=                  # Playback speed multiplier (supports decimals)
	volume=                 # the volume from 0-100.
	
	[window]                # Window Related Options
	width=                  # width of the window
	height=                 # height of the window
	autoFit=                # autoFit percentage (0 = no autofit)
	hidePopup=              # hide tray icon notifications
	onTop=                  # on top setting (always, never, or playing)
	remaining=              # display remaining time or duration time label
	splitter=               # the normal splitter position (playlist size)
	trayIcon=               # should we display the trayIcon

	[common]                # Common Options
	debug=                  # debugging enabled (output box)
	
