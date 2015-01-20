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
        {"Ctrl++",          {"mpv add sub-scale +0.02",   tr("Increase sub size")}},
        {"Ctrl+-",          {"mpv add sub-scale -0.02",   tr("Decrease sub size")}},
        {"Ctrl+R",          {"mpv set time-pos 0",        tr("Restart playback")}},
        {"Ctrl+Shift+Down", {"mpv add speed -0.25",       tr("Decrease speed by 25%")}},
        {"Ctrl+Shift+R",    {"mpv set speed 1",           tr("Reset speed")}},
        {"Ctrl+Shift+T",    {"mpv screenshot video",      tr("Take screenshot without subtitles")}},
        {"Ctrl+Shift+Up",   {"mpv add speed +0.25",       tr("Increase Playback Speed by 25%")}},
        {"Ctrl+T",          {"mpv screenshot subtitles",  tr("Take screenshot with subtitles")}},
        {"Ctrl+W",          {"mpv cycle sub-visibility",  tr("Toggle subtitle visibility")}},
        {"Left",            {"mpv seek -5",               tr("Seek backwards by 5 sec")}},
        {"PgUp",            {"mpv add chapter -1",        tr("Go to previous chapter")}},
        {"PgDown",          {"mpv add chapter +1",        tr("Go to next chapter")}},
        {"Right",           {"mpv seek +5",               tr("Seek forwards by 5 sec")}},
        {"Shift+Left",      {"mpv frame_back_step",       tr("Frame step backwards")}},
        {"Shift+Right",     {"mpv frame_step",            tr("Frame step")}},
        {"Ctrl+Down",       {"baka volume -5",            tr("Decrease volume")}},
        {"Ctrl+Up",         {"baka volume +5",            tr("Increase volume")}},
        {"Alt+Return",      {"baka fullscreen",           tr("Toggle fullscreen")}},
        {"Ctrl+D",          {"baka dim",                  tr("Dim Lights")}},
        {"Ctrl+E",          {"baka show_in_folder",       tr("Show the file in it's folder")}},
        {"Ctrl+I",          {"baka media_info",           tr("View Media Information")}},
        {"Ctrl+J",          {"baka jump",                 tr("Show Jump To Time dialog")}},
        {"Ctrl+Left",       {"baka play -1",              tr("Play previous file")}},
        {"Ctrl+N",          {"baka new",                  tr("Open a new window")}},
        {"Ctrl+O",          {"baka open",                 tr("Show Open File dialog")}},
        {"Ctrl+Q",          {"baka quit",                 tr("Quit")}},
        {"Ctrl+Right",      {"baka play +1",              tr("Play next file")}},
        {"Ctrl+S",          {"baka stop",                 tr("Stop playback")}},
        {"Ctrl+U",          {"baka open_location",        tr("Show Location dialog")}},
        {"Ctrl+V",          {"baka open_clipboard",       tr("Open clipboard location")}},
        {"Ctrl+F",          {"baka playlist toggle",      tr("Toggle Playlist visibility")}},
        {"Ctrl+Z",          {"baka open_recent 0",        tr("Open the last played file")}},
        {"Ctrl+G",          {"baka output",               tr("Access Baka-MPlayer Command-line")}},
        {"F1",              {"baka online_help",          tr("Launch Online Help")}},
        {"Space",           {"baka play_pause",           tr("Play/Pause")}},
        {"Alt+1",           {"baka fitwindow 0",          tr("Fit the Window to video")}},
        {"Alt+2",           {"baka fitwindow 50",         tr("Fit Window to 50%")}},
        {"Alt+3",           {"baka fitwindow 75",         tr("Fit Window to 75%")}},
        {"Alt+4",           {"baka fitwindow 100",        tr("Fit Window to 100%")}},
        {"Alt+5",           {"baka fitwindow 150",        tr("Fit Window to 150%")}},
        {"Alt+6",           {"baka fitwindow 200",        tr("Fit Window to 200%")}},
        {"Esc",             {"baka boss",                 tr("Boss Key")}},
        {"Up",              {"baka playlist select prev", tr("Select previous file on playlist")}},
        {"Down",            {"baka playlist select next", tr("Select next file on playlist")}},
        {"Return",          {"baka playlist play",        tr("Play selected file on playlist")}},
        {"Del",             {"baka playlist remove",      tr("Remove selected file from playlist")}}
    };


    // This is a baka-command hashtable initialized below
    //  by using a hash-table -> function pointer we acheive O(1) function lookups
    // Format: void BakaCommand(QStringList args)
    // See bakacommands.cpp for function definitions
    typedef void(BakaEngine::*BakaCommandFPtr)(QStringList&);
    const QHash<QString, BakaCommandFPtr> BakaCommandMap = {
        {"new", &BakaEngine::BakaNew},
        {"play", &BakaEngine::BakaPlay},
        {"open_location", &BakaEngine::BakaOpenLocation},
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
        {"debug", &BakaEngine::BakaDebug},
        {"quit", &BakaEngine::BakaQuit}
    };
    // Baka Command Functions
    void BakaNew(QStringList&);
    void BakaPlay(QStringList&);
    void BakaOpenLocation(QStringList&);
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
    void BakaDebug(QStringList&);
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
