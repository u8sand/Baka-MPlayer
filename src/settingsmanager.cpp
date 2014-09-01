#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject *parent) :
#if Q_OS_WIN // saves to $(application directory)\${SETTINGS_FILE}.ini
    QSettings(QApplication::applicationDirPath()+"\\"+SETTINGS_FILE, QSettings::IniFormat,parent)
#else // saves to  ~/.config/${SETTINGS_FILE}.ini on linux
    QSettings(QSettings::IniFormat, QSettings::UserScope, SETTINGS_FILE, QString(), parent)
#endif
{
    // window
    value("window/width", 600);
    value("window/height", 430);
    value("window/onTop", "never");
    value("window/autoFit", 100);
    value("window/trayIcon", false);
    value("window/hidePopup", false);
    // mpv
    value("mpv/lastFile", "");
    value("mpv/showAll", false);
    value("mpv/screenshotFormat", "png");
    value("mpv/screenshotTemplate", "screenshot%#04n");
    value("mpv/speed", 1.0);
    value("mpv/volume", 100);
    // common
    value("common/debug", false);
}
