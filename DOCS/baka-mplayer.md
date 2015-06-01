# Baka MPlayer User Manual

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.

## Features

### Playlist

The playlist is automatically populated by loading the files in the directory of the loaded file. If a folder is opened by drag and dropping it, the files in the immediate folder directory will be automatically populated (not including subdirectories).

### Gestures

Using a mouse or a touch screen, you can click and drag on the video to change playback.
Clicking and dragging left/right will seek backwards/forwards.
Clicking and dragging up/down will increase/decrease volume.
Gesture support can be disabled under Preferences if desired.

### Screenshots

The screenshot template is passed into mpv as `screenshot-template` (see mpv's manual for more info). If the screenshot dialog is hidden by unchecking the `Always show this dialog` option, the dialog can be restored by setting `baka-mplayer/screenshotDialog` to true in the settings file.

### Recent Files

Your recently viewed files are stored in settings for easy access. It is located at the bottom of the file under `[baka-mplayer] -> recent`. To store more recently viewed files, set the `[baka-mplayer] -> maxRecent` value and the list will trim itself accordingly.


## Special functions

Some functionality that is not explained or is immediately obvious is written here.

	Hotkeys
		Left                    Seek backwards 5 seconds
		Right                   Seek forwards 5 seconds
		Esc                     Exit fullscreen mode or Boss Key (pause and minimize)
		MediaPlay               Triggers Play/Pause
		MediaStop               Triggers Stop
		MediaNext               Triggers Play Next File
		MediaPrevious           Triggers Play Previous File
	Clicking and Dragging
		the main window         Triggers gesture or Moves the main window
	Right Clicking the video
		In Fullscreen           Shows the menubar actions in context menu form
		Outside of Fullscreen   Triggers Play/Pause
	Quick Open Button
		Left Click - Show the open file dialog
		Middle Click - Jump to a specific time
		Right Click - Open a location (including URLs)

Most of these features are customizable through preferences, including key-binding customization and feature toggles.