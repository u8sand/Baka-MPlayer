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
#include <QIcon>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "aboutdialog.h"
#include "infodialog.h"
#include "locationdialog.h"
#include "jumpdialog.h"
#include "inputdialog.h"

MainWindow::MainWindow(QSettings *_settings, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(_settings),
    dragging(false),
    lastMousePos(QPoint())
{
    ui->setupUi(this);

    // note: trayIcon does not work in my environment--known qt bug
    // see: https://bugreports.qt-project.org/browse/QTBUG-34364
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(qApp->windowIcon());
    // todo: tray menu/tooltip
    SetPlaylist(false);

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) // linux has native support for on top already, this feature is unnecessary
    ui->menu_Options->removeAction(ui->menu_On_Top->menuAction()); // remove the On Top menu
#endif

    // load settings
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    QSize(settings->value("window/width", 600).toInt(),
                                          settings->value("window/height", 430).toInt()),
                                    qApp->desktop()->availableGeometry()));
    ui->showAllButton->setChecked(settings->value("playlist/show-all", false).toBool());
    ui->volumeSlider->setValue(settings->value("mpv/volume", 100).toInt());
    QFile f(settings->value("last-file", "").toString());
    ui->actionOpen_Last_File->setEnabled(f.exists());
    ui->actionShow_D_ebug_Output->setChecked(settings->value("debug/output", false).toBool());
    ui->outputTextEdit->setVisible(settings->value("debug/output", false).toBool());

    // initialize managers/handlers
    mpv = new MpvHandler(settings, ui->mpvFrame->winId(), this);
    playlist = new PlaylistManager(settings, this);
    light = new LightDialog();

    // setup signals & slots
                                                                        // mpv updates
    connect(mpv, SIGNAL(TimeChanged(int)),                              // MPV_EVENT time-pos update
            this, SLOT(SetTime(int)));                                  // adjust time and slider accordingly
    connect(mpv, SIGNAL(PlayStateChanged(Mpv::PlayState)),              // MPV_EVENT playstate changes
            this, SLOT(SetPlayState(Mpv::PlayState)));                  // adjust interface based on new play-state
    connect(mpv, SIGNAL(VolumeChanged(int)),                            // MPV_EVENT volume update
            ui->volumeSlider, SLOT(setValueNoSignal(int)));             // update the volume slider without triggering it's value changed signal
    connect(mpv, SIGNAL(ErrorSignal(QString)),                          // mpv error occured
            this, SLOT(HandleError(QString)));                          // output error message
    connect(mpv, SIGNAL(DebugSignal(QString)),                          // mpv debug message received
            this, SLOT(Debug(QString)));                                // output to outputTextEdit
                                                                        // playlist
    connect(playlist, SIGNAL(Play(QString)),                            // playlist play signal
            mpv, SLOT(OpenFile(QString)), Qt::QueuedConnection);        // mpv open file
    connect(playlist, SIGNAL(Stop()),                                   // playlist stop signal
            mpv, SLOT(Stop()), Qt::QueuedConnection);                   // mpv stop
    connect(playlist, SIGNAL(Search(QString)),                          // playlist search
            ui->searchBox, SLOT(setText(QString)));                     // set the text of the searchbox
    connect(playlist, SIGNAL(Show(bool)),                               // playlist set visibility
            this, SLOT(SetPlaylist(bool)));                             // set visibility of the playlist
    connect(playlist, SIGNAL(ListChanged(QStringList)),                 // playlist update list
            this, SLOT(UpdatePlaylist(QStringList)));                   // update the playlistWidget
    connect(playlist, SIGNAL(IndexChanged(int)),                        // playlist update index
            this, SLOT(UpdatePlaylistIndex(int)));                      // update the playlistWidget selection
    connect(playlist, SIGNAL(ShuffleChanged(bool)),                     // playlist shuffle changed
            ui->actionSh_uffle, SLOT(setChecked(bool)));                // update the menu item
    connect(ui->indexLabel, SIGNAL(clicked()),                          // when indexLabel is clicked
            this, SLOT(GetPlaylistIndex()));                            // get the user input for the playlist index
    connect(ui->refreshButton, SIGNAL(clicked()),                       // refresh button
            playlist, SLOT(Refresh()));                                 // refresh playlist files
    connect(ui->actionSh_uffle, SIGNAL(triggered(bool)),                // shuffle action
            playlist, SLOT(Shuffle(bool)));                             // shuffle playlist
    connect(ui->showAllButton, SIGNAL(clicked(bool)),                   // show all button
            playlist, SLOT(ShowAll(bool)));                             // show all types file types in playlist
    connect(ui->playlistWidget, SIGNAL(doubleClicked(QModelIndex)),     // playlist selection
            this, SLOT(PlayIndex(QModelIndex)));                        // play the selected file
    connect(ui->currentFileButton, SIGNAL(clicked()),                   // current file button
            this, SLOT(PlaylistSelectCurrent()));                       // selects the current file in the playlist
    connect(ui->playlistWidget, SIGNAL(currentRowChanged(int)),         // playlist selection changed
            this, SLOT(UpdatePlaylistSelectionIndex(int)));             // update the indexLabel message
    connect(ui->searchBox, SIGNAL(textChanged(QString)),                // playlist search
            playlist, SLOT(SearchPlaylist(QString)));                   // narrow down playlist
                                                                        // sliders
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)),                // volume slider changed
            mpv, SLOT(AdjustVolume(int)));                              // adjust volume accordingly
    connect(ui->seekBar, SIGNAL(valueChanged(int)),                     // seek bar slider changed
            this, SLOT(Seek(int)));                                     // seek accordingly
                                                                        // buttons
    connect(ui->openButton, SIGNAL(LeftClick()),                        // left-clicked the open button
            this, SLOT(OpenFile()));                                    // open-file dialog
    connect(ui->openButton, SIGNAL(MiddleClick()),                      // middle-clicked the open button
            this, SLOT(JumpToTime()));                                  // jump-to-time dialog
    connect(ui->openButton, SIGNAL(RightClick()),                       // right-clicked the open button
            this, SLOT(OpenUrl()));                                     // open-url dialog
    connect(ui->playButton, SIGNAL(clicked()),                          // clicked the playpause button
            this, SLOT(PlayPause()));                                   // mpv playpause
    connect(ui->playlistButton, SIGNAL(clicked()),                      // clicked the playlist button
            this, SLOT(TogglePlaylist()));                              // toggle playlist visibility
    connect(ui->previousButton, SIGNAL(clicked()),                      // clicked the previous button
            playlist, SLOT(Previous()));                                // play the previous entry in the playlist
    connect(ui->nextButton, SIGNAL(clicked()),                          // clicked the next button
            playlist, SLOT(Next()));                                    // play the next entry in the playlist
    connect(ui->rewindButton, SIGNAL(clicked()),                        // clicked the rewind button
            this, SLOT(Rewind()));                                      // rewind the video
                                                                        // menu
                                                                        // File ->
    connect(ui->action_New_Player, SIGNAL(triggered()),                 // File -> New Player
            this, SLOT(NewPlayer()));                                   // create a new MainWindow
    connect(ui->action_Open_File, SIGNAL(triggered()),                  // File -> Open File
            this, SLOT(OpenFile()));                                    // open-file dialog
    connect(ui->actionOpen_URL, SIGNAL(triggered()),                    // File -> Open URL
            this, SLOT(OpenUrl()));                                     // open-url dialog
    connect(ui->actionOpen_Path_from_Clipboard, SIGNAL(triggered()),    // File -> Open Path from Clipboard
            this, SLOT(OpenFileFromClipboard()));                       // open file from clipboard
    connect(ui->actionOpen_Last_File, SIGNAL(triggered()),              // File -> Open Last File
            this, SLOT(OpenLastFile()));                                // reopen the last file opened (saved in settings)
    connect(ui->actionShow_in_Folder, SIGNAL(triggered()),              // File -> Show in Folder
            this, SLOT(ShowInFolder()));                                // open file explorer to file location
    connect(ui->actionPlay_Next_File, SIGNAL(triggered()),              // File -> Play Next File
            playlist, SLOT(Next()));                                    // play the next entry in the playlist
    connect(ui->actionPlay_Previous_File, SIGNAL(triggered()),          // File -> Play Previous File
            playlist, SLOT(Previous()));                                // play the previous entry in the playlist
    connect(ui->actionE_xit_2, SIGNAL(triggered()),                     // File -> Exit
            this, SLOT(close()));                                       // close this window
                                                                        // View ->
    connect(ui->action_Full_Screen, SIGNAL(triggered(bool)),            // View -> Full Screen
            this, SLOT(FullScreen(bool)));                              // full screen window
    connect(ui->actionTake_Snapshot, SIGNAL(triggered()),               // View -> Take Snapshot
            mpv, SLOT(Snapshot()));                                     // mpv snapshot
    QSignalMapper *fitWindowMap = new QSignalMapper(this);              // View -> FitWindow ->
    fitWindowMap->setMapping(ui->action_To_Current_Size, 0);            // View -> FitWindow -> To Current Size
    fitWindowMap->setMapping(ui->action50, .5);                         // View -> FitWindow -> 50%
    fitWindowMap->setMapping(ui->action75, .75);                        // View -> FitWindow -> 75%
    fitWindowMap->setMapping(ui->action100, 1);                         // View -> FitWindow -> 100%
    fitWindowMap->setMapping(ui->action200, 2);                         // View -> FitWindow -> 200%
    connect(ui->action_To_Current_Size, SIGNAL(triggered()),
            fitWindowMap, SLOT(map()));
    connect(ui->action50, SIGNAL(triggered()),
            fitWindowMap, SLOT(map()));
    connect(ui->action75, SIGNAL(triggered()),
            fitWindowMap, SLOT(map()));
    connect(ui->action100, SIGNAL(triggered()),
            fitWindowMap, SLOT(map()));
    connect(ui->action200, SIGNAL(triggered()),
            fitWindowMap, SLOT(map()));
    connect(fitWindowMap, SIGNAL(mapped(int)),
            this, SLOT(FitWindow(int)));                                // fit the window to the appropriate percentage
    QSignalMapper *aspectMap = new QSignalMapper(this);                 // View -> Aspect Ratio ->
    aspectMap->setMapping(ui->action_Autodetect, 0);                    // View -> Aspect Ratio -> Auto Detect
    aspectMap->setMapping(ui->actionForce_4_3, 4/3);                    // View -> Aspect Ratio -> 4:3
    aspectMap->setMapping(ui->actionForce_2_35_1, 2.35/1);              // View -> Aspect Ratio -> 2.35:1
    aspectMap->setMapping(ui->actionForce_16_9, 16/9);                  // View -> Aspect Ratio -> 16:9
    connect(ui->action_Autodetect, SIGNAL(triggered()),
            aspectMap, SLOT(map()));
    connect(ui->actionForce_4_3, SIGNAL(triggered()),
            aspectMap, SLOT(map()));
    connect(ui->actionForce_2_35_1, SIGNAL(triggered()),
            aspectMap, SLOT(map()));
    connect(ui->actionForce_16_9, SIGNAL(triggered()),
            aspectMap, SLOT(map()));
    connect(aspectMap, SIGNAL(mapped(QString)),
            this, SLOT(SetAspectRatio(QString)));                       // set the aspect ratio to the appropriate ratio
    connect(ui->actionShow_Subtitles, SIGNAL(triggered()),              // View -> Show Subtitles
            mpv, SLOT(ToggleSubs()));                                   // mpv show subs
    connect(ui->action_Add_Subtitle_File, SIGNAL(triggered()),          // add a new subtitle file
            this, SLOT(AddSubtitleTrack()));                            // get the file and add
                                                                        // subtitle track menu
                                                                        // View -> Font Size ->
    connect(ui->actionS_ize, SIGNAL(triggered()),                       // View -> Font Size -> Size +
            this, SLOT(IncreaseFontSize()));                            // increase the font size
    connect(ui->action_Size, SIGNAL(triggered()),                       // View -> Font Size -> Size -
            this, SLOT(DecreaseFontSize()));                            // decrease the font size
    connect(ui->action_Reset_Size, SIGNAL(triggered()),                 // View -> Font Size -> Reset Size
            this, SLOT(ResetFontSize()));                               // reset the font size

    connect(ui->actionMedia_Info, SIGNAL(triggered()),                  // View -> Media Info
            this, SLOT(MediaInfo()));                                   // show media info dialog
                                                                        // Playback ->
    connect(ui->action_Play, SIGNAL(triggered()),                       // Playback -> (Play|Pause)
            mpv, SLOT(PlayPause()));                                    // mpv playpause
    connect(ui->action_Stop, SIGNAL(triggered()),                       // Playback -> Stop
            mpv, SLOT(Stop()));                                         // mpv stop
    connect(ui->action_Restart, SIGNAL(triggered()),                    // Playback -> Restart
            mpv, SLOT(Restart()));                                      // mpv restart
    connect(ui->actionSh_uffle, SIGNAL(triggered(bool)),                // Playback -> Shuffle
            playlist, SLOT(Shuffle(bool)));                             // playlist shuffle
    // todo: repeat menu
    connect(ui->action_Increase_Volume, SIGNAL(triggered()),            // Playback -> Increase Volume
            this, SLOT(IncreaseVolume()));                              // increase mpv volume by 5
    connect(ui->action_Decrease_Volume, SIGNAL(triggered()),            // Playback -> Decrease Volume
            this, SLOT(DecreaseVolume()));                              // decrease mpv volume by 5
                                                                        // Navigate ->
    connect(ui->action_Next_Chapter, SIGNAL(triggered()),               // Navigate -> Next Chapter
            mpv, SLOT(NextChapter()));                                  // mpv next chapter
    connect(ui->action_Previous_Chapter, SIGNAL(triggered()),           // Navigate -> Previous Chapter
            mpv, SLOT(PreviousChapter()));                              // mpv previous chapter
                                                                        // chapters menu
    connect(ui->action_Frame_Step, SIGNAL(triggered()),                 // Navigate -> Frame Step
            mpv, SLOT(FrameStep()));                                    // mpv frame step
    connect(ui->actionFrame_Back_Step, SIGNAL(triggered()),             // Navigate -> Frame Back Step
            mpv, SLOT(FrameBackStep()));                                // mpv frame back step
    connect(ui->action_Jump_to_Time, SIGNAL(triggered()),               // Navigate -> Jump to Time
            this, SLOT(JumpToTime()));                                  // jump-to-time dialog
                                                                        // Options ->
    connect(ui->action_Show_Playlist_2, SIGNAL(triggered(bool)),        // Options -> Show Playlist
            this, SLOT(SetPlaylist(bool)));                             // toggle playlist visibility
    connect(ui->action_Hide_Album_Art_2, SIGNAL(triggered(bool)),       // Options -> Hide Album Art
            this, SLOT(ShowAlbumArt(bool)));                            // toggle album art
    connect(ui->action_Dim_Lights_2, SIGNAL(triggered(bool)),           // Options -> Dim Lights
            this, SLOT(DimLights(bool)));                               // toggle dim lights
    connect(ui->actionShow_D_ebug_Output, SIGNAL(triggered(bool)),      // Options -> Show Debug Output
            ui->outputTextEdit, SLOT(setVisible(bool)));                // toggle debug output
#ifdef Q_OS_WIN
                                                                        // Options -> On Top ->
    connect(ui->action_Always, SIGNAL(triggered(bool)),                 // Options -> On Top -> Always
            this, SLOT(AlwaysOnTop(bool)));                             // enable/disable
    connect(ui->actionWhen_Playing, SIGNAL(triggered(bool)),            // Options -> On Top -> When Playing
            this, SLOT(AlwaysOnTopWhenPlaying(bool)));                  // if playing, enable
    connect(ui->action_Never, SIGNAL(triggered(bool)),                  // Options -> On Top -> Never
            this, SLOT(NeverOnTop(bool)));                              // disable always/when playing
#endif
                                                                        // Options -> Tray Icon ->
    connect(ui->action_Show_in_Tray, SIGNAL(triggered(bool)),           // Options -> Tray Icon -> Show In Tray
            this, SLOT(ShowInTray(bool)));
    connect(ui->action_Hide_Popup, SIGNAL(triggered(bool)),             // Options -> Tray Icon -> Hide Popup
            this, SLOT(HidePopup(bool)));
                                                                        // Help ->
    connect(ui->actionOnline_Help, SIGNAL(triggered()),                 // Help -> Online Help
            this, SLOT(OnlineHelp()));                                  // open online help
    connect(ui->action_Check_for_Updates, SIGNAL(triggered()),          // Help -> Check for Updates
            this, SLOT(CheckForUpdates()));                             // check for updates
    connect(ui->actionAbout_Qt, SIGNAL(triggered()),                    // Help -> About Qt
            this, SLOT(AboutQt()));                                     // show about qt
    connect(ui->actionAbout_Baka_MPlayer, SIGNAL(triggered()),          // Help -> About Baka MPlayer
            this, SLOT(About()));                                       // show about dialog

    // keyboard shortcuts
    new QShortcut(QKeySequence("Right"), this, SLOT(SeekForward()));
    new QShortcut(QKeySequence("Left"), this, SLOT(SeekBack()));
    new QShortcut(QKeySequence("Esc"), this, SLOT(BossMode()));

    // load arguments (we'll just load the first argument for now as a file)
    QStringList args = QCoreApplication::arguments();
    QStringList::iterator arg = args.begin();
    if(++arg != args.end()) // does the next argument exist?
        playlist->LoadFile(*arg); // loadfile
}

MainWindow::~MainWindow()
{
    // save settings
    settings->setValue("mpv/volume", mpv->GetVolume());
    settings->setValue("window/width", normalGeometry().width());
    settings->setValue("window/height", normalGeometry().height());
    settings->setValue("playlist/show-all", ui->showAllButton->isChecked());
    settings->setValue("debug/output", ui->actionShow_D_ebug_Output->isChecked());

    // cleanup
    delete playlist;
    delete mpv;
    delete ui;
}

void MainWindow::HandleError(QString err)
{
    QMessageBox::warning(this, "Error", err);
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
        playlist->LoadFile(mimeData->text()); // load the text as a file
    else if(mimeData->hasUrls()) // urls
        for(auto &url : mimeData->urls())
            playlist->LoadFile(url.path()); // load the urls as files
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    dragging = true;
    lastMousePos = event->pos();
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
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

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    FullScreen(!isFullScreen());
    event->accept();
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::SetPlaybackControls(bool enable)
{
    // playback controls
    ui->seekBar->setEnabled(enable);
    ui->rewindButton->setEnabled(enable);
    ui->playlistButton->setEnabled(enable);
    // next file
    if(enable && playlist->GetIndex()+1 < ui->playlistWidget->count()) // not the last entry
    {
        ui->nextButton->setEnabled(true);
        ui->nextButton->setIndex(playlist->GetIndex()+2); // starting at 1 instead of at 0 like actual index
        ui->actionPlay_Next_File->setEnabled(true);
    }
    else
    {
        ui->nextButton->setEnabled(false);
        ui->actionPlay_Next_File->setEnabled(false);
    }
    // previous file
    if(enable && playlist->GetIndex()-1 >= 0) // not the first entry
    {
        ui->previousButton->setEnabled(true);
        ui->previousButton->setIndex(-playlist->GetIndex()); // we use a negative index value for the left button
        ui->actionPlay_Previous_File->setEnabled(true);
    }
    else
    {
        ui->previousButton->setEnabled(false);
        ui->actionPlay_Previous_File->setEnabled(false);
    }
    // menubar
    ui->action_Play->setEnabled(enable);
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Playlist->setEnabled(enable);
    ui->action_Full_Screen->setEnabled(enable);
    ui->actionTake_Snapshot->setEnabled(enable);
    ui->action_Add_Subtitle_File->setEnabled(enable);
    ui->menuFit_Window->setEnabled(enable);
    ui->menuAspect_Ratio->setEnabled(enable);
    ui->menuSubtitle_Track->setEnabled(enable);
    if(!enable)
        ui->action_Hide_Album_Art_2->setEnabled(false);

    if(enable && mpv->GetFileInfo().chapters.length() > 0) // only enable chapters if there are chapters
    {
        ui->action_Next_Chapter->setEnabled(true);
        ui->action_Previous_Chapter->setEnabled(true);
    }
    else
    {
        ui->action_Next_Chapter->setEnabled(false);
        ui->action_Previous_Chapter->setEnabled(false);
    }
}

QString MainWindow::FormatTime(int _time)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(mpv->GetFileInfo().length >= 3600) // hours
        return time.toString("h:mm:ss");
    if(mpv->GetFileInfo().length >= 60)   // minutes
        return time.toString("mm:ss");
    return time.toString("0:ss");   // seconds
}

void MainWindow::SetTime(int time)
{
    // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
    // the formula is a simple ratio seekBar's max * time/totalTime
    ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)time/mpv->GetFileInfo().length));

    // set duration and remaining labels, QDateTime takes care of formatting for us
    ui->durationLabel->setText(FormatTime(time));
    ui->remainingLabel->setText("-"+FormatTime(mpv->GetFileInfo().length-time));

    // update next/previous chapter's enabled status
    if(mpv->GetFileInfo().chapters.length() > 0)
    {
        ui->action_Next_Chapter->setEnabled(time < mpv->GetFileInfo().chapters.last().time);
        ui->action_Previous_Chapter->setEnabled(time > mpv->GetFileInfo().chapters.first().time);
    }
}

void MainWindow::SetPlayState(Mpv::PlayState playState)
{
    // triggered when mpv playstate is changed so we can update controls accordingly
    switch(playState)
    {
    case Mpv::Loaded: // todo: show the user we are loading their file
        break;
    case Mpv::Started:
    {
        const Mpv::FileInfo &fi = mpv->GetFileInfo();
        // load chapter list into menus
        QList<int> ticks;
        QSignalMapper *signalMapper = new QSignalMapper(this);
        int n = 1;
        ui->menu_Chapters->clear();
        for(auto &ch : fi.chapters)
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
        for(auto &track : fi.tracks)
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
            ui->menuSubtitle_Track->addAction("[ none ]")->setEnabled(false);
        connect(signalMapper, SIGNAL(mapped(int)),
                mpv, SLOT(SetSid(int)));


        setWindowTitle(fi.media_title);
        ui->seekBar->setTracking(fi.length);
        if(!ui->playButton->isEnabled()) // will only happen the first time a file is loaded.
        {
            ui->playButton->setEnabled(true);
            ui->playButton->Update();
        }
        mpv->Play();
        SetPlaybackControls(true);
    }
    case Mpv::Playing:
        ui->playButton->SetPlay(false);
        ui->action_Play->setText("&Pause");
#ifdef Q_OS_WIN
        if(ui->actionWhen_Playing->isChecked())
            SetAlwaysOnTop(true);
#endif
        break;
    case Mpv::Paused:
    case Mpv::Stopped:
        ui->playButton->SetPlay(true);
        ui->action_Play->setText("&Play");
#ifdef Q_OS_WIN
        if(ui->actionWhen_Playing->isChecked())
            SetAlwaysOnTop(false);
#endif
        break;
    case Mpv::Idle:
        if(ui->actionStop_after_Current->isChecked() || !playlist->Next())
        {
            setWindowTitle("Baka MPlayer");
            SetPlaybackControls(false);
            ui->seekBar->setTracking(0);
            SetTime(0);
        }
        break;
    case Mpv::Ended:
        settings->setValue("last-file", mpv->GetFileInfo().media_title);
        ui->actionOpen_Last_File->setEnabled(settings->value("last-file").toString()!="");
        break;
    }
}

void MainWindow::PlayPause()
{
    if(mpv->GetPlayState() == Mpv::Idle) // if idle, play plays the selected playlist file
        playlist->PlayIndex(ui->playlistWidget->currentRow());
    else
        mpv->PlayPause();
}

void MainWindow::Seek(int position)
{
    mpv->Seek(((double)position/ui->seekBar->maximum())*mpv->GetFileInfo().length);
}

void MainWindow::Rewind()
{
    // if user presses rewind button twice within 3 seconds, stop video
    if(mpv->GetTime() < 3)
        mpv->Stop();
    else
    {
        if(mpv->GetPlayState() == Mpv::Playing)
            mpv->Restart();
        else
            mpv->Stop();
    }
}

void MainWindow::NewPlayer()
{
    (new MainWindow(settings))->show();
}

void MainWindow::OpenFile()
{
    playlist->LoadFile(QFileDialog::getOpenFileName(this, "Open file"));
}

void MainWindow::OpenUrl()
{
    playlist->LoadFile(LocationDialog::getUrl(this));
}

void MainWindow::FullScreen(bool fs)
{
    if(fs)
    {
        setWindowState(windowState() | Qt::WindowFullScreen);
        ui->menubar->setVisible(false);
        SetPlaylist(false);
        setMouseTracking(true); // register mouse move event
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

void MainWindow::BossMode()
{
    mpv->Pause();
    setWindowState(windowState() | Qt::WindowMinimized);
}

void MainWindow::JumpToTime()
{
    int time = JumpDialog::getTime(mpv->GetFileInfo().length,this);
    if(time >= 0)
        mpv->Seek(time);
}

void MainWindow::MediaInfo() // todo
{
    InfoDialog::info(mpv->GetFileInfo(), this);
}

void MainWindow::OpenFileFromClipboard()
{
    playlist->LoadFile(QApplication::clipboard()->text());
}

void MainWindow::OpenLastFile()
{
    playlist->LoadFile(settings->value("last-file", "").toString());
}

void MainWindow::ShowInFolder()
{
    QDesktopServices::openUrl("file:///"+QDir::toNativeSeparators(playlist->GetPath()));
}

void MainWindow::UpdatePlaylist(QStringList list)
{
    ui->playlistWidget->clear();
    ui->playlistWidget->addItems(list);
}

void MainWindow::UpdatePlaylistIndex(int index)
{
    ui->playlistWidget->setCurrentRow(index);
}

void MainWindow::UpdatePlaylistSelectionIndex(int index)
{
    if(index == -1) // no selection
        ui->indexLabel->setText("File - of "+QString::number(ui->playlistWidget->count()));
    else
        ui->indexLabel->setText("File "+QString::number(index+1)+" of "+QString::number(ui->playlistWidget->count()));
}

void MainWindow::PlaylistSelectCurrent()
{
    ui->playlistWidget->setCurrentRow(playlist->GetIndex());
}

void MainWindow::PlayIndex(QModelIndex index)
{
    playlist->PlayIndex(index.row());
}

void MainWindow::SetPlaylist(bool visible)
{
    static QList<int> sizes;
    if(sizes.length() == 0) // initialize sizes
        sizes = {2, 1};     // todo: use better sizes

    if(!visible)
    {
        if(ui->splitter->sizes()[1] > 0)
            sizes = ui->splitter->sizes();
        ui->splitter->setSizes({sizes[0]+sizes[1], 0});
    }
    else
        ui->splitter->setSizes(sizes);

    ui->action_Playlist->setChecked(visible);
}

void MainWindow::TogglePlaylist()
{
    if(ui->splitter->sizes()[1] == 0) // playlist is not visible
        SetPlaylist(true);
    else
        SetPlaylist(false);
}

void MainWindow::GetPlaylistIndex()
{
    int index = InputDialog::getIndex(playlist->GetMax());
    if(index > 0)
        playlist->PlayIndex(index-1); // user index will be 1 greater than actual
}

void MainWindow::Debug(QString msg)
{
    ui->outputTextEdit->appendPlainText(msg);
}

void MainWindow::OnlineHelp()
{
    QDesktopServices::openUrl(QUrl("http://bakamplayer.u8sand.net/help"));
}

void MainWindow::CheckForUpdates() // todo
{

}

void MainWindow::AboutQt()
{
    qApp->aboutQt();
}

void MainWindow::About()
{
    AboutDialog::about(BAKA_MPLAYER_VERSION, this); // launch about dialog
}

void MainWindow::SeekForward()
{
    mpv->Seek(5, true);
}

void MainWindow::SeekBack()
{
    mpv->Seek(-5, true);
}

void MainWindow::AddSubtitleTrack()
{
    QString trackFile = QFileDialog::getOpenFileName(this, "Open subtitle file", playlist->GetPath(), "*.srt"); // todo: add more formats
    mpv->AddSub(trackFile);
    // todo: add track to tracklist
    // todo: select this track? it's not auto selected
}

void MainWindow::FitWindow(double scale) // todo
{
    if(isFullScreen())
        return;

    if(scale == 0) // current window
    {
    }
    else
    {
    }
}

void MainWindow::SetAspectRatio(double ratio) // todo
{
    if(isFullScreen())
        return;

    if(ratio == 0) // autodetect
    {
    }
    else
    {
    }
}

void MainWindow::IncreaseFontSize()
{
    mpv->AddSubScale(.02);
}

void MainWindow::DecreaseFontSize()
{
    mpv->AddSubScale(-.02);
}

void MainWindow::ResetFontSize()
{
    mpv->SetSubScale(1);
}

void MainWindow::DimLights(bool dim) // todo: make sure the dim window can't get focus
{
    if(dim)
        light->show();
    else
        light->close();
    activateWindow();
    raise();
    setFocus();
}

void MainWindow::IncreaseVolume()
{
    mpv->AddVolume(5);
}

void MainWindow::DecreaseVolume()
{
    mpv->AddVolume(-5);
}

void MainWindow::ShowInTray(bool show)
{
    trayIcon->setVisible(show);
}

void MainWindow::HidePopup(bool hide) // todo
{
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
