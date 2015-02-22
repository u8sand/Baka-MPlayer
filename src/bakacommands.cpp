#include "bakaengine.h"

#include <QApplication>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QProcess>
#include <QDir>
#include <QClipboard>
#include <QMessageBox>

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/aboutdialog.h"
#include "ui/locationdialog.h"
#include "ui/jumpdialog.h"
#include "ui/preferencesdialog.h"
#include "ui/updatedialog.h"
#include "ui/screenshotdialog.h"
#include "widgets/dimdialog.h"
#include "mpvhandler.h"
#include "overlayhandler.h"
#include "updatemanager.h"
#include "util.h"


void BakaEngine::BakaMpv(QStringList &args)
{
    if(mpv->playState > 0)
        Print(tr("mpv not yet initialized"));

    if(!args.empty())
        mpv->Command(args);
    else
        RequiresParameters("mpv");
}

void BakaEngine::BakaSh(QStringList &args)
{
    if(!args.empty())
    {
        QString arg = args.front();
        args.pop_front();
        QProcess *p = new QProcess(this);
        p->start(arg, args);
        connect(p, &QProcess::readyRead,
                [=]
                {
                    Print(p->readAll(), QString("%0(%1))").arg(p->program(), QString::number(quintptr(p))));
                });
//        connect(p, &QProcess::finished,
//                [=](int, QProcess::ExitStatus)
//                {
//                    delete p;
//                });
    }
    else
        RequiresParameters("mpv");
}

void BakaEngine::BakaNew(QStringList &args)
{
    if(args.empty())
        QProcess::startDetached(QApplication::applicationFilePath(), {});
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaOpenLocation(QStringList &args)
{
    if(args.empty())
        OpenLocation();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::OpenLocation()
{
    mpv->LoadFile(LocationDialog::getUrl(mpv->getPath()+mpv->getFile(), window));
}


void BakaEngine::BakaOpenClipboard(QStringList &args)
{
    if(args.empty())
        mpv->LoadFile(QApplication::clipboard()->text());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaShowInFolder(QStringList &args)
{
    if(args.empty())
        Util::ShowInFolder(mpv->getPath(), mpv->getFile());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaAddSubtitles(QStringList &args)
{
    QString trackFile;
    if(args.empty())
    {
        trackFile = QFileDialog::getOpenFileName(window, tr("Open Subtitle File"), mpv->getPath(),
                                                 QString("%0 (%1)").arg(tr("Subtitle Files"), Mpv::subtitle_filetypes.join(" ")),
                                                 0, QFileDialog::DontUseSheet);
    }
    else
        trackFile = args.join(' ');

    mpv->AddSubtitleTrack(trackFile);
}

void BakaEngine::BakaScreenshot(QStringList &args)
{
    if(args.empty())
        Screenshot(false);
    else
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty() && arg == "subtitles")
            Screenshot(true);
        else
            InvalidParameter(arg);
    }
}

void BakaEngine::Screenshot(bool subs)
{
    if(window->screenshotDialog)
    {
        mpv->Pause();
        if(ScreenshotDialog::showScreenshotDialog(window->screenshotDialog, subs, mpv) != QDialog::Accepted)
            return;
    }
    else
        mpv->Screenshot(subs);

    QString dir = mpv->getScreenshotDir();
    int i = dir.lastIndexOf('/');
    if(i != -1)
        dir.remove(0, i+1);
    if(subs)
        mpv->ShowText(tr("Saved to \"%0\", with subs").arg(dir));
    else
        mpv->ShowText(tr("Saved to \"%0\", without subs").arg(dir));
}


void BakaEngine::BakaMediaInfo(QStringList &args)
{
    if(args.empty())
        MediaInfo(window->ui->actionMedia_Info->isChecked());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::MediaInfo(bool show)
{
    overlay->showInfoText(show);
}


void BakaEngine::BakaStop(QStringList &args)
{
    if(args.empty())
        mpv->Stop();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaPlaylist(QStringList &args)
{
    if(!args.empty())
    {
        QString arg = args.front();
        args.pop_front();
        if(arg == "play")
        {
            if(args.empty())
                window->ui->playlistWidget->PlayIndex(window->ui->playlistWidget->currentRow());
            else
            {
                arg = args.front();
                args.pop_front();
                if(args.empty())
                {
                    if(arg.startsWith('+') || arg.startsWith('-'))
                        window->ui->playlistWidget->PlayIndex(arg.toInt(), true);
                    else
                        window->ui->playlistWidget->PlayIndex(arg.toInt());
                }
                else
                    InvalidParameter(args.join(' '));
            }
        }
        else if(arg == "select")
        {
            if(args.empty())
                window->ui->playlistWidget->SelectIndex(window->ui->playlistWidget->CurrentIndex());
            else
            {
                arg = args.front();
                args.pop_front();
                if(args.empty())
                {
                    if(arg.startsWith('+') || arg.startsWith('-'))
                        window->ui->playlistWidget->SelectIndex(arg.toInt(), true);
                    else
                        window->ui->playlistWidget->SelectIndex(arg.toInt());
                }
                else
                    InvalidParameter(args.join(' '));
            }
        }
        else if(args.empty())
        {
            if(arg == "remove")
            {
                if(window->isPlaylistVisible() && !window->ui->inputLineEdit->hasFocus() && !window->ui->searchBox->hasFocus())
                    window->ui->playlistWidget->RemoveIndex(window->ui->playlistWidget->currentRow());
            }
            else if(arg == "shuffle")
                window->ui->playlistWidget->Shuffle();
            else if(arg == "toggle")
                window->ShowPlaylist(!window->isPlaylistVisible());
            else if(arg == "full")
                window->HideAlbumArt(window->ui->action_Hide_Album_Art->isChecked());
            else
                InvalidParameter(arg);
        }
        else if(arg == "repeat")
        {
            arg = args.front();
            args.pop_front();
            if(args.empty())
            {
                if(arg == "off")
                {
                    if(window->ui->action_Off->isChecked())
                    {
                        window->ui->action_This_File->setChecked(false);
                        window->ui->action_Playlist->setChecked(false);
                    }
                }
                else if(arg == "this")
                {
                    if(window->ui->action_This_File->isChecked())
                    {
                        window->ui->action_Off->setChecked(false);
                        window->ui->action_Playlist->setChecked(false);
                    }
                }
                else if(arg == "playlist")
                {
                    if(window->ui->action_Playlist->isChecked())
                    {
                        window->ui->action_Off->setChecked(false);
                        window->ui->action_This_File->setChecked(false);
                    }
                }
                else
                    InvalidParameter(arg);
            }
            else
                InvalidParameter(args.join(' '));
        }
        else
            InvalidParameter(arg);
    }
    else
        RequiresParameters("baka playlist");
}

void BakaEngine::BakaJump(QStringList &args)
{
    if(args.empty())
        Jump();
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::Jump()
{
    int time = JumpDialog::getTime(mpv->getFileInfo().length, window);
    if(time >= 0)
        mpv->Seek(time);
}


void BakaEngine::BakaDim(QStringList &args)
{
    if(dimDialog == nullptr)
    {
        Print(tr("DimDialog not supported on this platform"));
        return;
    }
    if(args.empty())
        Dim(!dimDialog->isVisible());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::Dim(bool dim)
{
    if(dimDialog == nullptr)
    {
        QMessageBox::information(window, tr("Dim Lights"), tr("In order to dim the lights, the desktop compositor has to be enabled. This can be done through Window Manager Desktop."));
        return;
    }
    if(dim)
        dimDialog->show();
    else
        dimDialog->close();
}

void BakaEngine::BakaOutput(QStringList &args)
{
    if(args.empty())
        window->setDebug(!window->getDebug());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaPreferences(QStringList &args)
{
    if(args.empty())
        PreferencesDialog::showPreferences(this, window);
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaOnlineHelp(QStringList &args)
{
    if(args.empty())
        QDesktopServices::openUrl(QUrl(tr("http://bakamplayer.u8sand.net/help.php")));
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaUpdate(QStringList &args)
{
    if(args.empty())
        UpdateDialog::CheckForUpdates(this, window);
    else
    {
#if defined(Q_OS_WIN)
        QString arg = args.front();
        args.pop_front();
        if(arg == "youtube-dl")
            QProcess::startDetached("youtube-dl.exe", {"--update"});
        else
#endif
            InvalidParameter(args.join(' '));
    }
}

void BakaEngine::BakaOpen(QStringList &args)
{
    if(args.empty())
        Open();
    else
        mpv->LoadFile(args.join(' '));
}

void BakaEngine::Open()
{
    mpv->LoadFile(QFileDialog::getOpenFileName(window,
                   tr("Open File"),mpv->getPath(),
                   QString("%0 (%1);;").arg(tr("Media Files"), Mpv::media_filetypes.join(" "))+
                   QString("%0 (%1);;").arg(tr("Video Files"), Mpv::video_filetypes.join(" "))+
                   QString("%0 (%1)").arg(tr("Audio Files"), Mpv::audio_filetypes.join(" ")),
                   0, QFileDialog::DontUseSheet));
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
        mpv->ShowText(tr("Fit Window: %0").arg(percent == 0 ? tr("autofit") : (QString::number(percent)+"%")));
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

void BakaEngine::BakaSpeed(QStringList &args)
{
    if(!args.empty())
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty())
        {
            if(arg.startsWith('+') || arg.startsWith('-'))
                mpv->Speed(mpv->getSpeed()+arg.toDouble());
            else
                mpv->Speed(arg.toDouble());
            mpv->ShowText(tr("Speed: x%0").arg(QString::number(mpv->getSpeed(), 'f', 2)));
        }
        else
            InvalidParameter(args.join(' '));
    }
    else
        RequiresParameters("speed");
}

void BakaEngine::BakaFullScreen(QStringList &args)
{
    if(args.empty())
        window->FullScreen(!window->isFullScreen());
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaBoss(QStringList &args)
{
    if(args.empty())
    {
        if(window->isFullScreen()) // exit fullscreen if in fullscreen
            window->FullScreen(false);
        else
        {
            mpv->Pause();
            window->setWindowState(window->windowState() | Qt::WindowMinimized); // minimze window
        }
    }
    else
        InvalidParameter(args.join(' '));
}

void BakaEngine::BakaHelp(QStringList &args)
{
    if(args.empty())
    {
        Print(tr("usage: baka <command> [...]"));
        Print(tr("commands:"));
        int len, max_len = 22;
        for(auto command = BakaCommandMap.begin(); command != BakaCommandMap.end(); ++command)
        {
            QString str = QString("  %0 %1").arg(command.key(), command->second[0]);
            len = str.length();
            while(len++ <= max_len)
                str += ' ';
            str += command->second[1];
            Print(str);
        }
    }
    else
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty())
        {
            auto command = BakaCommandMap.find(arg);
            if(command != BakaCommandMap.end()) //found
            {
                Print(tr("usage: %0 %1").arg(arg, command->second[0]));
                Print(tr("description:"));
                Print(QString("  %0").arg(command->second[1]));
                if(command->second.length() > 2 && command->second[2] != QString())
                {
                    Print(tr("advanced:"));
                    Print(QString("  %0").arg(command->second[2]));
                }
            }
            else
                InvalidParameter(arg);
        }
        else
            InvalidParameter(args.join(' '));
    }
}

void BakaEngine::BakaAbout(QStringList &args)
{
    About(args.join(' '));
}

void BakaEngine::BakaDebug(QStringList &args)
{
    if(!args.empty())
    {
        QString arg = args.front();
        args.pop_front();
        if(args.empty())
            mpv->Debug(arg);
        else
            InvalidParameter(args.join(' '));
    }
    else
        RequiresParameters("debug");
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
