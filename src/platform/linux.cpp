#include "util.h"
#include "settings.h"

#include <QRegExp>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QGuiApplication>

#include <QX11Info>
#include <X11/Xlib.h>

namespace Util {

QString VersionFileUrl()
{
    return "http://bakamplayer.u8sand.net/version_linux";
}

QString DownloadFileUrl()
{
    return "";
}

bool DimLightsSupported()
{
    if(QGuiApplication::platformName().contains("wayland"))
        return false;
    QString tmp = "_NET_WM_CM_S"+QString::number(QX11Info::appScreen());
    Atom a = XInternAtom(QX11Info::display(), tmp.toUtf8().constData(), false);
    if(a && XGetSelectionOwner(QX11Info::display(), a)) // hack for QX11Info::isCompositingManagerRunning()
        return true;
    return false;
}

void SetAlwaysOnTop(WId wid, bool ontop)
{
    if(QGuiApplication::platformName().contains("wayland"))
        return;
    Display *display = QX11Info::display();
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = display;
    event.xclient.window  = wid;
    event.xclient.message_type = XInternAtom (display, "_NET_WM_STATE", False);
    event.xclient.format = 32;

    event.xclient.data.l[0] = ontop;
    event.xclient.data.l[1] = XInternAtom (display, "_NET_WM_STATE_ABOVE", False);
    event.xclient.data.l[2] = 0; //unused.
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    XSendEvent(display, DefaultRootWindow(display), False,
                           SubstructureRedirectMask|SubstructureNotifyMask, &event);
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
