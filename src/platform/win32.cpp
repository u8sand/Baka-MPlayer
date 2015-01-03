#include "platform.h"

#include <QApplication>
#include <windows.h>

namespace Platform {

bool DimLightsSupported()
{
    return true;
}

void SetAlwaysOnTop(WId wid, bool ontop)
{
    SetWindowPos((HWND)wid,
                 ontop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
}

QSettings *InitializeSettings(QObject *parent)
{
    // saves to $(application directory)\${SETTINGS_FILE}.ini
    return new QSettings(QApplication::applicationDirPath()+"\\"+SETTINGS_FILE+".ini", QSettings::IniFormat, parent);
}

}
