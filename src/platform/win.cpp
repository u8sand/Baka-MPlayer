#include "util.h"

#include <QApplication>
#include <QRegExp>
#include <QProcess>
#include <QDir>

#include <windows.h>

#include "settings.h"


namespace Util {

QString VersionFileUrl()
{
    return "http://bakamplayer.u8sand.net/version_windows";
}

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

QString SettingsLocation()
{
    // saves to $(application directory)\${SETTINGS_FILE}.ini
    return QString("%0\\%1.ini").arg(QApplication::applicationDirPath(), SETTINGS_FILE);
}

bool IsValidFile(QString path)
{
    QRegExp rx("^(\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // relative path, network location, drive
    return (rx.indexIn(path) != -1);
}

bool IsValidLocation(QString loc)
{
    QRegExp rx("^([a-z]{2,}://|\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // url, relative path, network location, drive
    return (rx.indexIn(loc) != -1);
}

void ShowInFolder(QString path, QString file)
{
    QProcess::startDetached("explorer.exe", QStringList{"/select,", path+file});
}

QString MonospaceFont()
{
    return "Lucida Console";
}

}
