#include "bakaengine.h"

#include "ui/aboutdialog.h"
#include "ui/mainwindow.h"

#include <QApplication>

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

