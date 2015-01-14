#ifndef BAKAENGINE_H
#define BAKAENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QFunctionPointer>

class MainWindow;
class MpvHandler;
class Settings;
class GestureHandler;
class UpdateDialog;

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
    void BakaHelp(QStringList&);
    void BakaAbout(QStringList&);
    void BakaQuit(QStringList&);
public:
    void Open(QString path = QString());
    void PlayPause();
    void FitWindow(int percent = 0, bool msg = true);

    void About(QString what = QString());
    void Quit();
};

#endif // BAKAENGINE_H
