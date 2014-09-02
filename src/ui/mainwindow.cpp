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

#ifdef Q_OS_WIN
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
    dragging(false),
    init(false)
{
    light = new LightDialog(); // lightdialog must be initialized before ui is setup
    ui->setupUi(this);
    addActions(ui->menubar->actions()); // makes menubar shortcuts work even when menubar is hidden

    // initialize managers/handlers
#if Q_OS_WIN // saves to $(application directory)\${SETTINGS_FILE}.ini
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

//#ifdef Q_WIN_OS
//    connect(this, &MainWindow::onTopChanged,
//            [=](QString onTop)
//            {
//                if(onTop == "never")
//                    AlwaysOnTop(true);
//                else if(trayIcon == "always")
//                    AlwaysOnTop(false);
//            });
//#endif

//    connect(this, &MainWindow::autoFitChanged,
//            [=](int i)
//            {
//            });

//    connect(this, &MainWindow::trayIconChanged,
//            [=](bool b)
//            {
//                trayIcon->setVisible(b);
//            });

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

    // mpv

    connect(mpv, &MpvHandler::volumeChanged,
            [=](int volume)
            {
                ui->volumeSlider->setValueNoSignal(volume);
            });

    connect(mpv, &MpvHandler::fileInfoChanged,
            [=](const Mpv::FileInfo &i) // todo: cleanup
            {
                if(mpv->getPlayState() != Mpv::Idle)
                {
                    // load chapter list into menus
                    QList<int> ticks;
                    QSignalMapper *signalMapper = new QSignalMapper(this);
                    int n = 1;
                    ui->menu_Chapters->clear();
                    for(auto &ch : i.chapters)
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
                        ui->menu_Chapters->addAction("[ none ]")->setEnabled(false);
                    ui->seekBar->setTicks(ticks);

                    // load subtitle list into menus
                    signalMapper = new QSignalMapper(this);
                    ui->menuSubtitle_Track->clear();
                    ui->menuSubtitle_Track->addAction(ui->action_Add_Subtitle_File);
                    for(auto &track : i.tracks)
                    {
                        if(track.type == "sub")
                        {
                            QAction *action = ui->menuSubtitle_Track->addAction(QString::number(track.id)+": "+track.title+" ("+track.lang+")");
                            signalMapper->setMapping(action, track.id);
                            connect(action, SIGNAL(triggered()),
                                    signalMapper, SLOT(map()));
                        }
                        else if(track.type == "video" && // video track
                                track.albumart)          // is album art
                        {
                            ui->action_Hide_Album_Art_2->setEnabled(true);
                        }
                    }
                    if(ui->menuSubtitle_Track->actions().count() == 0)
                    {
                        ui->menuSubtitle_Track->setEnabled(false);
                        ui->menuFont_Si_ze->setEnabled(false);
                    }
                    else
                    {
                        ui->menuSubtitle_Track->setEnabled(true);
                        ui->menuFont_Si_ze->setEnabled(true);
                    }
                    connect(signalMapper, SIGNAL(mapped(int)),
                            mpv, SLOT(SetSid(int)));

                    setWindowTitle(i.media_title);
                    ui->seekBar->setTracking(i.length);

                    if(!remaining)
                        ui->remainingLabel->setText(FormatTime(i.length));
                }
            });

    connect(mpv, &MpvHandler::playStateChanged,
            [=](Mpv::PlayState playState)
            {
                switch(playState)
                {
                case Mpv::Loaded: // todo: show the user we are loading their file
                    break;

                case Mpv::Started: // todo: fix initial load with fitWindow
                    mpv->LoadFileInfo();
                    if(!init) // will only happen the first time a file is loaded.
                    {
                        ui->action_Play->setEnabled(true);
                        ui->playButton->setEnabled(true);
                        init = true;
                    }
                    if(getAutoFit())
                        FitWindow(getAutoFit());
                    SetPlaybackControls(true);
                    mpv->Play();
                case Mpv::Playing:
                    ui->playButton->setIcon(QIcon(":/img/default_pause.svg"));
                    ui->action_Play->setText("&Pause");
            #ifdef Q_OS_WIN
                    if(onTop == "playing")
                        SetAlwaysOnTop(true);
            #endif
                    break;

                case Mpv::Paused:
                case Mpv::Stopped:
                    ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
                    ui->action_Play->setText("&Play");
            #ifdef Q_OS_WIN
                    if(ui->actionWhen_Playing->isChecked())
                        SetAlwaysOnTop(false);
            #endif
                    break;

                case Mpv::Idle:
                    if(init && (mpv->getIndex() > mpv->getMaxIndex() ||
                                ui->actionStop_after_Current->isChecked()))
                    {
                        setWindowTitle("Baka MPlayer");
                        SetPlaybackControls(false);
                        ui->seekBar->setTracking(0);
                        ui->actionStop_after_Current->setChecked(false);
                    }
                    break;

                case Mpv::Ended:
                    break;
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

    connect(mpv, &MpvHandler::lastFileChanged,
            [=](QString f)
            {
                ui->actionOpen_Last_File->setEnabled(f != "");
            });

    connect(mpv, &MpvHandler::indexChanged,
            [=](int index)
            {
                ui->playlistWidget->setCurrentRow(index);
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

    connect(mpv, &MpvHandler::searchChanged,
            [=](QString s)
            {
                ui->searchBox->setText(s);
            });

    connect(mpv, &MpvHandler::playlistVisibleChanged,
            [=](bool b)
            {
                SetPlaylist(b);
            });

    connect(mpv, &MpvHandler::playlistChanged,
            [=](const QStringList &list)
            {
                ui->playlistWidget->clear();
                ui->playlistWidget->addItems(list);

                if(list.length() > 1)
                {
                    ui->actionSh_uffle->setEnabled(true);
                    ui->playlistButton->setEnabled(true);
                    ui->action_Playlist->setEnabled(true);
                    ui->splitter->setEnabled(true);
                    ui->actionStop_after_Current->setEnabled(true);
                }
                else
                {
                    ui->actionSh_uffle->setEnabled(false);
                    ui->playlistButton->setEnabled(false);
                    ui->action_Playlist->setEnabled(false);
                    ui->splitter->setEnabled(false);
                    ui->actionStop_after_Current->setEnabled(false);
                }

                if(list.length() > 0)
                    ui->menuR_epeat->setEnabled(true);
                else
                    ui->menuR_epeat->setEnabled(false);
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



    // ui

    connect(ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                mpv->Seek(((double)i/ui->seekBar->maximum())*mpv->getFileInfo().length);
            });

    connect(ui->openButton, &OpenButton::LeftClick,                     // Playback: Open button (left click)
            [=]
            {
                mpv->LoadFile(QFileDialog::getOpenFileName(this, "Open File"));
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
                mpv->LoadFile(LocationDialog::getUrl(mpv->getPath(), this));
            });

    connect(ui->remainingLabel, &CustomLabel::clicked,
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
                if(ui->splitter->position() == ui->splitter->max()) // splitter is right-most (playlist not visible)
                    SetPlaylist(true);
                else
                    SetPlaylist(false);
            });

    connect(ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                if(i == ui->splitter->max()) // right-most, playlist is hidden
                {
                    ui->action_Playlist->setChecked(false);
                    ui->action_Hide_Album_Art_2->setChecked(false);
                }
                else if(i == 0) // left-most, album art is hidden, playlist is visible
                {
                    ui->action_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art_2->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    ui->action_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art_2->setChecked(false);
                }
            });

    connect(ui->searchBox, &QLineEdit::textChanged,                     // Playlist: Search box
            [=](QString s)
            {
                mpv->Search(s);
            });

    connect(ui->indexLabel, &CustomLabel::clicked,                      // Playlist: Clicked the indexLabel
            [=]
            {
                QString res = InputDialog::getInput("Enter the file number you want to play:\nNote: Value must be between 1 - "+QString::number(mpv->getMaxIndex()),
                                                    "Enter File Number",
                                                    [this](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if(in >= 1 && in <= mpv->getMaxIndex())
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
                    ui->indexLabel->setText("File - of "+QString::number(ui->playlistWidget->count()));
                else
                    ui->indexLabel->setText("File "+QString::number(i+1)+" of "+QString::number(ui->playlistWidget->count()));
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
                mpv->ShowAll(b);
            });

    connect(ui->refreshButton, &QPushButton::clicked,                   // Playlist: Refresh playlist button
            [=]
            {
                mpv->Refresh();
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
                mpv->LoadFile(QFileDialog::getOpenFileName(this, "Open File"));
            });

    connect(ui->actionOpen_URL, &QAction::triggered,                    // File -> Open URL
            [=]
            {
                mpv->LoadFile(LocationDialog::getUrl(mpv->getPath(), this));
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
                mpv->SetSubs(b);
            });

    connect(ui->action_Add_Subtitle_File, &QAction::triggered,          //  View -> Subtitle Track -> Add Subtitle File...
            [=]
            {
                QString trackFile = QFileDialog::getOpenFileName(this, "Open Subtitle File", mpv->getPath(), "*.sub *.srt *.ass *.ssa"); // todo: add more formats
                if(trackFile != "")
                    mpv->AddSub(trackFile);
                // todo: add track to tracklist
                // todo: select this track
            });
                                                                        // View -> Font Size ->
    connect(ui->actionS_ize, &QAction::triggered,                       // View -> Font Size -> Size +
            [=]
            {
                mpv->AddSubScale(.02);
            });

    connect(ui->action_Size, &QAction::triggered,                       // View -> Font Size -> Size -
            [=]
            {
                mpv->AddSubScale(-.02);
            });

    connect(ui->action_Reset_Size, &QAction::triggered,                 // View -> Font Size -> Reset Size
            [=]
            {
                mpv->SetSubScale(1);
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
                mpv->Shuffle(b);
            });
    // todo: repeat menu
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
                SetPlaylist(b);
            });

    connect(ui->action_Hide_Album_Art_2, &QAction::triggered,           // Settings -> Hide Album Art
            [=](bool b)
            {
                if(b)
                {
                    if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
                        ui->splitter->setNormalPosition(ui->splitter->position()); // save splitter position as the normal position
                    ui->splitter->setPosition(0); // bring the splitter position to the left-most
                }
                else
                    ui->splitter->setPosition(ui->splitter->normalPosition()); // bring the splitter to normal position
            });

    connect(ui->action_Dim_Lights_2, &QAction::triggered,               // Settings -> Dim Lights
            [=](bool b)
            {
                DimLights(b);
            });

    connect(ui->actionShow_D_ebug_Output, &QAction::triggered,          // Settings -> Show Debug Output
            [=](bool b)
            {
                mpv->Debug(b);
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
                QDesktopServices::openUrl(QUrl("http://bakamplayer.u8sand.net/help"));
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

    connect(qApp, &QApplication::focusWindowChanged,
            [=](QWindow *focusWindow)
            {
                // note: focusWindow will be 0 if anything is clicked outside of our program which is useful
                // the only other problem is that when dragging by the top handle
                // it will be 0 resulting in lights going off, this is a side effect
                // which will have to stay for now.
                if(focusWindow == 0 && light->isVisible())
                {
                    light->setVisible(false); // remove dimlights
                    ui->action_Dim_Lights_2->setChecked(false); // uncheck dimlights
                }
            });

    // keyboard shortcuts

    QAction *shortcut;

    shortcut = new QAction(this);
    shortcut->setShortcut(QKeySequence("Right"));
    connect(shortcut, &QAction::triggered,
            [=]
            {
                mpv->Seek(5, true);
            });
    addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(QKeySequence("Left"));
    connect(shortcut, &QAction::triggered,
            [=]
            {
                mpv->Seek(-5, true);
            });
    addAction(shortcut);

    shortcut = new QAction(this);
    shortcut->setShortcut(QKeySequence("Esc"));
    connect(shortcut, &QAction::triggered,
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
    addAction(shortcut);

    LoadSettings();
    SetPlaylist(false);
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
    // window
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    QSize(settings->value("window/width", 600).toInt(),
                                          settings->value("window/height", 430).toInt()),
                                    qApp->desktop()->availableGeometry()));
    setOnTop(settings->value("window/onTop", "never").toString());
    setAutoFit(settings->value("window/autoFit", 100).toInt());
    setTrayIcon(settings->value("window/trayIcon", false).toBool());
    setHidePopup(settings->value("window/hidePopup", false).toBool());
    setRemaining(settings->value("window/remaining", true).toBool());
    ui->splitter->setNormalPosition(settings->value("window/splitter",(int)(ui->splitter->max()*3.0/4.0)).toInt());
    // mpv
    mpv->setLastFile(settings->value("mpv/lastFile", "").toString());
    mpv->setShowAll(settings->value("mpv/showAll", false).toBool());
    mpv->setScreenshotFormat(settings->value("mpv/screenshotFormat", "png").toString());
    mpv->setScreenshotTemplate(settings->value("mpv/screenshotTemplate", "screenshot%#04n").toString());
    mpv->setScreenshotDir(settings->value("mpv/screenshotDir", "").toString());
    mpv->setSpeed(settings->value("mpv/speed", 1.0).toDouble());
    mpv->setVolume(settings->value("mpv/volume", 100).toInt());
    // common
    setDebug(settings->value("common/debug", false).toBool());
}

void MainWindow::SaveSettings()
{
    // window
    settings->setValue("window/width", normalGeometry().width());
    settings->setValue("window/height", normalGeometry().height());
    settings->setValue("window/onTop", getOnTop());
    settings->setValue("window/autoFit", getAutoFit());
    settings->setValue("window/trayIcon", getTrayIcon());
    settings->setValue("window/hidePopup", getHidePopup());
    settings->setValue("window/remaining", getRemaining());
    settings->setValue("window/splitter", ui->splitter->position() == ui->splitter->max() ?
                                            ui->splitter->normalPosition() :
                                            ui->splitter->position());
    // mpv
    settings->setValue("mpv/lastFile", mpv->getLastFile());
    settings->setValue("mpv/showAll", mpv->getShowAll());
    settings->setValue("mpv/screenshotFormat", mpv->getScreenshotFormat());
    settings->setValue("mpv/screenshotTemplate", mpv->getScreenshotTemplate());
    settings->setValue("mpv/screenshotDir", mpv->getScreenshotDir());
    settings->setValue("mpv/speed", mpv->getSpeed());
    settings->setValue("mpv/volume", mpv->getVolume());
    // common
    settings->setValue("debug/output", getDebug());
}

void MainWindow::Load(QString file)
{
    mpv->LoadFile(file);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) // todo: does this even work??
{
    if(event->mimeData()->hasText() || event->mimeData()->hasUrls()) // plain text / url
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) // todo: does this even work??
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasText()) // plain text
        mpv->LoadFile(mimeData->text()); // load the text as a file
    else if(mimeData->hasUrls()) // urls
        for(auto &url : mimeData->urls())
            mpv->LoadFile(url.path()); // load the urls as files
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(!isFullScreen())
    {
        dragging = true;
        lastMousePos = event->pos();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(!isFullScreen())
        dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // note: this will work completely when mpv stops steeling focus
    // todo: fix mouse tracking when not clicking
    static QRect playbackRect;
    if(dragging)
    {
        move(pos()+event->pos()-lastMousePos);
        event->accept();
    }
    else if(!ui->playbackLayoutWidget->isVisible() &&
            playbackRect.contains(event->pos()))
    {
        ui->playbackLayoutWidget->setVisible(true);
        ui->seekBar->setVisible(true);
    }
    else if(isFullScreen() &&
            ui->playbackLayoutWidget->isVisible() &&
            !ui->playbackLayoutWidget->geometry().contains(event->pos()))
    {
        playbackRect = ui->playbackLayoutWidget->geometry();
        ui->playbackLayoutWidget->setVisible(false);
        ui->seekBar->setVisible(false);
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
    if(ui->mpvFrame->geometry().contains(event->pos())) // if mouse is in the mpvFrame
    {
        FullScreen(!isFullScreen());
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
    ui->action_Frame_Step->setEnabled(enable);
    ui->actionFrame_Back_Step->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Full_Screen->setEnabled(enable);
    ui->menuTake_Screenshot->setEnabled(enable);
    ui->action_Add_Subtitle_File->setEnabled(enable);
    ui->menuFit_Window->setEnabled(enable);
    ui->menuAspect_Ratio->setEnabled(enable);
    if(!enable)
    {
        ui->action_Hide_Album_Art_2->setEnabled(false);
        ui->menuSubtitle_Track->setEnabled(false);
        ui->menuFont_Si_ze->setEnabled(false);
    }

    if(enable && mpv->getFileInfo().chapters.length() > 0) // only enable chapters if there are chapters
    {
        ui->menu_Chapters->setEnabled(true);
        ui->action_Next_Chapter->setEnabled(true);
        ui->action_Previous_Chapter->setEnabled(true);
    }
    else
    {
        ui->menu_Chapters->setEnabled(false);
        ui->action_Next_Chapter->setEnabled(false);
        ui->action_Previous_Chapter->setEnabled(false);
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

void MainWindow::FullScreen(bool fs)
{
    if(fs)
    {
        if(light->isVisible())
        {
            light->setVisible(false);
            ui->action_Dim_Lights_2->setChecked(false);
        }
        setWindowState(windowState() | Qt::WindowFullScreen);
        ui->menubar->setVisible(false);
        SetPlaylist(false);
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
    }
}

void MainWindow::SetPlaylist(bool visible)
{
    if(!ui->splitter->normalPosition())
        ui->splitter->setPosition(ui->splitter->max()*3.0/4);
    if(visible)
        ui->splitter->setPosition(ui->splitter->normalPosition()); // bring splitter position to normal
    else
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save splitter position as the normal position
        ui->splitter->setPosition(ui->splitter->max()); // set splitter position to right-most
    }
}

void MainWindow::FitWindow(int percent)
{
    if(isFullScreen())
        return;
    if(isMaximized())
        setWindowFlags(windowFlags() & ~Qt::WindowMaximized);

    // todo: refresh video_params
    //mpv->LoadVideoParams();
    const Mpv::VideoParams &params = mpv->getFileInfo().video_params;
    QRect fG = ui->mpvFrame->geometry(), // frame geometry
          cG = geometry(), // current geometry of window
          dG = qApp->desktop()->availableGeometry(); // desktop geometry
    int w, h;
    double a;

    // get aspect ratio
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
    mpv->SetAspect(aspect);
}

void MainWindow::DimLights(bool dim)
{
    if(dim)
        light->show();
    else
        light->close();
    activateWindow();
    raise();
    setFocus();
}

bool MainWindow::SetScreenshotDir()
{
    QMessageBox::information(this, "Take Screenshot",
                             "Choose the default location where you would like to save your screenshots. Also by default, we will save your screenshots as a jpg file. If you'd like to change any of these settings, it is under Preferences.");
    QString dir = QFileDialog::getExistingDirectory(this, "Screenshot Directory");
    if(dir != "")
    {
        mpv->setScreenshotDir(dir);
        return true;
    }
    return false;
}


#ifdef Q_OS_WIN
void MainWindow::SetAlwaysOnTop(bool ontop)
{
    // maybe in the future, Linux X specific code that way we could enable it for both platforms
#if defined(Q_OS_WIN)
    SetWindowPos((HWND)winId(),
                 ontop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
//#elif defined(Q_OS_LINUX) // though this code should work, I'm not sure how to implement it yet
//    XClientMessageEvent xclient;
//    memset(&xclient, 0, sizeof(xclient));
//    xclient.type = ClientMessage;
//    xclient.window = window;
//    xclient.message_type = "_NET_WM_STATE";
//    xclient.format = 32;
//    xclient.data.l[0] = ontop? 1 : 0;
//    xclient.data.l[1] = "_NET_WM_STATE_ABOVE"; // _NET_WM_STATE_BELOW
//    xclient.data.l[2] = 0;
//    xclient.data.l[3] = 0;
//    xclient.data.l[4] = 0;
//    XSendEvent (window, NULL, false,
//        SubstructureRedirectMask | SubstructureNotifyMask,
//        (XEvent*)&xclient);
#else // qt code
    if(ontop)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    show();
#endif
}

void MainWindow::AlwaysOnTop(bool ontop)
{
    ui->actionWhen_Playing->setChecked(false);
    ui->action_Never->setChecked(false);
    SetAlwaysOnTop(ontop);
}

void MainWindow::AlwaysOnTopWhenPlaying(bool ontop)
{
    if(ontop)
    {
        ui->action_Always->setChecked(false);
        ui->action_Never->setChecked(false);
        if(mpv->GetPlayState() == Mpv::Playing)
            SetAlwaysOnTop(true);
    }
    else
        SetAlwaysOnTop(false);
}

void MainWindow::NeverOnTop(bool ontop)
{
    if(ontop)
    {
        ui->actionWhen_Playing->setChecked(false);
        ui->action_Always->setChecked(false);
        SetAlwaysOnTop(false);
    }
}
#endif
