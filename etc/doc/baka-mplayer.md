# Baka MPlayer User Manual

## Introduction

Baka MPlayer is a free and open source, cross-platform, **libmpv** based multimedia player.
Its simple design reflects the idea for an uncluttered, simple, and enjoyable environment for watching tv shows.

This manual is based on the status of the `master` branch (not the latest released version).


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


## Settings

On Windows, the config files are saved with the executable in an ini file `bakamplayer.ini`.
On Linux, they are saved in the Qt location `~/.config/bakamplayer.ini`.

	# Format: [category] key=value
	# Below are the keys and comments specify setting details
	
	[baka-mplayer]          # program options
	autoFit=                # autoFit percentage (0 = no autofit)
	debug=                  # debugging enabled (output box)
	hidePopup=              # hide tray icon notifications
	lang=                   # the language used by the program (auto selects from locale)
	lastcheck=              # last time we checked for updates
	maxRecent=              # the maximum files saved in recent
	onTop=                  # on top setting (always, never, or playing)
	recent=                 # recent file history
	remaining=              # display remaining time or duration time label
	screenshotDialog=       # always show the screenshot dialog when taking screenshots
	showAll=                # should we load files of different extensions
	splitter=               # the normal splitter position (playlist size)
	trayIcon=               # should we display the trayIcon
	version=                # the settings version (do not modify)

	[input]                 # input macros
	Ctrl+Q=baka quit # Quit # Shortcut=Command # Label
	                        # for command information see Commands section

	[mpv]                   # mpv specific options
	screenshot-format=      # format of mpv's screenshots
	speed=                  # speed of the video playback
	af=                     # audio filters
	volume=                 # volume
	                        # ...

Note that the `[mpv]` section is using mpv's options. See mpv's manual for a list of valid options.


## Commands

The command-line is handled on an argument basis by splitting commands up by spaces and getting more and more specific. The major command differentiation is between mpv and baka-mplayer.

Baka-MPlayer commands should be prefixed with `baka` while mpv commands are prefixed with `mpv`. eg. `baka quit` vs `mpv quit`  Both will quit but one told mpv to quit (which then ended baka), while the other told baka to quit (which ended mpv).

In general you'll want to use any available baka-mplayer command before you use an mpv command; this is because some aspects either are baka-mplayer specific or handle certain issues. For example: `baka volume +5` vs `mpv volume +5`; baka-mplayer will adjust the volume even if no video is playing and update mpv accordingly when it starts up; mpv volume will only work if a file is playing.

The following is a list of the available `baka` commands (for `mpv` commands see mpv's manual):

	new                             # creates a new instance of baka-mplayer
	open_location                   # shows the open location dialog
	open_clipboard                  # opens the clipboard
	show_in_folder                  # shows the current file in folder
	add_subtitles                   # add subtitles dialog
	screenshot [subtitles]          # take a screenshot (with subs if specified)
	media_info                      # toggles media info state
	stop                            # stops the current playback
	playlist <...>                  # playlist options (playlist ...)
	  play [index]                  #  plays the selected file (or (relative)? index)
	  remove                        #  removes the selected file from the playlist
	  shuffle                       #  shuffles the playlist
	  toggle                        #  toggles the playlist
	  full                          #  expands fully the playlist (hide album art)
	  select [index]                #  selects the index (can be relative) or currently playing file
	  repeat <off|this|playlist>    #  sets repeat state
	jump                            # opens jump dialog
	dim                             # toggles dim desktop
	output                          # toggles output textbox
	preferences                     # opens preferences dialog
	online_help                     # launches online help
	update [youtube-dl]             # opens the update dialog or updates youtube-dl
	open [file]                     # opens the open file dialog or the file specified
	play_pause                      # toggle play/pause state
	fitwindow [percent]             # fit the window
	volume [value]                  # adjusts the volume (can be relative)
	fullscreen                      # toggles fullscreen state
	boss                            # pause and hide the window--run away from boss
	help [command]                  # internal help menu
	about [qt]                      # open about dialog
	debug                           # toggle mpv debugging messages
	quit                            # quit baka-mplayer

More commands will be coming but please feel free to suggest modifications or additions.


## Translations

### To add a new translation

1. Update your local repo to make sure everything is up to date.
2. Open `src/translations.pri` with your favorite text editor.
3. Add a new line that says `translations/baka-mplayer_{lang-code}.ts` in the format that exists (all lines except the last require a `\`).
4. Run `./configure CONFIG+=begin_translations` to prepare your translation file.
5. Go to `src/translations/` and you will now find your language's `.ts` file.
6. Open the `.ts` file with Qt Linguist and proceed to translate into your language.
7. Upon completion of the translation, run `./configure CONFIG+=update_translations`. This will trim the .ts file to the minimum required information for release.

### To update an existing translation

1. Update your local repo to make sure everything is up to date.
2. Run `./configure CONFIG+=begin_translations` to regenerate the `.ts` files.
3. Make your changes.
4. Run `./configure CONFIG+=update_translations` to trim the `.ts` files.

If you want to submit a translation, you can create a git pull request or send it to us through irc.

For more information on Qt Linguist (the program used to translate Qt projects) see http://qt-project.org/doc/qt-5/qtlinguist-index.html
