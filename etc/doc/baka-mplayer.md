# Baka MPlayer User Manual
Version 1.1

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.


## Portability

Baka MPlayer is made to be cross-platform and portable.
Although the Linux version only works on the distribution it was compiled for, the Windows version is compiled with mingw and should run in any version of Windows and on Linux with Wine installed.


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

Note that for mpv specific options you can use mpv's options. See mpv's man for valid options.