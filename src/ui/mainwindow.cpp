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
#include <QAction>
#include <QShortcut>
#include <QProcess>
#include <QIcon>
#include <QWindow>
#include <QCheckBox>
#include <QLibraryInfo>
#include <QCursor>

#include "aboutdialog.h"
#include "infodialog.h"
#include "locationdialog.h"
#include "jumpdialog.h"
#include "inputdialog.h"
#include "updatedialog.h"
#include "preferencesdialog.h"
#include "screenshotdialog.h"
#include "util.h"
#include "gesturehandler.h"
#include "bakaengine.h"
#include "widgets/dimdialog.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // update streaming support disabled on unix platforms
    ui->actionUpdate_Streaming_Support->setEnabled(false);
#endif
    ShowPlaylist(false);
    addActions(ui->menubar->actions()); // makes menubar shortcuts work even when menubar is hidden

    // initialize managers/handlers
    baka = new BakaEngine(this);
    mpv = baka->mpv;

    ui->playlistWidget->AttachEngine(baka);
    ui->mpvFrame->installEventFilter(this); // capture events on mpvFrame in the eventFilter function
    autohide = new QTimer(this);

    // command action mappings (action (right) performs command (left))
    commandActionMap = {
        {"mpv add chapter +1", ui->action_Next_Chapter},
        {"mpv add chapter -1", ui->action_Previous_Chapter},
        {"mpv set sub-scale 1", ui->action_Reset_Size},
        {"mpv add sub-scale +0.02", ui->action_Size},
        {"mpv add sub-scale -0.02", ui->actionS_ize},
        {"mpv osd-msg screenshot subtitles", ui->actionWith_Subtitles},
        {"mpv osd-msg screenshot video", ui->actionWithout_Subtitles},
        {"mpv osd-msg add speed +0.25", ui->action_Increase},
        {"mpv osd-msg add speed -0.25", ui->action_Decrease},
        {"mpv osd-msg set speed 1", ui->action_Reset},
        {"mpv osd-msg set video-aspect -1", ui->action_Auto_Detect},
        {"mpv osd-msg set video-aspect 16:9", ui->actionForce_16_9},
        {"mpv osd-msg set video-aspect 2_35:1", ui->actionForce_2_35_1},
        {"mpv osd-msg set video-aspect 4:3", ui->actionForce_4_3},
        {"mpv osd-msg cycle sub-visibility", ui->actionShow_Subtitles},
        {"mpv set time-pos 0", ui->action_Restart},
        {"mpv frame_step", ui->action_Frame_Step},
        {"mpv frame_back_step", ui->actionFrame_Back_Step},
        {"baka add_subtitles", ui->action_Add_Subtitle_File},
        {"baka fitwindow 0", ui->action_To_Current_Size},
        {"baka fitwindow 50", ui->action50},
        {"baka fitwindow 75", ui->action75},
        {"baka fitwindow 100", ui->action100},
        {"baka fitwindow 150", ui->action150},
        {"baka fitwindow 200", ui->action200},
        {"baka fullscreen", ui->action_Full_Screen},
        {"baka jump", ui->action_Jump_to_Time},
        {"baka media_info", ui->actionMedia_Info},
        {"baka new", ui->action_New_Player},
        {"baka open", ui->action_Open_File},
        {"baka open_clipboard", ui->actionOpen_Path_from_Clipboard},
        {"baka open_location", ui->actionOpen_URL},
        {"baka play +1", ui->actionPlay_Next_File},
        {"baka play -1", ui->actionPlay_Previous_File},
        {"baka playlist repeat off", ui->action_Off},
        {"baka playlist repeat playlist", ui->action_Playlist},
        {"baka playlist repeat this", ui->action_This_File},
        {"baka playlist shuffle", ui->actionSh_uffle},
        {"baka playlist toggle", ui->action_Show_Playlist},
        {"baka playlist full", ui->action_Hide_Album_Art},
        {"baka dim", ui->action_Dim_Lights},
        {"baka play_pause", ui->action_Play},
        {"baka quit", ui->actionE_xit},
        {"baka show_in_folder", ui->actionShow_in_Folder},
        {"baka stop", ui->action_Stop},
        {"baka volume +5", ui->action_Increase_Volume},
        {"baka volume -5", ui->action_Decrease_Volume},
        {"baka output", ui->actionShow_D_ebug_Output},
        {"baka preferences", ui->action_Preferences},
        {"baka online_help", ui->actionOnline_Help},
        {"baka update", ui->action_Check_for_Updates},
        {"baka update youtube-dl", ui->actionUpdate_Streaming_Support},
        {"baka about qt", ui->actionAbout_Qt},
        {"baka about", ui->actionAbout_Baka_MPlayer}
    };

    // map actions to commands
    for(auto action = commandActionMap.begin(); action != commandActionMap.end(); ++action)
    {
        const QString cmd = action.key();
        connect(*action, &QAction::triggered,
                [=] { baka->Command(cmd); });
    }

    // setup signals & slots

    // mainwindow
    connect(this, &MainWindow::langChanged,
            [=](QString lang)
            {
                if(lang == "auto") // fetch lang from locale
                    lang = QLocale::system().name();

                if(lang != "en")
                {
                    QTranslator *tmp;

                    // load the system translations provided by Qt
                    tmp = baka->qtTranslator;
                    baka->qtTranslator = new QTranslator();
                    baka->qtTranslator->load(QString("qt_%0").arg(lang), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
                    qApp->installTranslator(baka->qtTranslator);
                    if(tmp != nullptr)
                        delete tmp;

                    // load the application translations
                    tmp = baka->translator;
                    baka->translator = new QTranslator();
                    baka->translator->load(QString("baka-mplayer_%0").arg(lang), BAKA_MPLAYER_LANG_PATH);
                    qApp->installTranslator(baka->translator);
                    if(tmp != nullptr)
                        delete tmp;
                }
                else
                {
                    if(baka->qtTranslator != nullptr)
                        qApp->removeTranslator(baka->qtTranslator);
                    if(baka->translator != nullptr)
                        qApp->removeTranslator(baka->translator);
                }

                // save strings we want to keep
                QString title = windowTitle(),
                        duration = ui->durationLabel->text(),
                        remaining = ui->remainingLabel->text(),
                        index = ui->indexLabel->text();

                ui->retranslateUi(this);

                // reload strings we kept
                setWindowTitle(title);
                ui->durationLabel->setText(duration);
                ui->remainingLabel->setText(remaining);
                ui->indexLabel->setText(index);
            });

    connect(this, &MainWindow::onTopChanged,
            [=](QString onTop)
            {
                if(onTop == "never")
                    Util::SetAlwaysOnTop(winId(), false);
                else if(onTop == "always")
                    Util::SetAlwaysOnTop(winId(), true);
                else if(onTop == "playing" && mpv->getPlayState() > 0)
                    Util::SetAlwaysOnTop(winId(), true);
            });

    connect(this, &MainWindow::remainingChanged,
            [=](bool b)
            {
                if(!b)
                    ui->remainingLabel->setText(Util::FormatTime(mpv->getFileInfo().length, mpv->getFileInfo().length));
                else
                    ui->remainingLabel->setText("-"+Util::FormatTime(mpv->getFileInfo().length - mpv->getTime(), mpv->getFileInfo().length));
            });

    connect(this, &MainWindow::debugChanged,
            [=](bool b)
            {
                ui->actionShow_D_ebug_Output->setChecked(b);
                ui->verticalWidget->setVisible(b);
            });

    connect(baka->sysTrayIcon, &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason)
            {
                if(reason == QSystemTrayIcon::Trigger)
                {
                    if(!hidePopup)
                    {
                        if(mpv->getPlayState() == Mpv::Playing)
                            baka->sysTrayIcon->showMessage("Baka MPlayer", tr("Playing"), QSystemTrayIcon::NoIcon, 4000);
                        else if(mpv->getPlayState() == Mpv::Paused)
                            baka->sysTrayIcon->showMessage("Baka MPlayer", tr("Paused"), QSystemTrayIcon::NoIcon, 4000);
                    }
                    baka->PlayPause();
                }

            });

    //    connect(this, &MainWindow::hidePopupChanged,
    //            [=](bool b)
    //    {
    //    });

    connect(autohide, &QTimer::timeout, // cursor autohide
            [=]
            {
                setCursor(QCursor(Qt::BlankCursor));
                autohide->stop();
            });

    // dimDialog
    connect(baka->dimDialog, &DimDialog::visbilityChanged,
            [=](bool dim)
            {
                ui->action_Dim_Lights->setChecked(dim);
                if(dim)
                    Util::SetAlwaysOnTop(winId(), true);
                else if(onTop == "never" || (onTop == "playing" && mpv->getPlayState() > 0))
                    Util::SetAlwaysOnTop(winId(), false);
            });

    // mpv

    connect(mpv, &MpvHandler::playlistChanged,
            [=](const QStringList &list)
            {
                if(list.length() > 1)
                {
                    ui->actionSh_uffle->setEnabled(true);
                    ui->actionStop_after_Current->setEnabled(true);
                    ShowPlaylist(true);
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
            });

    connect(mpv, &MpvHandler::fileInfoChanged,
            [=](const Mpv::FileInfo &fileInfo)
            {
                if(mpv->getPlayState() > 0)
                {
                    if(fileInfo.media_title == "")
                        setWindowTitle("Baka MPlayer");
                    else if(fileInfo.media_title == "-")
                        setWindowTitle("Baka MPlayer: stdin"); // todo: disable playlist?
                    else
                        setWindowTitle(fileInfo.media_title);

                    // todo: deal with streamed input for which we do not know the length
                    ui->seekBar->setTracking(fileInfo.length);

                    if(!remaining)
                        ui->remainingLabel->setText(Util::FormatTime(fileInfo.length, fileInfo.length));
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
            ui->menuAudio_Tracks->clear();
            for(auto &track : trackList)
            {
                if(track.type == "sub")
                {
                    action = ui->menuSubtitle_Track->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
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
                                mpv->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Sub"), QString::number(track.id), track.title, track.lang));
                            });
                }
                else if(track.type == "audio")
                {
                    action = ui->menuAudio_Tracks->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
                    connect(action, &QAction::triggered,
                            [=]
                            {
                                if(mpv->getAid() != track.id) // don't allow selection of the same track
                                {
                                    mpv->Aid(track.id);
                                    mpv->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Audio"), QString::number(track.id), track.title, track.lang));
                                }
                                else
                                    action->setChecked(true); // recheck the track
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
                if(ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                    ui->mpvFrame->setStyleSheet("");
                if(ui->action_Hide_Album_Art->isChecked())
                    HideAlbumArt(false);
                ui->action_Hide_Album_Art->setEnabled(false);
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
                ui->menuAudio_Tracks->setEnabled((ui->menuAudio_Tracks->actions().count() > 0));
                if(ui->menuAudio_Tracks->actions().count() == 1)
                    ui->menuAudio_Tracks->actions().first()->setEnabled(false);
                ui->menuTake_Screenshot->setEnabled(true);
                ui->menuFit_Window->setEnabled(true);
                ui->menuAspect_Ratio->setEnabled(true);
                ui->action_Frame_Step->setEnabled(true);
                ui->actionFrame_Back_Step->setEnabled(true);
            }
            else
            {
                if(!albumArt)
                {
                    // put in filler albumArt
                    if(ui->mpvFrame->styleSheet() == QString())
                        ui->mpvFrame->setStyleSheet("background-image:url(:/img/album_art.png);background-repeat:no-repeat;background-position:center;");
                }
                ui->action_Hide_Album_Art->setEnabled(true);
                ui->menuAudio_Tracks->setEnabled((ui->menuAudio_Tracks->actions().count() > 1));
                ui->menuSubtitle_Track->setEnabled(false);
                ui->menuFont_Si_ze->setEnabled(false);
                ui->actionShow_Subtitles->setEnabled(false);
                ui->menuTake_Screenshot->setEnabled(false);
                ui->menuFit_Window->setEnabled(false);
                ui->menuAspect_Ratio->setEnabled(false);
                ui->action_Frame_Step->setEnabled(false);
                ui->actionFrame_Back_Step->setEnabled(false);


                if(baka->sysTrayIcon->isVisible() && !hidePopup)
                {
                    // todo: use {artist} - {title}
                    baka->sysTrayIcon->showMessage("Baka MPlayer", mpv->getFileInfo().media_title, QSystemTrayIcon::NoIcon, 4000);
                }
            }
        }
    });

    connect(mpv, &MpvHandler::chaptersChanged,
            [=](const QList<Mpv::Chapter> &chapters)
    {
        if(mpv->getPlayState() > 0)
        {
            QAction *action;
            QList<int> ticks;
            int n = 1,
                N = chapters.length();
            ui->menu_Chapters->clear();
            for(auto &ch : chapters)
            {
                action = ui->menu_Chapters->addAction(QString("%0: %1").arg(Util::FormatNumberWithAmpersand(n, N), ch.title));
                if(n <= 9)
                    action->setShortcut(QKeySequence("Ctrl+"+QString::number(n)));
                connect(action, &QAction::triggered,
                        [=]
                        {
                            mpv->Seek(ch.time);
                        });
                ticks.push_back(ch.time);
                n++;
            }
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
                        ui->action_Show_Playlist->setEnabled(true);
                        init = true;
                    }
                    if(pathChanged && autoFit)
                    {
                        baka->FitWindow(autoFit, false);
                        pathChanged = false;
                    }
                    SetPlaybackControls(true);
                    mpv->Play();
                case Mpv::Playing:
                    ui->playButton->setIcon(QIcon(":/img/default_pause.svg"));
                    ui->action_Play->setText(tr("&Pause"));
                    if(onTop == "playing")
                        Util::SetAlwaysOnTop(winId(), true);
                    break;

                case Mpv::Paused:
                case Mpv::Stopped:
                    ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
                    ui->action_Play->setText(tr("&Play"));
                    if(onTop == "playing")
                        Util::SetAlwaysOnTop(winId(), false);
                    break;

                case Mpv::Idle:
                    if(init)
                    {
                        if(ui->action_This_File->isChecked())
                            ui->playlistWidget->PlayIndex(0, true); // restart file
                        else if(ui->playlistWidget->currentRow() >= ui->playlistWidget->count()-1 ||
                           ui->actionStop_after_Current->isChecked())
                        {
                            if(ui->action_Playlist->isChecked() && ui->playlistWidget->count() > 0)
                                ui->playlistWidget->PlayIndex(0); // restart playlist
                            else
                            {
                                setWindowTitle("Baka MPlayer");
                                SetPlaybackControls(false);
                                ui->seekBar->setTracking(0);
                                ui->actionStop_after_Current->setChecked(false);
                                if(ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                                    ui->mpvFrame->setStyleSheet("");
                            }
                        }
                        else
                            ui->playlistWidget->PlayIndex(1, true);
                    }
                    break;
                }
            });

    connect(mpv, &MpvHandler::pathChanged,
            [=]()
            {
                pathChanged = true;
            });

    connect(mpv, &MpvHandler::fileChanged,
            [=](QString f)
            {
                QString file = mpv->getPath()+f;
                if((recent.isEmpty() || recent.front() != file) &&
                   f != QString() &&
                   maxRecent > 0)
                {
                    UpdateRecentFiles(); // update after initialization and only if the current file is different from the first recent
                    recent.removeAll(file);
                    while(recent.length() > maxRecent-1)
                        recent.removeLast();
                    recent.push_front(file);
                }
            });

    connect(mpv, &MpvHandler::timeChanged,
            [=](int i)
            {
                const Mpv::FileInfo &fi = mpv->getFileInfo();
                // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
                // the formula is a simple ratio seekBar's max * time/totalTime
                ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)i/fi.length));

                // set duration and remaining labels, QDateTime takes care of formatting for us
                ui->durationLabel->setText(Util::FormatTime(i, mpv->getFileInfo().length));
                if(remaining)
                    ui->remainingLabel->setText("-"+Util::FormatTime(fi.length-i, mpv->getFileInfo().length));

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

    connect(mpv, &MpvHandler::speedChanged,
            [=](double speed)
            {
                static double last = 1;
                if(last != speed)
                {
                    if(init)
                        mpv->ShowText(tr("Speed: %0x").arg(QString::number(speed)));
                    if(speed <= 0.25)
                        ui->action_Decrease->setEnabled(false);
                    else
                        ui->action_Decrease->setEnabled(true);
                    last = speed;
                }
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

    connect(mpv, &MpvHandler::aidChanged,
            [=](int aid)
            {
                QList<QAction*> actions = ui->menuAudio_Tracks->actions();
                for(auto &action : actions)
                {
                    if(action->text().startsWith(QString::number(aid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                        action->setChecked(false);
                }
            });

    connect(mpv, &MpvHandler::subtitleVisibilityChanged,
            [=](bool b)
            {
                ui->actionShow_Subtitles->setChecked(b);
            });

    // ui

    connect(ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                mpv->Seek(mpv->Relative(((double)i/ui->seekBar->maximum())*mpv->getFileInfo().length), true);
            });

    connect(ui->openButton, &OpenButton::LeftClick,                     // Playback: Open button (left click)
            [=]
            {
                baka->Open();
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
                mpv->LoadFile(LocationDialog::getUrl(mpv->getPath()+mpv->getFile(), this));
            });

    connect(ui->remainingLabel, &CustomLabel::clicked,                  // Playback: Remaining Label
            [=]
            {
                setRemaining(!remaining);
            });

    connect(ui->rewindButton, &QPushButton::clicked,                    // Playback: Rewind button
            [=]
            {
                mpv->Rewind();
            });

    connect(ui->previousButton, &IndexButton::clicked,                  // Playback: Previous button
            [=]
            {
                ui->playlistWidget->PlayIndex(-1, true);
            });

    connect(ui->playButton, &QPushButton::clicked,                      // Playback: Play/pause button
            [=]
            {
                baka->PlayPause();
            });

    connect(ui->nextButton, &IndexButton::clicked,                      // Playback: Next button
            [=]
            {
                ui->playlistWidget->PlayIndex(1, true);
            });

    connect(ui->volumeSlider, &CustomSlider::valueChanged,              // Playback: Volume slider adjusted
            [=](int i)
            {
                mpv->Volume(i);
            });

    connect(ui->playlistButton, &QPushButton::clicked,                  // Playback: Clicked the playlist button
            [=]
            {
                TogglePlaylist();
            });

    connect(ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                blockSignals(true);
                if(i == 0) // right-most, playlist is hidden
                {
                    ui->action_Show_Playlist->setChecked(false);
                    ui->action_Hide_Album_Art->setChecked(false);
                }
                else if(i == ui->splitter->max()) // left-most, album art is hidden, playlist is visible
                {
                    ui->action_Show_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    ui->action_Show_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art->setChecked(false);
                }
                blockSignals(false);
            });

    connect(ui->searchBox, &QLineEdit::textChanged,                     // Playlist: Search box
            [=](QString s)
            {
                ui->playlistWidget->Search(s);
            });

    connect(ui->indexLabel, &CustomLabel::clicked,                      // Playlist: Clicked the indexLabel
            [=]
            {
                QString res = InputDialog::getInput(tr("Enter the file number you want to play:\nNote: Value must be from %0 - %1").arg("1", QString::number(ui->playlistWidget->count())),
                                                    tr("Enter File Number"),
                                                    [this](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if(in >= 1 && in <= ui->playlistWidget->count())
                                                            return true;
                                                        return false;
                                                    },
                                                    this);
                if(res != "")
                    ui->playlistWidget->PlayIndex(res.toInt()-1); // user index will be 1 greater than actual
            });

    connect(ui->playlistWidget, &PlaylistWidget::currentRowChanged,     // Playlist: Playlist selection changed
            [=](int)
            {
                SetIndexLabels(true);
            });

    connect(ui->currentFileButton, &QPushButton::clicked,               // Playlist: Select current file button
            [=]
            {
                ui->playlistWidget->SelectIndex(ui->playlistWidget->CurrentIndex());
            });

    connect(ui->hideFilesButton, &QPushButton::clicked,                 // Playlist: Hide files button
            [=](bool b)
            {
                ui->playlistWidget->ShowAll(!b);
            });

    connect(ui->refreshButton, &QPushButton::clicked,                   // Playlist: Refresh playlist button
            [=]
            {
                ui->playlistWidget->RefreshPlaylist();
            });

    connect(ui->inputLineEdit, &CustomLineEdit::submitted,
            [=](QString s)
            {
                baka->Command(s);
                ui->inputLineEdit->setText("");
            });

    // add multimedia shortcuts
    ui->action_Play->setShortcuts({ui->action_Play->shortcut(), QKeySequence(Qt::Key_MediaPlay)});
    ui->action_Stop->setShortcuts({ui->action_Stop->shortcut(), QKeySequence(Qt::Key_MediaStop)});
    ui->actionPlay_Next_File->setShortcuts({ui->actionPlay_Next_File->shortcut(), QKeySequence(Qt::Key_MediaNext)});
    ui->actionPlay_Previous_File->setShortcuts({ui->actionPlay_Previous_File->shortcut(), QKeySequence(Qt::Key_MediaPrevious)});
}

MainWindow::~MainWindow()
{
    baka->SaveSettings();

    // Note: child objects _should_ not need to be deleted because
    // all children should get deleted when mainwindow is deleted
    // see: http://qt-project.org/doc/qt-4.8/objecttrees.html

    // but apparently they don't (https://github.com/u8sand/Baka-MPlayer/issues/47)

    delete baka;
    delete ui;
}

void MainWindow::Load(QString file)
{
    // load the settings here--the constructor has already been called
    // this solves some issues with setting things before the constructor has ended
    menuVisible = ui->menubar->isVisible(); // does the OS use a menubar? (appmenu doesn't)
    baka->LoadSettings();
    mpv->LoadFile(file);
}

void MainWindow::MapShortcuts()
{
    auto tmp = commandActionMap;
    // map shortcuts to actions
    for(auto input_iter = baka->input.begin(); input_iter != baka->input.end(); ++input_iter)
    {
        auto commandAction = tmp.find(input_iter->first);
        if(commandAction != tmp.end())
        {
            (*commandAction)->setShortcut(QKeySequence(input_iter.key()));
            tmp.erase(commandAction);
        }
    }
    // clear the rest
    for(auto iter = tmp.begin(); iter != tmp.end(); ++iter)
        (*iter)->setShortcut(QKeySequence());
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
    if(event->button() == Qt::LeftButton)
    {
        if(gestures)
        {
            if(ui->mpvFrame->rect().contains(event->pos())) // mouse is in the mpvFrame
                baka->gesture->Begin(GestureHandler::HSEEK_VVOLUME, event->globalPos(), pos());
            else if(!isFullScreen()) // not fullscreen
                baka->gesture->Begin(GestureHandler::MOVE, event->globalPos(), pos());
        }
        else if(!isFullScreen()) // not fullscreen
            baka->gesture->Begin(GestureHandler::MOVE, event->globalPos(), pos());
    }
    else if(event->button() == Qt::RightButton &&
            !isFullScreen() &&  // not fullscreen
            mpv->getPlayState() > 0 &&  // playing
            ui->mpvFrame->rect().contains(event->pos())) // mouse is in the mpvFrame
    {
        mpv->PlayPause(ui->playlistWidget->CurrentItem());
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    baka->gesture->End();
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    static QRect playbackRect,
                 playlistRect;

    if(baka->gesture->Process(event->globalPos()))
        event->accept();
    else if(isFullScreen())
    {
        bool in = false;
        setCursor(QCursor(Qt::ArrowCursor)); // show the cursor
        autohide->stop();

        if(!ui->playbackLayoutWidget->isVisible())
        {
            if(playbackRect.contains(event->globalPos()))
            {
                ui->playbackLayoutWidget->setVisible(true);
                ui->seekBar->setVisible(true);
                in = true;
            }
        }
        else
        {
            playbackRect = qApp->desktop()->rect();
            playbackRect.setTop(playbackRect.bottom() - ui->playbackLayoutWidget->height() - 20);

            if(!playbackRect.contains(event->globalPos()))
            {
                ui->playbackLayoutWidget->setVisible(false);
                ui->seekBar->setVisible(false);
            }
            else
                in = true;
        }

        if(isPlaylistVisible())
        {
            playlistRect = ui->playlistLayoutWidget->geometry();
            playlistRect.moveTo(ui->playlistLayoutWidget->pos());
            playlistRect.setLeft(playlistRect.left()-20);
            playlistRect.setRight(playlistRect.right()+5);
            playlistRect.setHeight(height());

            if(!playlistRect.contains(event->globalPos()))
                ShowPlaylist(false);
            else
                in = true;
        }

        if(!in)
            autohide->start(500);
    }
    QMainWindow::mouseMoveEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->mpvFrame && isFullScreen() && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
    }
    else if(event->type() == 6) // QEvent::KeyPress = 6  (but using QEvent::KeyPress causes compile errors, not sure why)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        keyPressEvent(keyEvent);
    }
    else if(obj == ui->splitter->handle(1) && event->type() == QEvent::Enter && !isPlaylistVisible())
    {
        ShowPlaylist(true);
        autohide->stop();
    }
    return false;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        mpv->Volume(mpv->getVolume()+5);
    else
        mpv->Volume(mpv->getVolume()-5);
    QMainWindow::wheelEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // keyboard shortcuts
    if(!baka->input.empty())
    {
        // Convert KeyEvent to Shortcut:
        QString key = QString();
        if(event->modifiers() & Qt::MetaModifier)    key += "Meta+";
        if(event->modifiers() & Qt::ControlModifier) key += "Ctrl+";
        if(event->modifiers() & Qt::AltModifier)     key += "Alt+";
        if(event->modifiers() & Qt::ShiftModifier)   key += "Shift+";
        key += QKeySequence(event->key()).toString();

        // find shortcut in input hash table
        auto iter = baka->input.find(key);
        if(iter != baka->input.end())
            baka->Command(iter->first); // execute command
    }
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

void MainWindow::SetIndexLabels(bool enable)
{
    int i = ui->playlistWidget->currentRow(),
        index = ui->playlistWidget->CurrentIndex();

    // next file
    if(enable && index+1 < ui->playlistWidget->count()) // not the last entry
    {
        ui->nextButton->setEnabled(true);
        ui->nextButton->setIndex(index+2); // starting at 1 instead of at 0 like actual index
        ui->actionPlay_Next_File->setEnabled(true);
    }
    else
    {
        ui->nextButton->setEnabled(false);
        ui->actionPlay_Next_File->setEnabled(false);
    }
    // previous file
    if(enable && index-1 >= 0) // not the first entry
    {
        ui->previousButton->setEnabled(true);
        ui->previousButton->setIndex(-index); // we use a negative index value for the left button
        ui->actionPlay_Previous_File->setEnabled(true);
    }
    else
    {
        ui->previousButton->setEnabled(false);
        ui->actionPlay_Previous_File->setEnabled(false);
    }

    if(i == -1) // no selection
    {
        ui->indexLabel->setText(tr("No selection"));
        ui->indexLabel->setEnabled(false);
    }
    else
    {
        ui->indexLabel->setEnabled(true);
        ui->indexLabel->setText(tr("File %0 of %1").arg(QString::number(i+1), QString::number(ui->playlistWidget->count())));
    }
}

void MainWindow::SetPlaybackControls(bool enable)
{
    // playback controls
    ui->seekBar->setEnabled(enable);
    ui->rewindButton->setEnabled(enable);

    SetIndexLabels(enable);

    // menubar
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->menuS_peed->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Full_Screen->setEnabled(enable);
    if(!enable)
    {
        ui->action_Hide_Album_Art->setEnabled(false);
        ui->menuSubtitle_Track->setEnabled(false);
        ui->menuAudio_Tracks->setEnabled(false);
        ui->menuFont_Si_ze->setEnabled(false);
    }
}

void MainWindow::FullScreen(bool fs)
{
    if(fs)
    {
        if(baka->dimDialog && baka->dimDialog->isVisible())
            baka->Dim(false);
        setWindowState(windowState() | Qt::WindowFullScreen);
        ui->menubar->setVisible(false);
        ui->splitter->handle(1)->installEventFilter(this); // capture events for the splitter
        setMouseTracking(true); // register mouse move event
        setContextMenuPolicy(Qt::ActionsContextMenu);

        // post a mouseMoveEvent to autohide
        QCoreApplication::postEvent(this, new QMouseEvent(QMouseEvent::MouseMove,
                                                          QCursor::pos(),
                                                          Qt::NoButton,Qt::NoButton,Qt::NoModifier));
    }
    else
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        if(menuVisible)
            ui->menubar->setVisible(true);
        ui->seekBar->setVisible(true);
        ui->playbackLayoutWidget->setVisible(true);
        ui->splitter->handle(1)->removeEventFilter(this);
        setMouseTracking(false); // stop registering mouse move event
        setContextMenuPolicy(Qt::NoContextMenu);
        setCursor(QCursor(Qt::ArrowCursor)); // show cursor
        autohide->stop();
    }
}

bool MainWindow::isPlaylistVisible()
{
    // if the position is 0, playlist is hidden
    return (ui->splitter->position() != 0);
}

void MainWindow::TogglePlaylist()
{
    ShowPlaylist(!isPlaylistVisible());
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
        setFocus();
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

void MainWindow::TakeScreenshot(bool subs)
{
    if(screenshotDialog)
    {
        mpv->Pause();
        if(ScreenshotDialog::showScreenshotDialog(screenshotDialog, subs, mpv) != QDialog::Accepted)
            return;
    }
    else
        mpv->Screenshot(subs);
    ShowScreenshotMessage(subs);
}

void MainWindow::ShowScreenshotMessage(bool subs)
{
    QString dir = mpv->getScreenshotDir();
    int i = dir.lastIndexOf('/');
    if(i != -1)
        dir.remove(0, i+1);
    if(subs)
        mpv->ShowText(tr("Saved to \"%0\", with subs").arg(dir));
    else
        mpv->ShowText(tr("Saved to \"%0\", without subs").arg(dir));
}

void MainWindow::UpdateRecentFiles()
{
    ui->menu_Recently_Opened->clear();
    QAction *action;
    int n = 1,
        N = recent.length();
    for(auto &f : recent)
    {
        action = ui->menu_Recently_Opened->addAction(QString("%0. %1").arg(Util::FormatNumberWithAmpersand(n, N), Util::ShortenPathToParent(f).replace("&","&&")));
        if(n++ == 1)
            action->setShortcut(QKeySequence("Ctrl+Z"));
        connect(action, &QAction::triggered,
                [=]
                {
                    mpv->LoadFile(f);
                });
    }
}
