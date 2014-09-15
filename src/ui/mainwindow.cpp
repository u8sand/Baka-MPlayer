#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QMimeData>
#include <QTime>
#include <QFileDialog>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QSignalMapper>
#include <QAction>
#include <QShortcut>
#include <QProcess>
#include <QIcon>
#include <QWindow>

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
#include <QX11Info>
#include <X11/Xlib.h>
#else
#include <windows.h>
#endif

#include "aboutdialog.h"
#include "infodialog.h"
#include "locationdialog.h"
#include "jumpdialog.h"
#include "inputdialog.h"
#include "updatedialog.h"
#include "preferencesdialog.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastMousePos(QPoint()),
    move(false),
    init(false),
    autohide(new QTimer(this))
{
    QAction *action;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) // if on x11, dim desktop requires a compositing manager, make dimDialog NULL if there is none
    Atom a = XInternAtom(QX11Info::display(), "_NET_WM_CM_S0", false);
    if(a && XGetSelectionOwner(QX11Info::display(), a)) // hack for QX11Info::isCompositingManagerRunning()
        dimDialog = new DimDialog(); // dimdialog must be initialized before ui is setup
    else
        dimDialog = 0;
#else
    dimDialog = new DimDialog(); // dimDialog must be initialized before ui is setup
#endif
    ui->setupUi(this);
    ShowPlaylist(false);
    addActions(ui->menubar->actions()); // makes menubar shortcuts work even when menubar is hidden


    // initialize managers/handlers
#if defined(Q_OS_WIN) // saves to $(application directory)\${SETTINGS_FILE}.ini
    settings = new QSettings(QApplication::applicationDirPath()+"\\"+SETTINGS_FILE, QSettings::IniFormat,this);
#else // saves to  ~/.config/${SETTINGS_FILE}.ini on linux
    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, SETTINGS_FILE, QString(), this);
#endif
    mpv = new MpvHandler(ui->mpvFrame->winId(), this);
    update = new UpdateManager(this);

    // initialize other ui elements
    // note: trayIcon does not work in my environment--known qt bug
    // see: https://bugreports.qt-project.org/browse/QTBUG-34364
    // todo: tray menu/tooltip
    sysTrayIcon = new QSystemTrayIcon(qApp->windowIcon(), this);
    ui->mpvFrame->installEventFilter(this); // capture events on mpvFrame in the eventFilter function

    // setup signals & slots

    // mainwindow
    connect(this, &MainWindow::onTopChanged,
            [=](QString onTop)
            {
                if(onTop == "never")
                    AlwaysOnTop(false);
                else if(onTop == "always")
                    AlwaysOnTop(true);
                else if(onTop == "playing" && mpv->getPlayState() > 0)
                    AlwaysOnTop(true);
            });

    connect(sysTrayIcon, &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason)
            {
                if(reason == QSystemTrayIcon::Trigger)
                {
                    if(!hidePopup)
                    {
                        if(mpv->getPlayState() == Mpv::Playing)
                            sysTrayIcon->showMessage("Baka MPlayer", "Playing", QSystemTrayIcon::NoIcon, 4000);
                        else if(mpv->getPlayState() == Mpv::Paused)
                            sysTrayIcon->showMessage("Baka MPlayer", "Paused", QSystemTrayIcon::NoIcon, 4000);
                    }
                    mpv->PlayPause(ui->playlistWidget->currentRow());
                }

            });

//    connect(this, &MainWindow::hidePopupChanged,
//            [=](bool b)
//    {
//    });

    connect(this, &MainWindow::debugChanged,
            [=](bool b)
            {
                mpv->Debug(b);
                ui->actionShow_D_ebug_Output->setChecked(b);
                ui->outputTextEdit->setVisible(b);
            });

    connect(autohide, &QTimer::timeout, // cursor autohide
            [=]
            {
                setCursor(QCursor(Qt::BlankCursor));
                autohide->stop();
            });

    // mpv


    connect(mpv, &MpvHandler::playlistChanged,
            [=](const QStringList &list)
            {
                ui->playlistWidget->clear();
                ui->playlistWidget->addItems(list);

                if(list.length() > 1)
                {
                    ui->actionSh_uffle->setEnabled(true);
                    ui->actionStop_after_Current->setEnabled(true);
                }
                else
                {
                    ui->actionSh_uffle->setEnabled(false);
                    ui->actionStop_after_Current->setEnabled(false);
                }

                if(list.length() > 0)
                    ui->menuR_epeat->setEnabled(true);
                else
                    ui->menuR_epeat->setEnabled(false);
                ui->playlistWidget->setCurrentRow(mpv->getIndex());
            });

    connect(mpv, &MpvHandler::fileInfoChanged,
            [=](const Mpv::FileInfo &fileInfo)
            {
                if(mpv->getPlayState() > 0)
                {
                    setWindowTitle(fileInfo.media_title);
                    ui->seekBar->setTracking(fileInfo.length);

                    if(!remaining)
                        ui->remainingLabel->setText(FormatTime(fileInfo.length));
                }
            });

    connect(mpv, &MpvHandler::trackListChanged,
            [=](const QList<Mpv::Track> &trackList)
    {
        if(mpv->getPlayState() > 0)
        {
            QAction *action;
            bool video = false,
                 albumArt = false;

            ui->menuSubtitle_Track->clear();
            ui->menuSubtitle_Track->addAction(ui->action_Add_Subtitle_File);
            for(auto &track : trackList)
            {
                if(track.type == "sub")
                {
                    action = ui->menuSubtitle_Track->addAction(QString::number(track.id)+": "+track.title+" ("+track.lang+")");
                    connect(action, &QAction::triggered,
                            [=]
                            {
                                // basically, if you uncheck the selected subtitle id, we hide subtitles
                                // when you check a subtitle id, we make sure subtitles are showing and set it
                                if(mpv->getSid() == track.id)
                                {
                                    if(mpv->getSubtitleVisibility())
                                    {
                                        mpv->ShowSubtitles(false);
                                        return;
                                    }
                                    else
                                        mpv->ShowSubtitles(true);
                                }
                                else if(!mpv->getSubtitleVisibility())
                                    mpv->ShowSubtitles(true);
                                mpv->Sid(track.id);
                            });
                }
                else if(track.type == "video") // video track
                {
                    if(!track.albumart) // isn't album art
                        video = true;
                    else
                        albumArt = true;
                }
            }
            if(video)
            {
                // if we were hiding album art, show it--we've gone to a video
                if(ui->action_Hide_Album_Art_2->isChecked())
                {
                    HideAlbumArt(false);
                    ui->action_Show_Playlist_2->setEnabled(true);
                    ui->splitter->setEnabled(true);
                }
                ui->action_Hide_Album_Art_2->setEnabled(false);
                ui->menuSubtitle_Track->setEnabled(true);
                if(ui->menuSubtitle_Track->actions().count() > 1)
                {
                    ui->menuFont_Si_ze->setEnabled(true);
                    ui->actionShow_Subtitles->setEnabled(true);
                    ui->actionShow_Subtitles->setChecked(mpv->getSubtitleVisibility());
                }
                else
                {
                    ui->menuFont_Si_ze->setEnabled(false);
                    ui->actionShow_Subtitles->setEnabled(false);
                }
                ui->menuTake_Screenshot->setEnabled(true);
                ui->menuFit_Window->setEnabled(true);
                ui->menuAspect_Ratio->setEnabled(true);
                ui->action_Frame_Step->setEnabled(true);
                ui->actionFrame_Back_Step->setEnabled(true);
            }
            else
            {
                // if there is no album art we force hide album art
                if(!albumArt)
                {
                    HideAlbumArt(true);
                    ui->action_Show_Playlist_2->setEnabled(false);
                    ui->splitter->setEnabled(false);
                }
                else
                {
                    ui->action_Show_Playlist_2->setEnabled(true);
                    ui->action_Hide_Album_Art_2->setEnabled(true);
                    ui->splitter->setEnabled(true);
                }
                ui->menuSubtitle_Track->setEnabled(false);
                ui->menuFont_Si_ze->setEnabled(false);
                ui->actionShow_Subtitles->setEnabled(false);
                ui->menuTake_Screenshot->setEnabled(false);
                ui->menuFit_Window->setEnabled(ui->action_Hide_Album_Art_2->isEnabled());
                ui->menuAspect_Ratio->setEnabled(false);
                ui->action_Frame_Step->setEnabled(false);
                ui->actionFrame_Back_Step->setEnabled(false);


                if(sysTrayIcon->isVisible() && !hidePopup)
                {
                    // todo: use {artist} - {title}
                    sysTrayIcon->showMessage("Baka MPlayer", mpv->getFileInfo().media_title, QSystemTrayIcon::NoIcon, 4000);
                }
            }
        }
    });

    connect(mpv, &MpvHandler::chaptersChanged,
            [=](const QList<Mpv::Chapter> &chapters)
    {
        if(mpv->getPlayState() > 0)
        {
            QList<int> ticks;
            QSignalMapper *signalMapper = new QSignalMapper(this);
            int n = 1;
            ui->menu_Chapters->clear();
            for(auto &ch : chapters)
            {
                QAction *action;
                if(n <= 9)
                    action = ui->menu_Chapters->addAction(QString::number(n)+": "+ch.title, NULL, NULL, QKeySequence("Ctrl+"+QString::number(n)));
                else
                    action = ui->menu_Chapters->addAction(QString::number(n)+": "+ch.title);
                n++;
                signalMapper->setMapping(action, ch.time);
                connect(action, SIGNAL(triggered()),
                        signalMapper, SLOT(map()));
                ticks.push_back(ch.time);
            }
            connect(signalMapper, SIGNAL(mapped(int)),
                    mpv, SLOT(Seek(int)));

            if(ui->menu_Chapters->actions().count() == 0)
            {
                ui->menu_Chapters->setEnabled(false);
                ui->action_Next_Chapter->setEnabled(false);
                ui->action_Previous_Chapter->setEnabled(false);
            }
            else
            {
                ui->menu_Chapters->setEnabled(true);
                ui->action_Next_Chapter->setEnabled(true);
                ui->action_Previous_Chapter->setEnabled(true);
            }

            ui->seekBar->setTicks(ticks);
        }
    });

    connect(mpv, &MpvHandler::playStateChanged,
            [=](Mpv::PlayState playState)
            {
                switch(playState)
                {
                case Mpv::Loaded: // todo: show the user we are loading their file?
                    break;

                case Mpv::Started:
                    if(!init) // will only happen the first time a file is loaded.
                    {
                        ui->action_Play->setEnabled(true);
                        ui->playButton->setEnabled(true);
                        ui->playlistButton->setEnabled(true);
                        ui->action_Show_Playlist_2->setEnabled(true);
                        ui->splitter->setEnabled(true);
                        init = true;
                    }
                    if(pathChanged && autoFit)
                    {
                        FitWindow(autoFit);
                        pathChanged = false;
                    }
                    SetPlaybackControls(true);
                    mpv->Play();
                case Mpv::Playing:
                    ui->playButton->setIcon(QIcon(":/img/default_pause.svg"));
                    ui->action_Play->setText("&Pause");
                    if(onTop == "playing")
                        AlwaysOnTop(true);
                    break;

                case Mpv::Paused:
                case Mpv::Stopped:
                    ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
                    ui->action_Play->setText("&Play");
                    if(ui->actionWhen_Playing->isChecked())
                        AlwaysOnTop(false);
                    break;

                case Mpv::Idle:
                    if(init)
                    {
                        if(ui->action_This_File->isChecked())
                            mpv->PlayIndex(mpv->getIndex()); // restart file
                        else if(mpv->getIndex() >= mpv->getMaxIndex() ||
                           ui->actionStop_after_Current->isChecked())
                        {
                            if(ui->action_Playlist->isChecked())
                                mpv->PlayIndex(0); // restart playlist
                            else
                            {
                                setWindowTitle("Baka MPlayer");
                                SetPlaybackControls(false);
                                ui->seekBar->setTracking(0);
                                ui->actionStop_after_Current->setChecked(false);
                            }
                        }
                        else
                            mpv->NextFile();
                    }
                    break;
                }
            });

    connect(mpv, &MpvHandler::pathChanged,
            [=]()
            {
                pathChanged = true;
            });

    connect(mpv, &MpvHandler::lastFileChanged,
            [=](QString f)
            {
                ui->actionOpen_Last_File->setEnabled(f != "");
            });

    connect(mpv, &MpvHandler::searchChanged,
            [=](QString s)
            {
                ui->searchBox->setText(s);
            });

    connect(mpv, &MpvHandler::timeChanged,
            [=](int i)
            {
                const Mpv::FileInfo &fi = mpv->getFileInfo();
                // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
                // the formula is a simple ratio seekBar's max * time/totalTime
                ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)i/fi.length));

                // set duration and remaining labels, QDateTime takes care of formatting for us
                ui->durationLabel->setText(FormatTime(i));
                if(remaining)
                    ui->remainingLabel->setText("-"+FormatTime(fi.length-i));

                // set next/previous chapter's enabled state
                if(fi.chapters.length() > 0)
                {
                    ui->action_Next_Chapter->setEnabled(i < fi.chapters.last().time);
                    ui->action_Previous_Chapter->setEnabled(i > fi.chapters.first().time);
                }
            });

    connect(mpv, &MpvHandler::volumeChanged,
            [=](int volume)
            {
                ui->volumeSlider->setValueNoSignal(volume);
            });

    connect(mpv, &MpvHandler::indexChanged,
            [=](int index)
            {
                ui->playlistWidget->setCurrentRow(index);
            });

    connect(mpv, &MpvHandler::sidChanged,
            [=](int sid)
            {
                QList<QAction*> actions = ui->menuSubtitle_Track->actions();
                for(auto &action : actions)
                {
                    if(action->text().startsWith(QString::number(sid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                        action->setChecked(false);
                }
            });

    connect(mpv, &MpvHandler::showAllChanged,
            [=](bool b)
            {
                ui->showAllButton->setChecked(b);
            });

    connect(mpv, &MpvHandler::shuffleChanged,
            [=](bool b)
            {
                ui->actionSh_uffle->setChecked(b);
            });

    connect(mpv, &MpvHandler::playlistVisibleChanged,
            [=](bool b)
            {
                ShowPlaylist(b);
            });

    connect(mpv, &MpvHandler::subtitleVisibilityChanged,
            [=](bool b)
            {
                ui->actionShow_Subtitles->setChecked(b);
            });

    connect(mpv, &MpvHandler::errorSignal,
            [=](QString err)
            {
                QMessageBox::warning(this, "Mpv Error", err);
            });

    connect(mpv, &MpvHandler::debugSignal,
            [=](QString msg)
            {
                ui->outputTextEdit->appendPlainText(msg);
            });

    // update manager

    /* automatic updating support
    connect(update, &UpdateManager::Update,
            [=](QMap<QString, QString> info)
            {
                if(info["version"] != BAKA_MPLAYER_VERSION)
                    update->DownloadUpdate();
            });

    connect(update, &UpdateManager::Downloaded,
            [=](int percent)
            {
                if(percent == 100)
                {
                    // prepare for update
                }
                // show progress as status message?
            });
    */

    // ui

    connect(ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                mpv->Seek(((double)i/ui->seekBar->maximum())*mpv->getFileInfo().length);
            });

    connect(ui->openButton, &OpenButton::LeftClick,                     // Playback: Open button (left click)
            [=]
            {
                mpv->LoadFile(QFileDialog::getOpenFileName(this,
                               "Open File",mpv->getPath(),
                               "Media Files ("+Mpv::media_filetypes.join(" ")+");;"+
                               "Video Files ("+Mpv::video_filetypes.join(" ")+");;"+
                               "Audio Files ("+Mpv::audio_filetypes.join(" ")+")"));
            });

    connect(ui->openButton, &OpenButton::MiddleClick,                   // Playback: Open button (middle click)
            [=]
            {
                int time = JumpDialog::getTime(mpv->getFileInfo().length,this);
                if(time >= 0)
                    mpv->Seek(time);
            });

    connect(ui->openButton, &OpenButton::RightClick,                    // Playback: Open button (right click)
            [=]
            {
                mpv->LoadFile(LocationDialog::getUrl(mpv->getFile(), this));
            });

    connect(ui->remainingLabel, &CustomLabel::clicked,                  // Playback: Remaining Label
            [=]
            {
                if(remaining)
                {
                    setRemaining(false);
                    ui->remainingLabel->setText(FormatTime(mpv->getFileInfo().length));
                }
                else
                    setRemaining(true);
            });

    connect(ui->rewindButton, &QPushButton::clicked,                    // Playback: Rewind button
            [=]
            {
                mpv->Rewind();
            });

    connect(ui->previousButton, &IndexButton::clicked,                  // Playback: Previous button
            [=]
            {
                mpv->PreviousFile();
            });

    connect(ui->playButton, &QPushButton::clicked,                      // Playback: Play/pause button
            [=]
            {
                mpv->PlayPause(ui->playlistWidget->currentRow());
            });

    connect(ui->nextButton, &IndexButton::clicked,                      // Playback: Next button
            [=]
            {
                mpv->NextFile();
            });

    connect(ui->volumeSlider, &CustomSlider::valueChanged,              // Playback: Volume slider adjusted
            [=](int i)
            {
                mpv->Volume(i);
            });

    connect(ui->playlistButton, &QPushButton::clicked,                  // Playback: Clicked the playlist button
            [=]
            {
                // if the position is 0, playlist is hidden so show it
                ShowPlaylist(!ui->splitter->position());
            });

    connect(ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                blockSignals(true);
                if(i == 0) // right-most, playlist is hidden
                {
                    ui->action_Show_Playlist_2->setChecked(false);
                    ui->action_Hide_Album_Art_2->setChecked(false);
                }
                else if(i == ui->splitter->max()) // left-most, album art is hidden, playlist is visible
                {
                    ui->action_Show_Playlist_2->setChecked(true);
                    ui->action_Hide_Album_Art_2->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    ui->action_Show_Playlist_2->setChecked(true);
                    ui->action_Hide_Album_Art_2->setChecked(false);
                }
                blockSignals(false);
            });

    connect(ui->searchBox, &QLineEdit::textChanged,                     // Playlist: Search box
            [=](QString s)
            {
                mpv->SearchPlaylist(s);
            });

    connect(ui->indexLabel, &CustomLabel::clicked,                      // Playlist: Clicked the indexLabel
            [=]
            {
                QString res = InputDialog::getInput("Enter the file number you want to play:\nNote: Value must be from 1 - "+QString::number(mpv->getMaxIndex()+1),
                                                    "Enter File Number",
                                                    [this](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if(in >= 1 && in <= mpv->getMaxIndex()+1)
                                                            return true;
                                                        return false;
                                                    },
                                                    this);
                if(res != "")
                    mpv->PlayIndex(res.toInt()-1); // user index will be 1 greater than actual
            });

    connect(ui->playlistWidget, &CustomListWidget::currentRowChanged,   // Playlist: Playlist selection changed
            [=](int i)
            {
                if(i == -1) // no selection
                {
                    ui->indexLabel->setText("No files in playlist");
                    ui->indexLabel->setEnabled(false);
                }
                else
                {
                    ui->indexLabel->setEnabled(true);
                    ui->indexLabel->setText("File "+QString::number(i+1)+" of "+QString::number(ui->playlistWidget->count()));
                }
            });

    connect(ui->playlistWidget, &CustomListWidget::doubleClicked,       // Playlist: Item double clicked
            [=](const QModelIndex &i)
            {
                mpv->PlayIndex(i.row());
            });

    connect(ui->currentFileButton, &QPushButton::clicked,               // Playlist: Select current file button
            [=]
            {
                ui->playlistWidget->setCurrentRow(mpv->getIndex());
            });

    connect(ui->showAllButton, &QPushButton::clicked,                   // Playlist: Show All button
            [=](bool b)
            {
                mpv->ShowAllPlaylist(b);
            });

    connect(ui->refreshButton, &QPushButton::clicked,                   // Playlist: Refresh playlist button
            [=]
            {
                mpv->RefreshPlaylist();
            });

    action = ui->playlistWidget->addAction("R&emove from Playlist");
    connect(action, &QAction::triggered,                                // Playlist: Remove from playlist (right-click)
            [=]
            {
                ui->playlistWidget->takeItem(ui->playlistWidget->currentRow());
            });

    action = ui->playlistWidget->addAction("&Delete from Disk");
    connect(action, &QAction::triggered,                                // Playlist: Delete from Disk (right-click)
            [=]
            {
                QListWidgetItem *item = ui->playlistWidget->takeItem(ui->playlistWidget->currentRow());
                QFile f(mpv->getPath()+item->text());
                f.remove();
            });

    action = ui->playlistWidget->addAction("&Refresh");                 // Playlist: Refresh (right-click)
    connect(action, &QAction::triggered,
            [=]
            {
                mpv->RefreshPlaylist();
            });

    connect(ui->playlistWidget, &CustomListWidget::reordered,           // Playlist: Re-arrange
            [=](int old_index, int new_index)
            {
                mpv->ReorderPlaylist(old_index, new_index);
            });

                                                                        // File ->
    connect(ui->action_New_Player, &QAction::triggered,                 // File -> New Player
            [=]
            {
                QProcess *p = new QProcess(0);
                p->startDetached(QApplication::applicationFilePath());
            });

    connect(ui->action_Open_File, &QAction::triggered,                  // File -> Open File
            [=]
            {
                mpv->LoadFile(QFileDialog::getOpenFileName(this,
                              "Open File",mpv->getPath(),
                              "Media Files ("+Mpv::media_filetypes.join(" ")+");;"+
                              "Video Files ("+Mpv::video_filetypes.join(" ")+");;"+
                              "Audio Files ("+Mpv::audio_filetypes.join(" ")+")"));
            });

    connect(ui->actionOpen_URL, &QAction::triggered,                    // File -> Open URL
            [=]
            {
                mpv->LoadFile(LocationDialog::getUrl(mpv->getFile(), this));
            });

    connect(ui->actionOpen_Path_from_Clipboard, &QAction::triggered,    // File -> Open Path from Clipboard
            [=]
            {
                mpv->LoadFile(QApplication::clipboard()->text());
            });

    connect(ui->actionOpen_Last_File, &QAction::triggered,              // File -> Open Last File
            [=]
            {
                mpv->LoadFile(mpv->getLastFile());
            });

    connect(ui->actionShow_in_Folder, &QAction::triggered,              // File -> Show in Folder
            [=]
            {
                QDesktopServices::openUrl("file:///"+QDir::toNativeSeparators(mpv->getPath()));
            });

    connect(ui->actionPlay_Next_File, &QAction::triggered,              // File -> Play Next File
            [=]
            {
                mpv->NextFile();
            });

    connect(ui->actionPlay_Previous_File, &QAction::triggered,          // File -> Play Previous File
            [=]
            {
                mpv->PreviousFile();
            });

    connect(ui->actionE_xit_2, &QAction::triggered,                     // File -> Exit
            [=]
            {
                close();
            });
                                                                        // View ->
    connect(ui->action_Full_Screen, &QAction::triggered,                // View -> Full Screen
            [=]
            {
                FullScreen(true);
            });

    connect(ui->actionWith_Subtitles, &QAction::triggered,              // View -> Take Screenshot -> With Subtitles
            [=]
            {
                if(mpv->getScreenshotDir() == "" && !SetScreenshotDir())
                    return;
                mpv->Screenshot(true);
            });

    connect(ui->actionWithout_Subtitles, &QAction::triggered,           // View -> Take Screenshot -> Without Subtitles
            [=]
            {
                if(mpv->getScreenshotDir() == "" && !SetScreenshotDir())
                    return;
                mpv->Screenshot(false);
            });
                                                                        // View -> Fit Window ->
    connect(ui->action_To_Current_Size, &QAction::triggered,            // View -> Fit Window -> To Current Size
            [=]
            {
                FitWindow(0);
            });

    connect(ui->action50, &QAction::triggered,                          // View -> Fit Window -> 50%
            [=]
            {
                FitWindow(50);
            });

    connect(ui->action75, &QAction::triggered,                          // View -> Fit Window -> 75%
            [=]
            {
                FitWindow(75);
            });

    connect(ui->action100, &QAction::triggered,                         // View -> Fit Window -> 100%
            [=]
            {
                FitWindow(100);
            });

    connect(ui->action200, &QAction::triggered,                         // View -> Fit Window -> 200%
            [=]
            {
                FitWindow(200);
            });
                                                                        // View -> Aspect Ratio ->
    connect(ui->action_Autodetect, &QAction::triggered,                 // View -> Aspect Ratio -> Auto Detect
            [=]
            {
                SetAspectRatio("-1");
            });

    connect(ui->actionForce_4_3, &QAction::triggered,                   // View -> Aspect Ratio -> 4:3
            [=]
            {
                SetAspectRatio("4:3");
            });

    connect(ui->actionForce_2_35_1, &QAction::triggered,                // View -> Aspect Ratio -> 2.35:1
            [=]
            {
                SetAspectRatio("2_35:1");
            });

    connect(ui->actionForce_16_9, &QAction::triggered,                  // View -> Aspect Ratio -> 16:9
            [=]
            {
                SetAspectRatio("16:9");
            });

    connect(ui->actionShow_Subtitles, &QAction::triggered,              // View -> Show Subtitles
            [=](bool b)
            {
                mpv->ShowSubtitles(b);
            });

    connect(ui->action_Add_Subtitle_File, &QAction::triggered,          //  View -> Subtitle Track -> Add Subtitle File...
            [=]
            {
                QString trackFile = QFileDialog::getOpenFileName(this, "Open Subtitle File", mpv->getPath(),
                                                                  "Subtitle Files ("+Mpv::subtitle_filetypes.join(" ")+")");
                if(trackFile != "")
                    mpv->AddSubtitleTrack(trackFile);
            });
                                                                        // View -> Font Size ->
    connect(ui->action_Size, &QAction::triggered,                       // View -> Font Size -> Size +
            [=]
            {
                mpv->SubtitleScale(.02, true);
            });

    connect(ui->actionS_ize, &QAction::triggered,                       // View -> Font Size -> Size -
            [=]
            {
                mpv->SubtitleScale(-.02, true);
            });

    connect(ui->action_Reset_Size, &QAction::triggered,                 // View -> Font Size -> Reset Size
            [=]
            {
                mpv->SubtitleScale(1);
            });

    connect(ui->actionMedia_Info, &QAction::triggered,                  // View -> Media Info
            [=]
            {
                InfoDialog::info(mpv->getFileInfo(), this);
            });
                                                                        // Playback ->
    connect(ui->action_Play, &QAction::triggered,                       // Playback -> (Play|Pause)
            [=]
            {
                mpv->PlayPause(ui->playlistWidget->currentRow());
            });

    connect(ui->action_Stop, &QAction::triggered,                       // Playback -> Stop
            [=]
            {
                mpv->Stop();
            });

    connect(ui->action_Restart, &QAction::triggered,                    // Playback -> Restart
            [=]
            {
                mpv->Restart();
            });

    connect(ui->actionSh_uffle, &QAction::triggered,                    // Playback -> Shuffle
            [=](bool b)
            {
                mpv->ShufflePlaylist(b);
            });
                                                                        // Playback -> Repeat
    connect(ui->action_Off, &QAction::triggered,                        // Playback -> Repeat -> Off
            [=](bool b)
            {
                if(b)
                {
                    ui->action_This_File->setChecked(false);
                    ui->action_Playlist->setChecked(false);
                }
            });

    connect(ui->action_This_File, &QAction::triggered,                  // Playback -> Repeat -> This File
            [=](bool b)
            {
                if(b)
                {
                    ui->action_Off->setChecked(false);
                    ui->action_Playlist->setChecked(false);
                }
            });

    connect(ui->action_Playlist, &QAction::triggered,                   // Playback -> Repeat -> Playlist
            [=](bool b)
            {
                if(b)
                {
                    ui->action_Off->setChecked(false);
                    ui->action_This_File->setChecked(false);
                }
            });

    connect(ui->action_Increase_Volume, &QAction::triggered,            // Playback -> Increase Volume
            [=]
            {
                mpv->Volume(mpv->getVolume()+5);
            });

    connect(ui->action_Decrease_Volume, &QAction::triggered,            // Playback -> Decrease Volume
            [=]
            {
                mpv->Volume(mpv->getVolume()-5);
            });
                                                                        // Navigate ->
    connect(ui->action_Next_Chapter, &QAction::triggered,               // Navigate -> Next Chapter
            [=]
            {
                mpv->NextChapter();
            });

    connect(ui->action_Previous_Chapter, &QAction::triggered,           // Navigate -> Previous Chapter
            [=]
            {
                mpv->PreviousChapter();
            });

    connect(ui->action_Frame_Step, &QAction::triggered,                 // Navigate -> Frame Step
            [=]
            {
                mpv->FrameStep();
            });

    connect(ui->actionFrame_Back_Step, &QAction::triggered,             // Navigate -> Frame Back Step
            [=]
            {
                mpv->FrameBackStep();
            });

    connect(ui->action_Jump_to_Time, &QAction::triggered,               // Navigate -> Jump to Time
            [=]
            {
                int time = JumpDialog::getTime(mpv->getFileInfo().length,this);
                if(time >= 0)
                    mpv->Seek(time);
            });
                                                                        // Settings ->
    connect(ui->action_Show_Playlist_2, &QAction::triggered,            // Settings -> Show Playlist
            [=](bool b)
            {
                ShowPlaylist(b);
            });

    connect(ui->action_Hide_Album_Art_2, &QAction::triggered,           // Settings -> Hide Album Art
            [=](bool b)
            {
                HideAlbumArt(b);
            });

    connect(ui->action_Dim_Desktop, &QAction::triggered,               // Settings -> Dim Lights
            [=](bool b)
            {
                DimDesktop(b);
            });

    connect(ui->actionShow_D_ebug_Output, &QAction::triggered,          // Settings -> Show Debug Output
            [=](bool b)
            {
                setDebug(b);
            });

    connect(ui->action_Preferences, &QAction::triggered,                // Settings -> Preferences...
            [=]
            {
                SaveSettings();
                PreferencesDialog::showPreferences(settings, this);
                LoadSettings();
            });
                                                                        // Help ->
    connect(ui->actionOnline_Help, &QAction::triggered,                 // Help -> Online Help
            [=]
            {
                QDesktopServices::openUrl(QUrl("http://bakamplayer.u8sand.net/help.php"));
            });

    connect(ui->action_Check_for_Updates, &QAction::triggered,          // Help -> Check for Updates
            [=]
            {
                if(UpdateDialog::update(update, this) == QDialog::Accepted)
                {
                    // todo: close and finish update (overwrite self and restart)
                }
            });

    connect(ui->actionAbout_Qt, &QAction::triggered,                    // Help -> About Qt
            [=]
            {
                qApp->aboutQt();
            });

    connect(ui->actionAbout_Baka_MPlayer, &QAction::triggered,          // Help -> About Baka MPlayer
            [=]
            {
                AboutDialog::about(BAKA_MPLAYER_VERSION, this); // launch about dialog
            });

    // qApp

    if(dimDialog) // no need to monitor focus if you can't use dimDialog
    {
        connect(qApp, &QApplication::focusWindowChanged,
                [=](QWindow *focusWindow)
                {
                    // note: focusWindow will be 0 if anything is clicked outside of our program which is useful
                    // the only other problem is that when dragging by the top handle
                    // it will be 0 thus reverting dim desktop, this is a side effect
                    // which will have to stay for now.
                    if(dimDialog->isVisible())
                    {
                        if(focusWindow == 0)
                        {
                            dimDialog->setVisible(false); // remove dim desktop
                            ui->action_Dim_Desktop->setChecked(false); // uncheck dim desktop
                        }
                        else if(focusWindow == dimDialog->windowHandle())
                        {
                            activateWindow();
                            raise();
                            setFocus();
                        }
                    }
                });
        connect(dimDialog, &DimDialog::clicked,
                [=]
                {
                    dimDialog->setVisible(false); // remove dim desktop
                    ui->action_Dim_Desktop->setChecked(false); // uncheck dim desktop
                    activateWindow();
                    raise();
                    setFocus();
                });
    }

    // keyboard shortcuts
    action = new QAction(this);
    action->setShortcut(QKeySequence("Right"));
    connect(action, &QAction::triggered,
            [=]
            {
                mpv->Seek(5, true);
            });
    addAction(action);

    action = new QAction(this);
    action->setShortcut(QKeySequence("Left"));
    connect(action, &QAction::triggered,
            [=]
            {
                mpv->Seek(-5, true);
            });
    addAction(action);

    action = new QAction(this);
    action->setShortcut(QKeySequence("Esc"));
    connect(action, &QAction::triggered,
            [=]
            {
                if(isFullScreen()) // in fullscreen mode, escape will exit fullscreen
                    FullScreen(false);
                else
                {
                    mpv->Pause();
                    setWindowState(windowState() | Qt::WindowMinimized);
                }
            });
    addAction(action);


    // add multimedia shortcuts
    ui->action_Play->setShortcuts({ui->action_Play->shortcut(), QKeySequence(Qt::Key_MediaPlay)});
    ui->action_Stop->setShortcuts({ui->action_Stop->shortcut(), QKeySequence(Qt::Key_MediaStop)});
    ui->actionPlay_Next_File->setShortcuts({ui->actionPlay_Next_File->shortcut(), QKeySequence(Qt::Key_MediaNext)});
    ui->actionPlay_Previous_File->setShortcuts({ui->actionPlay_Previous_File->shortcut(), QKeySequence(Qt::Key_MediaPrevious)});
//    ui->action_Increase_Volume->setShortcuts();
//    ui->action_Decrease_Volume->setShortcuts();

    // set window geometry from settings: leave this out of settings so that preference dialog doesn't center/resize the window
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    QSize(settings->value("window/width", 600).toInt(),
                                          settings->value("window/height", 430).toInt()),
                                    qApp->desktop()->availableGeometry()));
}

MainWindow::~MainWindow()
{
    SaveSettings();

    // cleanup
    delete update;
    delete mpv;
    delete settings;
    delete ui;
}

void MainWindow::LoadSettings()
{
    setOnTop(settings->value("window/onTop", "never").toString());
    setAutoFit(settings->value("window/autoFit", 100).toInt());
    sysTrayIcon->setVisible(settings->value("window/trayIcon", false).toBool());
    setHidePopup(settings->value("window/hidePopup", false).toBool());
    setRemaining(settings->value("window/remaining", true).toBool());
    ui->splitter->setNormalPosition(settings->value("window/splitter", ui->splitter->max()*1.0/8).toInt());
    setDebug(settings->value("common/debug", false).toBool());

    mpv->LoadSettings(settings);
}

void MainWindow::SaveSettings()
{
    mpv->SaveSettings(settings);

    // window
    settings->setValue("window/width", normalGeometry().width());
    settings->setValue("window/height", normalGeometry().height());
    settings->setValue("window/onTop", getOnTop());
    settings->setValue("window/autoFit", getAutoFit());
    settings->setValue("window/trayIcon", sysTrayIcon->isVisible());
    settings->setValue("window/hidePopup", getHidePopup());
    settings->setValue("window/remaining", getRemaining());
    settings->setValue("window/splitter", (ui->splitter->position() == 0 ||
                                           ui->splitter->position() == ui->splitter->max()) ?
                                            ui->splitter->normalPosition() :
                                            ui->splitter->position());
    // mpv
    // common
    settings->setValue("common/debug", getDebug());
}

void MainWindow::Load(QString file)
{
    // load the settings here--the constructor has already been called
    // this solves some issues with setting things before the constructor has ended
    LoadSettings();
    mpv->LoadFile(file);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls() || event->mimeData()->hasText()) // url / text
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls()) // urls
    {
        for(QUrl &url : mimeData->urls())
        {
            if(url.isLocalFile())
                mpv->LoadFile(url.toLocalFile());
            else
                mpv->LoadFile(url.url());
        }
    }
    else if(mimeData->hasText()) // text
        mpv->LoadFile(mimeData->text());
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(!isFullScreen())
    {
        if(event->button() == Qt::LeftButton)
        {
            move = true;
            lastMousePos = event->pos();
        }
        else if(event->button() == Qt::RightButton && mpv->getPlayState() > 0) // if playing
            mpv->PlayPause(ui->playlistWidget->currentRow());
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    move = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    static QRect playbackRect;

    if(move)
    {
        QMainWindow::move(pos()+event->pos()-lastMousePos);
        event->accept();
    }
    else if(isFullScreen())
    {
        setCursor(QCursor(Qt::ArrowCursor)); // show the cursor
        autohide->stop();

        if(!ui->playbackLayoutWidget->isVisible())
        {
            if(playbackRect.contains(event->pos()))
            {
                ui->playbackLayoutWidget->setVisible(true);
                ui->seekBar->setVisible(true);
            }
            else
                autohide->start(500);
        }
        else
        {
            playbackRect = ui->playbackLayoutWidget->geometry();
            playbackRect.setTop(playbackRect.top()-20);
            if(!playbackRect.contains(event->pos()))
            {
                ui->playbackLayoutWidget->setVisible(false);
                ui->seekBar->setVisible(false);
                autohide->start(500);
            }
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->mpvFrame && event->type() == QEvent::MouseMove && isFullScreen())
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
    }
    return false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && ui->mpvFrame->geometry().contains(event->pos())) // if mouse is in the mpvFrame
    {
        if(!isFullScreen() && ui->action_Full_Screen->isEnabled()) // don't allow people to go full screen if they shouldn't be able to
            FullScreen(true);
        else // they can leave fullscreen even if it's disabled (eg. video ends while in full screen)
            FullScreen(false);
        event->accept();
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::SetPlaybackControls(bool enable)
{
    // playback controls
    ui->seekBar->setEnabled(enable);
    ui->rewindButton->setEnabled(enable);
    // next file
    if(enable && mpv->getIndex()+1 < ui->playlistWidget->count()) // not the last entry
    {
        ui->nextButton->setEnabled(true);
        ui->nextButton->setIndex(mpv->getIndex()+2); // starting at 1 instead of at 0 like actual index
        ui->actionPlay_Next_File->setEnabled(true);
    }
    else
    {
        ui->nextButton->setEnabled(false);
        ui->actionPlay_Next_File->setEnabled(false);
    }
    // previous file
    if(enable && mpv->getIndex()-1 >= 0) // not the first entry
    {
        ui->previousButton->setEnabled(true);
        ui->previousButton->setIndex(-mpv->getIndex()); // we use a negative index value for the left button
        ui->actionPlay_Previous_File->setEnabled(true);
    }
    else
    {
        ui->previousButton->setEnabled(false);
        ui->actionPlay_Previous_File->setEnabled(false);
    }
    // menubar
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Full_Screen->setEnabled(enable);
    if(!enable)
    {
        ui->action_Hide_Album_Art_2->setEnabled(false);
        ui->menuSubtitle_Track->setEnabled(false);
        ui->menuFont_Si_ze->setEnabled(false);
    }
}

QString MainWindow::FormatTime(int _time)
{
    const Mpv::FileInfo &fi = mpv->getFileInfo();
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(fi.length >= 3600) // hours
        return time.toString("h:mm:ss");
    if(fi.length >= 60)   // minutes
        return time.toString("mm:ss");
    return time.toString("0:ss");   // seconds
}

bool MainWindow::SetScreenshotDir()
{
    QMessageBox::information(this, "Take Screenshot",
                             "Choose the default location where you would like to save your screenshots. Also by default, we will save your screenshots as a jpg file. If you'd like to change any of these settings, it is under Preferences.");
    QString dir = QFileDialog::getExistingDirectory(this, "Screenshot Directory");
    if(dir != "")
    {
        mpv->ScreenshotDirectory(dir);
        return true;
    }
    return false;
}

void MainWindow::FullScreen(bool fs)
{
    if(fs)
    {
        if(dimDialog && dimDialog->isVisible())
        {
            dimDialog->setVisible(false);
            ui->action_Dim_Desktop->setChecked(false);
        }
        setWindowState(windowState() | Qt::WindowFullScreen);
        ui->menubar->setVisible(false);
        ShowPlaylist(false);
        setMouseTracking(true); // register mouse move event

        // post a mouseMoveEvent (in case user doesn't actually move the mouse when entering fs)
        QMouseEvent *event = new QMouseEvent(QMouseEvent::MouseMove,
                                             QCursor::pos(),
                                             Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QCoreApplication::postEvent(this, event);
    }
    else
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        ui->menubar->setVisible(true);
        ui->seekBar->setVisible(true);
        ui->playbackLayoutWidget->setVisible(true);
        setMouseTracking(false); // stop registering mouse move event
        setCursor(QCursor(Qt::ArrowCursor)); // show cursor
        autohide->stop();
    }
}

void MainWindow::ShowPlaylist(bool visible)
{
    if(visible)
        ui->splitter->setPosition(ui->splitter->normalPosition()); // bring splitter position to normal
    else
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save current splitter position as the normal position
        ui->splitter->setPosition(0); // set splitter position to right-most
    }
}

void MainWindow::HideAlbumArt(bool hide)
{
    if(hide)
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save splitter position as the normal position
        ui->splitter->setPosition(ui->splitter->max()); // bring the splitter position to the left-most
    }
    else
        ui->splitter->setPosition(ui->splitter->normalPosition()); // bring the splitter to normal position
}

void MainWindow::FitWindow(int percent)
{
    if(isFullScreen())
        return;
    if(isMaximized())
        setWindowFlags(windowFlags() & ~Qt::WindowMaximized);

    mpv->LoadVideoParams();
    const Mpv::VideoParams &params = mpv->getFileInfo().video_params;
    QRect fG = ui->mpvFrame->geometry(), // frame geometry
          cG = geometry(), // current geometry of window
          dG = qApp->desktop()->availableGeometry(); // desktop geometry
    int w, h;
    double a;

    // get aspect ratio
    if(params.width == 0 || params.height == 0) // width/height are 0 when there is no output
    	return;
    if(params.dwidth == 0 || params.dheight == 0) // dwidth/height are 0 on load
        a = (double)params.width/params.height; // use video width and height for aspect ratio
    else
        a = (double)params.dwidth/params.dheight; // use display geometry for aspect ratio

    // get width and height of new display
    if(percent == 0) // fit to window
    {
        w = fG.width();
        h = fG.height();
    }
    else
    {
        double scale = percent/100.0;
        w = params.width*scale;
        h = (params.width/a)*scale; // get height from aspect ratio

        // bigger than desktop geometry correction
        // todo: explain how this works, I came up with the algorithm and
        // simplified but intuitively it's hard to understand
        if(w + (frameGeometry().width() - fG.width()) > dG.width())
        {
            w = dG.width() - frameGeometry().width() + fG.width();
            h = w/a;
        }
        if(h + (frameGeometry().height() - fG.height()) > dG.height())
        {
            h = dG.height() - frameGeometry().height() + fG.height();
            w = a*h;
        }
    }

    // autofit algorithm
    if((double)w/h > a) // width > what it's supposed to be
        w = a*h;
    else                // height > what it's supposed to be
        h = w/a;

    // add the size of the things not in the frame
    w += cG.width() - fG.width();
    h += cG.height() - fG.height();

    // set window position
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    QSize(w, h),
                                    dG));
}

void MainWindow::SetAspectRatio(QString aspect)
{
    if(isFullScreen())
        return;
    mpv->Aspect(aspect);
}

void MainWindow::DimDesktop(bool dim)
{
    if(!dimDialog) // dimDialog is NULL if desktop compositor is disabled or missing
    {
        QMessageBox::information(this, "Dim Desktop", "In order to dim the desktop, the desktop compositor has to be enabled. This can be done through Window Manager Desktop.");
        ui->action_Dim_Desktop->setChecked(false);
        return;
    }
    if(dim)
        dimDialog->show();
    else
        dimDialog->close();
}

void MainWindow::AlwaysOnTop(bool ontop)
{
    // maybe in the future, Linux X specific code that way we could enable it for both platforms
#if defined(Q_OS_WIN)
    SetWindowPos((HWND)winId(),
                 ontop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
#elif defined(Q_OS_LINUX)
    Display *display = QX11Info::display();
    XEvent event;
    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.display = display;
    event.xclient.window  = winId();
    event.xclient.message_type = XInternAtom (display, "_NET_WM_STATE", False);
    event.xclient.format = 32;

    event.xclient.data.l[0] = ontop;
    event.xclient.data.l[1] = XInternAtom (display, "_NET_WM_STATE_ABOVE", False);
    event.xclient.data.l[2] = 0; //unused.
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    XSendEvent(display, DefaultRootWindow(display), False,
                           SubstructureRedirectMask|SubstructureNotifyMask, &event);
#else // qt code
    if(ontop)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    show();
#endif
}
