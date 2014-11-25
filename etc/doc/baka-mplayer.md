# Baka MPlayer User Manual
Version 1.1

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.

## Portability

Baka MPlayer is made to be cross-platform and portable.
Although no Windows builds currently exist, work is being done to build for Windows.

## Documentation

### Playlist

The playlist is populated by loading the files in the directory of the loaded file. Items can be rearranged, removed, and deleted from disk.

Refreshing the playlist will recheck the directory (files that were deleted will still be there until playlist is refreshed).

Clicking the Index allows you to type a specific index to play--this is particularly useful for playing a specific episode if all episodes are sorted by ascending order.

### Sreenshots

Screenshots are taken through mpv. The template specified is passed into mpv as screenshot-template; see mpv's manual for more details on valid screenshot-template. Specified directories are prepended to the screenshot-template. If the screenshot dialog is hidden by unchecking the `Always show this dialog` option, the dialog can be restored by setting `baka-mplayer/screenshotDialog` to true in the settings file.

### Recent Files
Your recently viewed files are stored in settings for easy access. They will be seen in the settings file at the bottom as `recent/files`. To make the recent history bigger or smaller simply set `recent/max` and the list will trim itself accordingly.

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
		In Fullscreen           Shows the menubar actions in context menu form
		Outside of Fullscreen   Triggers Play/Pause
	Open Button
		Left Click - Show the open file dialog
		Middle Click - Jump to a specific time
		Right Click - Open a location (including URLs)

*Customization of the key bindings will be implemented in the near future.

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
	                        
	[recent]                # recent file history
	files=                  # the files
	max=                    # the maximum files saved in recent

Note that for mpv specific options you can use mpv's options. See mpv's manual for valid options.
