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
    deinterlace                     # toggles deinterlation
    interpolate                     # toggles vo interpolation
    mute                            # toggles mute
    volume [value]                  # adjusts the volume (can be relative)
    fullscreen                      # toggles fullscreen state
    hide_all_controls               # toggles hide all controls state
    boss                            # pause and hide the window--run away from boss
    clear                           # clears the output text
    help [command]                  # internal help menu
    about [qt]                      # open about dialog
    msg_level [level]               # set mpv debugging message level
    quit                            # quit baka-mplayer

More commands will be coming but please feel free to suggest modifications or additions.