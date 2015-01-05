#ifndef BAKAENGINE_H
#define BAKAENGINE_H

#include <QObject>
#include <QStringList>

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
    void BakaCommand(QStringList command);

    void BakaPrint(QString);
    void MpvPrint(QString);
    void InvalidCommand(QString);
    void InvalidParameter(QString);

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

public slots:
};

#endif // BAKAENGINE_H
