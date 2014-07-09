#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

// todo

#include <QSettings>

class SettingsManager : public QSettings
{
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = 0);
};

#endif // SETTINGSMANAGER_H
