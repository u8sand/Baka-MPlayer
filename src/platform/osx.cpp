#include "util.h"
#include "settings.h"

#include <QRegExp>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
// not needed as SetAlwaysOnTop was stubbed for now
//#include <QWindow>

namespace Util {

QString VersionFileUrl()
{
    return "http://bakamplayer.u8sand.net/version_osx";
}

QString DownloadFileUrl()
{
    return "";
}

bool DimLightsSupported()
{
    // stubbed
    return true;
}

void SetAlwaysOnTop(WId wid, bool ontop)
{
  if (ontop){
    // doesn't work
    /*
    QWindow *window = QWindow::fromWinId(wid);
    window->setFlags(
          Qt::WindowStaysOnTopHint
      );
    */
  }
}

QString SettingsLocation()
{
    // saves to  ~/.config/${SETTINGS_FILE}.ini
    QString s1  = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString s2 = SETTINGS_FILE;
    return QString("%0/%1.ini").arg(
            QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
            SETTINGS_FILE);
}

bool IsValidFile(QString path)
{
    QRegExp rx("^\\.{1,2}|/", Qt::CaseInsensitive); // relative path, network location, drive
    return (rx.indexIn(path) != -1);
}

bool IsValidLocation(QString loc)
{
    QRegExp rx("^([a-z]{2,}://|\\.{1,2}|/)", Qt::CaseInsensitive); // url, relative path, drive
    return (rx.indexIn(loc) != -1);
}

void ShowInFolder(QString path, QString)
{
    QDesktopServices::openUrl(QString("file:///%0").arg(path));
}

QString MonospaceFont()
{
    return "Monospace";
}

}
