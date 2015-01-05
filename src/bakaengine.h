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

class BakaEngine : public QObject
{
    Q_OBJECT
public:
    explicit BakaEngine(QObject *parent = 0);
    ~BakaEngine();

    MainWindow *window;
    MpvHandler *mpv;
    Settings *settings;

public slots:
    void LoadSettings();
    void SaveSettings();

    void Command(QString command);

protected slots:
    // Utility functions
    void BakaCommand(QStringList command);
    void BakaPrint(QString);
    void MpvPrint(QString);
    void InvalidCommand(QString);
    void InvalidParameter(QString);

    // Baka Command Functions
    void BakaAbout();
    void BakaAboutQt();
    void BakaQuit();

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
    // This is a baka-command hashtable initialized in the constructor
    //  by using a hash-table -> function pointer we acheive O(1) function lookups XD
    typedef void(BakaEngine::*BakaCommandFPtr)();
    const QHash<QString, BakaCommandFPtr> CommandMap;
};

#endif // BAKAENGINE_H
