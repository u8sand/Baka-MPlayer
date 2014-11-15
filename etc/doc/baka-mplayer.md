# Baka MPlayer User Manual
Version 1.1

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.


## Portability

Baka MPlayer is made to be cross-platform and portable.
Although currently no Windows binaries exist, Windows compilation is in the scope of the project for the near future.

## Baka-MPlayer Documentation

### Special functions
Some functionality is not entirely explained to the user; this is explained here.

	Hotkeys
		Left                    Seek backwards 5 seconds
		Right                   Seek forwards 5 seconds
		Esc                     Exit fullscreen mode or Boss Key (pause and minimize)
		MediaPlay               Triggers Play/Pause
		MediaStop               Triggers Stop
		MediaNext               Triggers Play Next File
		MediaPrevious           Triggers Play Previous File
	Clicking and Dragging
		the main window         Move the main window
	Right Clicking the video
		In Fullscreen           Context menu with options that were available in the menubar
		Outside of Fullscreen   Play/Pause the video
	Open Button
		Left Click - Open a local file (File Dialog)
		Middle Click - Jump to a specific time in a file
		Right Click - Open a location (URL Dialog)

*Though customization of key bindings is not yet possible, it is in the scope of the project for the near future.

### Settings

On Windows, the config files are saved with the executable in an ini file `bakamplayer.ini`.
On Linux, they are saved in the Qt location `~/.config/bakamplayer.ini`.

	# Format: [category] key=value
	# Below are the keys and comments specify setting details
	
	[baka-mplayer]          # program options
	autoFit=                # autoFit percentage (0 = no autofit)
	debug=                  # debugging enabled (output box)
	height=                 # height of the window
	hidePopup=              # hide tray icon notifications
	last-file=              # The full path of the last file opened
	onTop=                  # on top setting (always, never, or playing)
	remaining=              # display remaining time or duration time label
	showAll=                # Should we load files of different extensions
	splitter=               # the normal splitter position (playlist size)
	trayIcon=               # should we display the trayIcon
    version=                # the settings version (do not modify)
	width=                  # width of the window

	[mpv]                   # mpv specific options
	screenshot-format=      # format of mpv's screenshots
	speed=                  # speed of the video playback
	af=                     # audio filters
	volume=                 # volume
	                        # ...

Note that for mpv specific options you can use mpv's options. See mpv's manual for valid options.

### Playlist

The Baka-MPlayer playlist is populated by loading the files in the directory of the loaded  file. Items can be rearranged at will, removed and deleted.

Refreshing the playlist will recheck the directory (files that were deleted will still be there until playlist is refreshed).

Shuffling the playlist shuffles the actual contents of the list rather than jumping to a random index--this way you know exactly what will be playing next.

Clicking the Index allows you to type a specific index to play--this is particularly useful for playing a specific episode if all episodes are sorted by ascending order.

Typing into the search makes the list only show results that contain the search text.
