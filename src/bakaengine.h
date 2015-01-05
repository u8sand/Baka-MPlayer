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
        {"play_pause", &BakaEngine::BakaPlayPause},
        {"volume", &BakaEngine::BakaVolume},
        {"help", &BakaEngine::BakaHelp},
        {"about", &BakaEngine::BakaAbout},
        {"quit", &BakaEngine::BakaQuit}
    };
    // Baka Command Functions
    void BakaPlayPause(QStringList&);
    void BakaVolume(QStringList&);
    void BakaHelp(QStringList&);
    void BakaAbout(QStringList&);
    void BakaQuit(QStringList&);
};

#endif // BAKAENGINE_H
