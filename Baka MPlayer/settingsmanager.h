#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

// todo

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = 0);
    ~SettingsManager();


private:
    QSettings *settings;
};

#endif // SETTINGSMANAGER_H
