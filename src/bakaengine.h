#ifndef BAKAENGINE_H
#define BAKAENGINE_H

#include <QObject>

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

    void LoadSettings();
    void SaveSettings();

protected:
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

signals:

public slots:
};

#endif // BAKAENGINE_H
