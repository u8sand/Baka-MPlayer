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

#include "aboutdialog.h"
#include "infodialog.h"
#include "locationdialog.h"
#include "jumpdialog.h"

MainWindow::MainWindow(QSettings *_settings, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(_settings)
{
    ui->setupUi(this);
    ui->playlistLayoutWidget->setVisible(false); // no playlist by default

    // load settings
    setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                    Qt::AlignCenter,
                                    QSize(settings->value("window/width", 600).toInt(),
                                          settings->value("window/height", 430).toInt()),
                                    qApp->desktop()->availableGeometry()));
    ui->showAllButton->setChecked(settings->value("playlist/show-all", false).toBool());
    ui->volumeSlider->setValue(settings->value("mpv/volume", 100).toInt());
    ui->actionOpen_Last_File->setEnabled(settings->value("last-file", "").toString()!="");
    ui->actionShow_D_ebug_Output->setChecked(settings->value("debug/output", false).toBool());
    ui->outputTextEdit->setVisible(settings->value("debug/output", false).toBool());

    // initialize managers/handlers
    mpv = new MpvHandler(settings, ui->mpvFrame->winId());
    playlist = new PlaylistManager(settings);

    // setup signals & slots
                                                                        // mpv updates
    connect(mpv, SIGNAL(FileChanged(QString)),                          // MPV_EVENT file changed
            this, SLOT(SetTitle(QString)));                             // set window title
    connect(mpv, SIGNAL(TimeChanged(int)),                              // MPV_EVENT time-pos update
            this, SLOT(SetTime(int)));                                  // adjust time and slider accordingly
    connect(mpv, SIGNAL(TotalTimeChanged(int)),                         // MPV_EVENT total time changed
            ui->seekBar, SLOT(setTracking(int)));                       // send the value to the seekbar so it can give track
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
            ui->playlistLayoutWidget, SLOT(setVisible(bool)));          // set visibility of the playlist
    connect(playlist, SIGNAL(ListChanged(QStringList)),                 // playlist update list
            this, SLOT(UpdatePlaylist(QStringList)));                   // update the playlistWidget
    connect(playlist, SIGNAL(IndexChanged(int)),                        // playlist update index
            this, SLOT(UpdatePlaylistIndex(int)));                      // update the playlistWidget selection
    connect(playlist, SIGNAL(ShuffleChanged(bool)),                     // playlist shuffle changed
            ui->actionSh_uffle, SLOT(setChecked(bool)));                // update the menu item
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
            mpv, SLOT(PlayPause()));                                    // mpv playpause
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
            playlist, SLOT(PlayNext()));                                // play the next entry in the playlist
    connect(ui->actionPlay_Previous_File, SIGNAL(triggered()),          // File -> Play Previous File
            playlist, SLOT(PlayPrevious()));                            // play the previous entry in the playlist
    connect(ui->actionE_xit_2, SIGNAL(triggered()),                     // File -> Exit
            this, SLOT(close()));                                       // close this window
                                                                        // View ->
    connect(ui->action_Full_Screen, SIGNAL(triggered()),                // View -> Full Screen
            this, SLOT(FullScreen()));                                  // full screen window
    connect(ui->actionTake_Snapshot, SIGNAL(triggered()),               // View -> Take Snapshot
            mpv, SLOT(Snapshot()));                                     // mpv snapshot
    // todo: fit window menu
    // todo: aspect ratio menu
    connect(ui->actionShow_Subtitles, SIGNAL(triggered()),              // View -> Show Subtitles
            mpv, SLOT(ToggleSubs()));                                   // mpv show subs
                                                                        // subtitle track menu
    // todo: font size menu
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
            ui->playlistLayoutWidget, SLOT(setVisible(bool)));                             // toggle playlist visibility
    connect(ui->action_Hide_Album_Art_2, SIGNAL(triggered(bool)),       // Options -> Hide Album Art
            this, SLOT(ShowAlbumArt(bool)));                            // toggle album art
    connect(ui->action_Dim_Lights_2, SIGNAL(triggered(bool)),           // Options -> Dim Lights
            this, SLOT(DimLights(bool)));                               // toggle dim lights
    connect(ui->actionShow_D_ebug_Output, SIGNAL(triggered(bool)),      // Options -> Show Debug Output
            ui->outputTextEdit, SLOT(setVisible(bool)));                // toggle debug output
    // todo: on-top menu
    // todo: tray icon menu
                                                                        // Help ->
    connect(ui->actionOnline_Help, SIGNAL(triggered()),                 // Help -> Online Help
            this, SLOT(OnlineHelp()));                                  // open online help
    connect(ui->action_Check_for_Updates, SIGNAL(triggered()),          // Help -> Check for Updates
            this, SLOT(CheckForUpdates()));                             // check for updates
    connect(ui->actionAbout_Qt, SIGNAL(triggered()),                    // Help -> About Qt
            this, SLOT(AboutQt()));                                     // show about qt
    connect(ui->actionAbout_Baka_MPlayer, SIGNAL(triggered()),          // Help -> About Baka MPlayer
            this, SLOT(About()));                                       // show about dialog

    // load arguments
    // todo: put baka mplayer options and such rather than just blindly treating all args as files
    for(auto arg = QCoreApplication::arguments().begin()+1; arg != QCoreApplication::arguments().end(); ++arg) // loop through arguments except first (executable name)
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

void MainWindow::SetPlaybackControls(bool enable)
{
    ui->seekBar->setEnabled(enable);
    // playback controls
    ui->rewindButton->setEnabled(enable);
    ui->playlistButton->setEnabled(enable);
    ui->playButton->setEnabled(enable);
    ui->playButton->Update();

    // next button
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

    // previous button
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

    // todo: same thing as above, disable if first/last entry
    ui->action_Next_Chapter->setEnabled(enable);
    ui->action_Previous_Chapter->setEnabled(enable);

    // menubar
    ui->action_Play->setEnabled(enable);
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Playlist->setEnabled(enable);
    ui->action_Full_Screen->setEnabled(enable);
}

void MainWindow::SetTitle(QString title)
{
    if(title == "")
        setWindowTitle("Baka MPlayer");
    else
        setWindowTitle(QFileInfo(title).fileName());
}

QString MainWindow::FormatTime(int _time)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(mpv->GetTotalTime() >= 3600) // hours
        return time.toString("h:mm:ss");
    if(mpv->GetTotalTime() >= 60)   // minutes
        return time.toString("mm:ss");
    return time.toString("0:ss");   // seconds
}

void MainWindow::SetTime(int time)
{
    // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
    // the formula is a simple ratio seekBar's max * time/totalTime
    ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)time/mpv->GetTotalTime()));

    // set duration and remaining labels, QDateTime takes care of formatting for us
    ui->durationLabel->setText(FormatTime(time));
    ui->remainingLabel->setText("-"+FormatTime(mpv->GetTotalTime()-time));
}

void MainWindow::SetPlayState(Mpv::PlayState playState)
{
    // triggered when mpv playstate is changed so we can update controls accordingly
    switch(playState)
    {
    case Mpv::Started: // ignore, use loaded--dispite it's name loaded comes after started
        break;
    case Mpv::Loaded:
    {
        SetPlaybackControls(true);
        mpv->PlayPause();
        // todo: clean this up, use a function to remove repetition

        // load chapter list into menus
        QList<Mpv::Chapter> chapters = mpv->GetChapters();
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
            ui->menu_Chapters->addAction("[ none ]")->setEnabled(false);
        ui->seekBar->setTicks(ticks);

        // load subtitle list into menus
        signalMapper = new QSignalMapper(this);
        QList<Mpv::Track> tracks = mpv->GetTracks();
        ui->menuSubtitle_Track->clear();
        for(auto &track : tracks)
        {
            if(track.type == "sub")
            {
                QAction *action = ui->menuSubtitle_Track->addAction(QString::number(track.id)+": "+track.title+" ("+track.lang+")");
                signalMapper->setMapping(action, track.id);
                connect(action, SIGNAL(triggered()),
                        signalMapper, SLOT(map()));
            }
        }
        if(ui->menuSubtitle_Track->actions().count() == 0)
            ui->menuSubtitle_Track->addAction("[ none ]")->setEnabled(false);
        connect(signalMapper, SIGNAL(mapped(int)),
                mpv, SLOT(SetSid(int)));
        break;
    }
    case Mpv::Playing:
        ui->playButton->SetPlay(false);
        ui->action_Play->setText("&Pause");
        break;
    case Mpv::Paused:
    case Mpv::Stopped:
        ui->playButton->SetPlay(true);
        ui->action_Play->setText("&Play");
        break;
    case Mpv::Idle:
        if(!ui->actionStop_after_Current->isChecked())
            playlist->Next();
        else
            SetPlaybackControls(false);
        break;
    case Mpv::Ended:
        settings->setValue("last-file", mpv->GetFile());
        ui->actionOpen_Last_File->setEnabled(settings->value("last-file").toString()!="");
        break;
    }
}

void MainWindow::Seek(int position)
{
    mpv->Seek(((double)position/ui->seekBar->maximum())*mpv->GetTotalTime());
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

void MainWindow::OpenUrl() // todo
{
    playlist->LoadFile(LocationDialog::getUrl(this));
}

void MainWindow::FullScreen() // todo: make nicer
{
    static Qt::WindowStates state, frame_state;
    if(windowState() & Qt::WindowFullScreen)
    {
        setWindowState(state);
        ui->mpvFrame->setWindowState(frame_state);
    }
    else
    {
        state = windowState();
        frame_state = ui->mpvFrame->windowState();

        showFullScreen();
        ui->mpvFrame->showFullScreen();
    }
}

void MainWindow::JumpToTime()
{
    int time = JumpDialog::getTime(mpv->GetTotalTime(),this);
    if(time >= 0)
        mpv->Seek(time);
}

void MainWindow::MediaInfo() // todo
{
//    InfoDialog::info(mpv->GetMetaData(), this);
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

void MainWindow::TogglePlaylist()
{
    ui->playlistLayoutWidget->setVisible(!ui->playlistLayoutWidget->isVisible());
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
    AboutDialog::about(this); // launch about dialog
}
