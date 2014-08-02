#include "settingsmanager.h"

SettingsManager::SettingsManager(QString file, QObject *parent):
    QSettings(file, QSettings::IniFormat, parent)
{
}

SettingsManager::~SettingsManager()
{
}

