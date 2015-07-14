## Settings

On Windows, the config files are saved with the executable in an ini file `bakamplayer.ini`.
On Linux, they are saved in the Qt location `~/.config/bakamplayer.ini`.

    # As of 2.0.3 we use JSON format:
    # Below are the keys and comments specify setting details
    {
      "autoFit": n,                # autoFit percentage (0 = no autofit)
      "debug": b,                  # debugging enabled (output box)
      "gestures": b,               # enable/disable gesture support
      "hidePopup": b,              # hide tray icon notifications
      "input": {                   # input macros
        "Ctrl+Q": [                # key-binding
          "baka quit",             # baka command (for command information see Commands section)
          "Quit"                   # label
        ],
        ...
      },
      "lang": "",                  # the language used by the program (auto selects from locale)
      "lastcheck": "",             # last time we checked for updates
      "maxRecent": n,              # the maximum files saved in recent
      "mpv": {                     # mpv specific options
        "screenshot-format": "",   # format of mpv's screenshots
        "speed": f                 # speed of the video playback
        "vf": ""                   # video filters
        "volume": n,               # volume
        ...
      },
      "onTop": "b",                # on top setting (always, never, or playing)
      "recent": [                  # recent file history
        {
          "path": "/file/path",
          "title": "The Title in case of Url!",
          "time": n
        },
        ...
      ],
      "remaining": b,              # display remaining time or duration time label
      "resume": b,                 # enable/disable auto resume from last time feature
      "screenshotDialog": b,       # always show the screenshot dialog when taking screenshots
      "showAll": n,                # should we load files of different extensions
      "splitter": n,               # the normal splitter position (playlist size)
      "trayIcon": b,               # should we display the trayIcon
      "version": "a.b.c"           # the settings version (do not modify)
    }

Note that the `[mpv]` section is using mpv's options. See mpv's manual for a list of valid options.

Json should be relatively self explanitory; it's simple tree format makes it very dynamic compared to the previous .ini format. Largely seen in `recent` and `input` parameters, json's syntax allowed us to save more complex objects with less crazy string parsing.