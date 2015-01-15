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
class UpdateDialog;
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
    UpdateDialog   *updateDialog;
    DimDialog      *dimDialog;

    QSystemTrayIcon *sysTrayIcon;
    QMenu           *trayIconMenu;

    QTranslator     *translator,
                    *qtTranslator;

    // input hash-table provides O(1) input-command lookups
    QHash<QString, QPair<QString, QString>> input; // [shortcut] = QPair<command, comment>

public slots:
    void LoadSettings();
    void SaveSettings();

    void Command(QString command);

protected slots:
    // Utility functions
    void MpvCommand(QStringList&);
    void BakaCommand(QStringList&);
    void BakaPrint(QString);
    void MpvPrint(QString);
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
    // the following are the default input bindings
    // they are loaded into the input before parsing the settings file
    // when saving the settings file we don't save things that appear here
    const QHash<QString, QPair<QString, QString>> default_input = {
        {"Ctrl++",          {"mpv add sub-scale +0.02",   tr("Increase Font Size")}},
        {"Ctrl+-",          {"mpv add sub-scale -0.02",   tr("Decrease Font Size")}},
        {"Ctrl+R",          {"mpv set time-pos 0",        tr("Restart Video Playback")}},
        {"Ctrl+Shift+Down", {"mpv add speed -0.25",       tr("Decrease Playback Speed by 25%")}},
        {"Ctrl+Shift+R",    {"mpv set speed 1",           tr("Reset Playback Speed")}},
        {"Ctrl+Shift+T",    {"mpv screenshot video",      tr("Take a screenshot without subtitles")}},
        {"Ctrl+Shift+Up",   {"mpv add speed +0.25",       tr("Increase Playback Speed by 25%")}},
        {"Ctrl+T",          {"mpv screenshot subtitles",  tr("Take a screenshot with subtitles")}},
        {"Ctrl+W",          {"mpv toggle sub-visibility", tr("Toggle Subtitle Visibility")}},
        {"Left",            {"mpv seek -5",               tr("Short Seek Backwards")}},
        {"PgDown",          {"mpv add chapter -1",        tr("Jump to the Previous Chapter")}},
        {"PgUp",            {"mpv add chapter +1",        tr("Jump to the Next Chapter")}},
        {"Right",           {"mpv seek +5",               tr("Short Seek Forward")}},
        {"Shift+Left",      {"mpv frame_back_step",       tr("Frame Back-Step")}},
        {"Shift+Right",     {"mpv frame_step",            tr("Frame Step")}},
        {"Ctrl+Down",       {"baka volume -5",            tr("Decrease the Volume")}},
        {"Ctrl+Up",         {"baka volume +5",            tr("Increase the Volume")}},
        {"Alt+Return",      {"baka fullscreen",           tr("Toggle Fullscreen Mode")}},
        {"Ctrl+D",          {"baka dim",                  tr("Dim the lights")}},
        {"Ctrl+X",          {"baka show_in_folder",       tr("Show the file in it's folder")}},
        {"Ctrl+I",          {"baka media_info",           tr("View Media Information")}},
        {"Ctrl+J",          {"baka jump",                 tr("Open the Jump To Time Dialog")}},
        {"Ctrl+Left",       {"baka play -1",              tr("Play the Previous Video")}},
        {"Ctrl+N",          {"baka new",                  tr("Create a new Baka-MPlayer window")}},
        {"Ctrl+O",          {"baka open",                 tr("Open a file Dialog")}},
        {"Ctrl+Q",          {"baka quit",                 tr("Quit the Baka-MPlayer")}},
        {"Ctrl+Right",      {"baka play +1",              tr("Play the Next Video")}},
        {"Ctrl+S",          {"baka stop",                 tr("Stop Video")}},
        {"Ctrl+U",          {"baka open_url",             tr("Open a URL Dialog")}},
        {"Ctrl+V",          {"baka open_clipboard",       tr("Open the file in your Clipboard")}},
        {"Ctrl+X",          {"baka playlist toggle",      tr("Toggle the Playlist Visibility")}},
        {"Ctrl+Z",          {"baka open_recent 0",        tr("Open the previous file")}},
        {"Ctrl+G",          {"baka output",               tr("Access Baka-MPlayer Command-line")}},
        {"F1",              {"baka online_help",          tr("Launch Online Help")}},
        {"Space",           {"baka play_pause",           tr("Play/Pause Video")}},
        {"Alt+1",           {"baka fitwindow 0",          tr("Fit the Window to the video frame")}},
        {"Alt+2",           {"baka fitwindow 50",         tr("Fit the Window to 50% of the Original Size")}},
        {"Alt+3",           {"baka fitwindow 75",         tr("Fit the Window to 75% of the Original Size")}},
        {"Alt+4",           {"baka fitwindow 100",        tr("Fit the Window to 100% of the Original Size")}},
        {"Alt+5",           {"baka fitwindow 150",        tr("Fit the Window to 150% of the Original Size")}},
        {"Alt+6",           {"baka fitwindow 200",        tr("Fit the Window to 200% of the Original Size")}},
        {"Esc",             {"baka boss",                 tr("Hide Baka-MPlayer from Your Boss")}},
        {"Up",              {"baka playlist select prev", tr("Select the Previous Item on the Playlist")}},
        {"Down",            {"baka playlist select next", tr("Select the Next Item on the Playlist")}},
        {"Return",          {"baka playlist play",        tr("Play Selected Item on Playlist")}},
        {"Del",             {"baka playlist remove",      tr("Remove Selected Item from Playlist")}}
    };


    // This is a baka-command hashtable initialized below
    //  by using a hash-table -> function pointer we acheive O(1) function lookups
    // Format: void BakaCommand(QStringList args)
    // See bakacommands.cpp for function definitions
    typedef void(BakaEngine::*BakaCommandFPtr)(QStringList&);
    const QHash<QString, BakaCommandFPtr> BakaCommandMap = {
        {"new", &BakaEngine::BakaNew},
        {"open_url", &BakaEngine::BakaOpenUrl},
        {"open_clipboard", &BakaEngine::BakaOpenClipboard},
        {"show_in_folder", &BakaEngine::BakaShowInFolder},
        {"add_subtitles", &BakaEngine::BakaAddSubtitles},
        {"media_info", &BakaEngine::BakaMediaInfo},
        {"stop", &BakaEngine::BakaStop},
        {"playlist", &BakaEngine::BakaPlaylist},
        {"jump", &BakaEngine::BakaJump},
        {"dim", &BakaEngine::BakaDim},
        {"output", &BakaEngine::BakaOutput},
        {"preferences", &BakaEngine::BakaPreferences},
        {"online_help", &BakaEngine::BakaOnlineHelp},
        {"update", &BakaEngine::BakaUpdate},
        {"open", &BakaEngine::BakaOpen},
        {"play_pause", &BakaEngine::BakaPlayPause},
        {"fitwindow", &BakaEngine::BakaFitWindow},
        {"volume", &BakaEngine::BakaVolume},
        {"fullscreen", &BakaEngine::BakaFullScreen},
        {"boss", &BakaEngine::BakaBoss},
        {"help", &BakaEngine::BakaHelp},
        {"about", &BakaEngine::BakaAbout},
        {"quit", &BakaEngine::BakaQuit}
    };
    // Baka Command Functions
    void BakaNew(QStringList&);
    void BakaOpenUrl(QStringList&);
    void BakaOpenClipboard(QStringList&);
    void BakaShowInFolder(QStringList&);
    void BakaAddSubtitles(QStringList&);
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
    void BakaFullScreen(QStringList&);
    void BakaBoss(QStringList&);
    void BakaHelp(QStringList&);
    void BakaAbout(QStringList&);
    void BakaQuit(QStringList&);
public:
    void Open(QString path = QString());
    void PlayPause();
    void FitWindow(int percent = 0, bool msg = true);
    void Dim(bool dim);
    void About(QString what = QString());
    void Quit();
};

#endif // BAKAENGINE_H
