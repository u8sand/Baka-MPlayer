#include "bakaengine.h"

#include "ui/aboutdialog.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "mpvhandler.h"

#include <QApplication>
#include <QFileDialog>
#include <QDesktopWidget>


void BakaEngine::BakaOpen(QStringList &args)
{
    if(args.empty())
        Open();
    else
        Open(args.join(' '));
}

void BakaEngine::Open(QString path)
{
    mpv->LoadFile(path == QString() ?
                   QFileDialog::getOpenFileName(window,
                   tr("Open File"),mpv->getPath(),
                   QString("%0 (%1);;").arg(tr("Media Files"), Mpv::media_filetypes.join(" "))+
                   QString("%0 (%1);;").arg(tr("Video Files"), Mpv::video_filetypes.join(" "))+
                   QString("%0 (%1)").arg(tr("Audio Files"), Mpv::audio_filetypes.join(" ")),
                   0, QFileDialog::DontUseSheet) :
                   path);
}


void BakaEngine::BakaPlayPause(QStringList &args)
{
    if(args.empty())
        PlayPause();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::PlayPause()
{
    if(mpv->getPlayState() > 0)
        mpv->PlayPause(window->ui->playlistWidget->CurrentItem());
}


void BakaEngine::BakaFitWindow(QStringList &args)
{
    if(args.empty())
        FitWindow();
    else
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty())
            FitWindow(arg.toInt());
        else
            InvalidParameter(args.join(' '));
    }
}

void BakaEngine::FitWindow(int percent, bool msg)
{
    if(window->isFullScreen() || window->isMaximized())
        return;

    mpv->LoadVideoParams();

    const Mpv::VideoParams &vG = mpv->getFileInfo().video_params; // video geometry
    QRect mG = window->ui->mpvFrame->geometry(),                  // mpv geometry
          wfG = window->frameGeometry(),                          // frame geometry of window (window geometry + window frame)
          wG = window->geometry(),                                // window geometry
          aG = qApp->desktop()->availableGeometry(wfG.center());  // available geometry of the screen we're in--(geometry not including the taskbar)

    double a, // aspect ratio
           w, // width of vid we want
           h; // height of vid we want

    // obtain natural video aspect ratio
    if(vG.width == 0 || vG.height == 0) // width/height are 0 when there is no output
        return;
    if(vG.dwidth == 0 || vG.dheight == 0) // dwidth/height are 0 on load
        a = double(vG.width)/vG.height; // use video width and height for aspect ratio
    else
        a = double(vG.dwidth)/vG.dheight; // use display width and height for aspect ratio

    // calculate resulting display:
    if(percent == 0) // fit to window
    {
        // set our current mpv frame dimensions
        w = mG.width();
        h = mG.height();

        double cmp = w/h - a, // comparison
               eps = 0.01;  // epsilon (deal with rounding errors) we consider -eps < 0 < eps ==> 0

        if(cmp > eps) // too wide
            w = h * a; // calculate width based on the correct height
        else if(cmp < -eps) // too long
            h = w / a; // calculate height based on the correct width
    }
    else // fit into desired dimensions
    {
        double scale = percent / 100.0; // get scale

        w = vG.width * scale;  // get scaled width
        h = vG.height * scale; // get scaled height
    }

    double dW = w + (wfG.width() - mG.width()),   // calculate display width of the window
           dH = h + (wfG.height() - mG.height()); // calculate display height of the window

    if(dW > aG.width()) // if the width is bigger than the available area
    {
        dW = aG.width(); // set the width equal to the available area
        w = dW - (wfG.width() - mG.width());    // calculate the width
        h = w / a;                              // calculate height
        dH = h + (wfG.height() - mG.height());  // calculate new display height
    }
    if(dH > aG.height()) // if the height is bigger than the available area
    {
        dH = aG.height(); // set the height equal to the available area
        h = dH - (wfG.height() - mG.height()); // calculate the height
        w = h * a;                             // calculate the width accordingly
        dW = w + (wfG.width() - mG.width());   // calculate new display width
    }

    // get the centered rectangle we want
    QRect rect = QStyle::alignedRect(Qt::LeftToRight,
                                     Qt::AlignCenter,
                                     QSize(dW,
                                           dH),
                                     percent == 0 ? wfG : aG); // center in window (autofit) or on our screen

    // adjust the rect to compensate for the frame
    rect.setLeft(rect.left() + (wG.left() - wfG.left()));
    rect.setTop(rect.top() + (wG.top() - wfG.top()));
    rect.setRight(rect.right() - (wfG.right() - wG.right()));
    rect.setBottom(rect.bottom() - (wfG.bottom() - wG.bottom()));

    // finally set the geometry of the window
    window->setGeometry(rect);

    // note: the above block is required because there is not setFrameGeometry function

    if(msg)
        mpv->ShowText(tr("Fit Window: %0%").arg(percent == 0 ? tr("autofit") : QString::number(percent)));
}


void BakaEngine::BakaVolume(QStringList &args)
{
    if(!args.empty())
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty())
        {
            if(arg.startsWith('+') || arg.startsWith('-'))
                mpv->Volume(mpv->getVolume()+arg.toInt(), true);
            else
                mpv->Volume(arg.toInt(), true);
        }
        else
            InvalidParameter(args.join(' '));
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
    About(args.join(' '));
}

void BakaEngine::About(QString what)
{
    if(what == QString())
        AboutDialog::about(BAKA_MPLAYER_VERSION, window);
    else if(what == "qt")
        qApp->aboutQt();
    else
        InvalidParameter(what);
}


void BakaEngine::BakaQuit(QStringList &args)
{
    if(args.empty())
        Quit();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::Quit()
{
    qApp->quit();
}
