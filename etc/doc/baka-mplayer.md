# Baka MPlayer User Manual
Version 1.1

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.


## Portability

Baka MPlayer is made to be cross-platform and portable.
Although currently no Windows binaries exist, Windows compilation is in the scope of the project for the near future.

## Documentation

### Playlist

The playlist is populated by loading the files in the directory of the loaded  file. Items can be rearranged at will, removed and deleted.

Refreshing the playlist will recheck the directory (files that were deleted will still be there until playlist is refreshed).

Shuffling the playlist shuffles the actual contents of the list rather than jumping to a random index--this way you know exactly what will be playing next.

Clicking the Index allows you to type a specific index to play--this is particularly useful for playing a specific episode if all episodes are sorted by ascending order.

Typing into the search makes the list only show results that contain the search text.

### Sreenshots

Screenshots can be taken through the mpv engine with the relevant actions. The template used in the program is directly inserted into mpv as screenshot-template; see mpv's manual for more details on valid screenshot-template. Specified directories are prepended to the screenshot-template passed to mpv. The screenshot mostly needed initially can be hidden by unselecting the `Always show this dialog` option; to get the dialog back one must set the `baka-mplayer/screenshotDialog` to true.

### Recent Files

The last file viewed is stored as `last-file` in the settings for easy access.

The following is in the scope of the project for the near future.
*Your recently viewed files are stored in settings in settings for easy access. They will be seen in the settings file at the bottom under `[recent]` line by line as file paths. The program will maintain a `recent-length` length (also in the settings) to those files. (disabling this feature can be done by setting `recent-length` to 0; aquiring the old functionality can be done by setting `recent-length` to 1).*

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
	screenshotDialog=       # always show the screenshot dialog when taking screenshots
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
