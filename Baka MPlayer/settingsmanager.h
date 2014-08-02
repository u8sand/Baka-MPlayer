#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

// todo

#include <QSettings>

class SettingsManager : public QSettings
{
    Q_OBJECT
public:
    explicit SettingsManager(QString file, QObject *parent = 0);
    ~SettingsManager();

private:
};

#endif // SETTINGSMANAGER_H
