#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent):
    QSettings("bakamplayer.ini", QSettings::IniFormat, parent)
{
    // todo: set default settings
}
