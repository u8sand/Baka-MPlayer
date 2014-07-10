#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent):
    QObject(parent),
    settings(new QSettings("bakamplayer.ini", QSettings::IniFormat, parent))
{

}

SettingsManager::~SettingsManager()
{
}
