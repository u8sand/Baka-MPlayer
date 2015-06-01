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