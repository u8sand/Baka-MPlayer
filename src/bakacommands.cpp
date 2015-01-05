#include "bakaengine.h"

#include "ui/aboutdialog.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "mpvhandler.h"

#include <QApplication>

void BakaEngine::BakaPlayPause(QStringList &args)
{
    if(args.empty())
    {
        if(mpv->getPlayState() > 0)
            mpv->PlayPause(window->ui->playlistWidget->CurrentItem());
    }
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaVolume(QStringList &args)
{
    if(!args.empty())
    {
        if(args.front().startsWith('+') || args.front().startsWith('-'))
            mpv->Volume(mpv->getVolume()+args.front().toInt(), true);
        else
            mpv->Volume(args.front().toInt(), true);
    }
    else
        RequiresParameters("volume");
}

void BakaEngine::BakaHelp(QStringList &)
{
    BakaPrint("This feature is coming soon...\n");
}

void BakaEngine::BakaAbout(QStringList &args)
{
    if(args.empty())
        AboutDialog::about(BAKA_MPLAYER_VERSION, window);
    else if(args.front() == "qt")
        qApp->aboutQt();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaQuit(QStringList &args)
{
    if(args.empty())
        qApp->quit();
    else
        InvalidParameter(args.join(' '));
}

