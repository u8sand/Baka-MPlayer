#ifndef BAKAENGINE_H
#define BAKAENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTranslator>

class MainWindow;
class MpvHandler;
class Settings;
class GestureHandler;
class OverlayHandler;
class UpdateManager;
class DimDialog;

class BakaEngine : public QObject
{
    Q_OBJECT
public:
    explicit BakaEngine(QObject *parent = 0);
    ~BakaEngine();

    MainWindow     *window;
    MpvHandler     *mpv;
    Settings       *settings;
    GestureHandler *gesture;
    OverlayHandler *overlay;
    UpdateManager  *update;
    DimDialog      *dimDialog;

    QSystemTrayIcon *sysTrayIcon;
    QMenu           *trayIconMenu;

    QTranslator     *translator,
                    *qtTranslator;

    // input hash-table provides O(1) input-command lookups
    QHash<QString, QPair<QString, QString>> input; // [shortcut] = QPair<command, comment>

    // the following are the default input bindings
    // they are loaded into the input before parsing the settings file
    // when saving the settings file we don't save things that appear here
    const QHash<QString, QPair<QString, QString>> default_input = {
        {"Ctrl++",          {"mpv add sub-scale +0.1",              tr("Increase sub size")}},
        {"Ctrl+-",          {"mpv add sub-scale -0.1",              tr("Decrease sub size")}},
        {"Ctrl+W",          {"mpv osd-msg cycle sub-visibility",    tr("Toggle subtitle visibility")}},
        {"Ctrl+R",          {"mpv set time-pos 0",                  tr("Restart playback")}},
        {"PgDown",          {"mpv add chapter +1",                  tr("Go to next chapter")}},
        {"PgUp",            {"mpv add chapter -1",                  tr("Go to previous chapter")}},
        {"Right",           {"mpv seek +5",                         tr("Seek forwards by 5 sec")}},
        {"Left",            {"mpv seek -5",                         tr("Seek backwards by 5 sec")}},
        {"Shift+Left",      {"mpv frame_back_step",                 tr("Frame step backwards")}},
        {"Shift+Right",     {"mpv frame_step",                      tr("Frame step")}},
        {"Ctrl+T",          {"screenshot subtitles",                tr("Take screenshot with subtitles")}},
        {"Ctrl+Shift+T",    {"screenshot",                          tr("Take screenshot without subtitles")}},
        {"Ctrl+Down",       {"volume -5",                           tr("Decrease volume")}},
        {"Ctrl+Up",         {"volume +5",                           tr("Increase volume")}},
        {"Ctrl+Shift+Up",   {"speed +0.1",                          tr("Increase playback speed by 10%")}},
        {"Ctrl+Shift+Down", {"speed -0.1",                          tr("Decrease playback speed by 10%")}},
        {"Ctrl+Shift+R",    {"speed 1.0",                           tr("Reset speed")}},
        {"Alt+Return",      {"fullscreen",                          tr("Toggle fullscreen")}},
        {"Ctrl+D",          {"dim",                                 tr("Dim lights")}},
        {"Ctrl+E",          {"show_in_folder",                      tr("Show the file in its folder")}},
        {"Tab",             {"media_info",                          tr("View media information")}},
        {"Ctrl+J",          {"jump",                                tr("Show jump to time dialog")}},
        {"Ctrl+N",          {"new",                                 tr("Open a new window")}},
        {"Ctrl+O",          {"open",                                tr("Show open file dialog")}},
        {"Ctrl+Q",          {"quit",                                tr("Quit")}},
        {"Ctrl+Right",      {"playlist play +1",                    tr("Play next file")}},
        {"Ctrl+Left",       {"playlist play -1",                    tr("Play previous file")}},
        {"Ctrl+S",          {"stop",                                tr("Stop playback")}},
        {"Ctrl+U",          {"open_location",                       tr("Show location dialog")}},
        {"Ctrl+V",          {"open_clipboard",                      tr("Open clipboard location")}},
        {"Ctrl+F",          {"playlist toggle",                     tr("Toggle playlist visibility")}},
        {"Ctrl+Z",          {"open_recent 0",                       tr("Open the last played file")}},
        {"Ctrl+G",          {"output",                              tr("Access command-line")}},
        {"F1",              {"online_help",                         tr("Launch online help")}},
        {"Space",           {"play_pause",                          tr("Play/Pause")}},
        {"Alt+1",           {"fitwindow",                           tr("Fit the window to the video")}},
        {"Alt+2",           {"fitwindow 50",                        tr("Fit window to 50%")}},
        {"Alt+3",           {"fitwindow 75",                        tr("Fit window to 75%")}},
        {"Alt+4",           {"fitwindow 100",                       tr("Fit window to 100%")}},
        {"Alt+5",           {"fitwindow 150",                       tr("Fit window to 150%")}},
        {"Alt+6",           {"fitwindow 200",                       tr("Fit window to 200%")}},
        {"Esc",             {"boss",                                tr("Boss key")}},
        {"Down",            {"playlist select +1",                  tr("Select next file on playlist")}},
        {"Up",              {"playlist select -1",                  tr("Select previous file on playlist")}},
        {"Return",          {"playlist play",                       tr("Play selected file on playlist")}},
        {"Del",             {"playlist remove",                     tr("Remove selected file from playlist")}}
    };

public slots:
    void LoadSettings();
    void SaveSettings();

    void Command(QString command);

protected slots:
    // Utility functions
    void Print(QString what, QString who = "baka");
    void InvalidCommand(QString);
    void InvalidParameter(QString);
    void RequiresParameters(QString);

    // Settings Loading
    void Load2_0_2();
    void Load2_0_1();
    void Load2_0_0();
    void Load1_9_9();

    void LoadBaka2_0_2();
    void LoadBaka2_0_1();
    void LoadBaka2_0_0();
    void LoadBaka1_9_9();

    void LoadMpv2_0_0();
    void LoadMpv1_9_9();

    void LoadInput2_0_2();

signals:


private:
    // This is a baka-command hashtable initialized below
    //  by using a hash-table -> function pointer we acheive O(1) function lookups
    // Format: void BakaCommand(QStringList args)
    // See bakacommands.cpp for function definitions

    // todo: write advanced information about commands
    typedef void(BakaEngine::*BakaCommandFPtr)(QStringList&);
    const QHash<QString, QPair<BakaCommandFPtr, QStringList>> BakaCommandMap = {
        {"mpv", // command
         {&BakaEngine::BakaMpv,
          {
           // params     description
           QString(), tr("executes mpv command"),
           QString() // advanced
          }
         }
        },
        {"sh",
         {&BakaEngine::BakaSh,
          {
           QString(), tr("executes system shell command"),
           QString()
          }
         }
        },
        {"new",
         {&BakaEngine::BakaNew, // function pointer to command functionality
          {
           // params     description
           QString(), tr("creates a new instance of baka-mplayer"),
           QString()
          }
         }
        },
        {"open_location",
         {&BakaEngine::BakaOpenLocation,
          {
           QString(),
           tr("shows the open location dialog"),
           QString()
          }
         }
        },
        {"open_clipboard",
         {&BakaEngine::BakaOpenClipboard,
          {
           QString(),
           tr("opens the clipboard"),
           QString()
          }
         }
        },
        {"show_in_folder",
         {&BakaEngine::BakaShowInFolder,
          {
           QString(),
           tr("shows the current file in folder"),
           QString()
          }
         }
        },
        {"add_subtitles",
         {&BakaEngine::BakaAddSubtitles,
          {
           QString(),
           tr("add subtitles dialog"),
           QString()
          }
         }
        },
        {"screenshot",
         {&BakaEngine::BakaScreenshot,
          {
           tr("[subs]"),
           tr("take a screenshot (with subtitles if specified)"),
           QString()
          }
         }
        },
        {"media_info",
         {&BakaEngine::BakaMediaInfo,
          {
           QString(),
           tr("toggles media info display"),
           QString()
          }
         }
        },
        {"stop",
         {&BakaEngine::BakaStop,
          {
           QString(),
           tr("stops the current playback"),
           QString()
          }
         }
        },
        {"playlist",
         {&BakaEngine::BakaPlaylist,
          {
           tr("[...]"),
           tr("playlist options"),
           QString()
          }
         }
        },
        {"jump",
         {&BakaEngine::BakaJump,
          {
           QString(),
           tr("opens jump dialog"),
           QString()
          }
         }
        },
        {"dim",
         {&BakaEngine::BakaDim,
          {
           QString(),
           tr("toggles dim desktop"),
           QString()
          }
         }
        },
        {"output",
         {&BakaEngine::BakaOutput,
          {
           QString(),
           tr("toggles output textbox"),
           QString()
          }
         }
        },
        {"preferences",
         {&BakaEngine::BakaPreferences,
          {
           QString(),
           tr("opens preferences dialog"),
           QString()
          }
         }
        },
        {"online_help",
         {&BakaEngine::BakaOnlineHelp,
          {
           QString(),
           tr("launches online help"),
           QString()
          }
         }
        },
        {"update",
         {&BakaEngine::BakaUpdate,
          {
           QString(),
           tr("opens the update dialog or updates youtube-dl"),
           QString()
          }
         }
        },
        {"open",
         {&BakaEngine::BakaOpen,
          {
           tr("[file]"),
           tr("opens the open file dialog or the file specified"),
           QString()
          }
         }
        },
        {"play_pause",
         {&BakaEngine::BakaPlayPause,
          {
           QString(),
           tr("toggle play/pause state"),
           QString()
          }
         }
        },
        {"fitwindow",
         {&BakaEngine::BakaFitWindow,
          {
           tr("[percent]"),
           tr("fit the window"),
           QString()
          }
         }
        },
        {"volume",
         {&BakaEngine::BakaVolume,
          {
           tr("[level]"),
           tr("adjusts the volume"),
           QString()
          }
         }
        },
        {"speed",
         {&BakaEngine::BakaSpeed,
          {
           tr("[ratio]"),
           tr("adjusts the speed"),
           QString()
          }
         }
        },
        {"fullscreen",
         {&BakaEngine::BakaFullScreen,
          {
           QString(),
           tr("toggles fullscreen state"),
           QString()
          }
         }
        },
        {"boss",
         {&BakaEngine::BakaBoss,
          {
           QString(),
           tr("pause and hide the window"),
           QString()
          }
         }
        },
        {"help",
         {&BakaEngine::BakaHelp,
          {
           tr("[command]"),
           tr("internal help menu"),
           QString()
          }
         }
        },
        {"about",
         {&BakaEngine::BakaAbout,
          {
           tr("[qt]"),
           tr("open about dialog"),
           QString()
          }
         }
        },
        {"debug",
         {&BakaEngine::BakaDebug,
          {
           tr("[level]"),
           tr("set mpv debugging level"),
           QString()
          }
         }
        },
        {"quit",
         {&BakaEngine::BakaQuit,
          {
           QString(),
           tr("quit baka-mplayer"),
           QString()
          }
         }
        }
    };
    // Baka Command Functions
    void BakaMpv(QStringList&);
    void BakaSh(QStringList&);
    void BakaNew(QStringList&);
    void BakaOpenLocation(QStringList&);
    void BakaOpenClipboard(QStringList&);
    void BakaShowInFolder(QStringList&);
    void BakaAddSubtitles(QStringList&);
    void BakaScreenshot(QStringList&);
    void BakaMediaInfo(QStringList&);
    void BakaStop(QStringList&);
    void BakaPlaylist(QStringList&);
    void BakaJump(QStringList&);
    void BakaDim(QStringList&);
    void BakaOutput(QStringList&);
    void BakaPreferences(QStringList&);
    void BakaOnlineHelp(QStringList&);
    void BakaUpdate(QStringList&);
    void BakaOpen(QStringList&);
    void BakaPlayPause(QStringList&);
    void BakaFitWindow(QStringList&);
    void BakaAspect(QStringList&);
    void BakaVolume(QStringList&);
    void BakaSpeed(QStringList&);
    void BakaFullScreen(QStringList&);
    void BakaBoss(QStringList&);
    void BakaHelp(QStringList&);
    void BakaAbout(QStringList&);
    void BakaDebug(QStringList&);
    void BakaQuit(QStringList&);
public:
    void Open();
    void OpenLocation();
    void Screenshot(bool subs);
    void MediaInfo(bool show);
    void PlayPause();
    void Jump();
    void FitWindow(int percent = 0, bool msg = true);
    void Dim(bool dim);
    void About(QString what = QString());
    void Quit();
};

#endif // BAKAENGINE_H
