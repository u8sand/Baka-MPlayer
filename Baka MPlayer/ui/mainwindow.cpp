#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "infodialog.h"
#include "locationdialog.h"
#include "jumpdialog.h"

#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStyle>

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
    ui->actionSh_uffle->setChecked(settings->value("playlist/shuffle", false).toBool());
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
    connect(mpv, SIGNAL(TimeChanged(time_t)),                           // MPV_EVENT time-pos update
            this, SLOT(SetTime(time_t)));                               // adjust time and slider accordingly
    connect(mpv, SIGNAL(PlayStateChanged(Mpv::PlayState)),              // MPV_EVENT playstate changes
            this, SLOT(SetPlayState(Mpv::PlayState)));                  // adjust interface based on new play-state
    connect(mpv, SIGNAL(VolumeChanged(int)),                            // MPV_EVENT volume update
            ui->volumeSlider, SLOT(setValueNoSignal(int)));             // update the volume slider without triggering it's value changed signal
    connect(mpv, SIGNAL(ErrorSignal(QString)),                          // mpv error occured
            this, SLOT(HandleError(QString)));                          // output error message
                                                                        // playlist
    connect(playlist, SIGNAL(Play(QString)),                            // playlist play signal
            mpv, SLOT(OpenFile(QString)), Qt::QueuedConnection);        // mpv open file
    connect(playlist, SIGNAL(Stop()),                                   // playlist stop signal
            mpv, SLOT(Stop()), Qt::QueuedConnection);                   // mpv stop
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
    connect(ui->voiceButton, SIGNAL(clicked()),                         // clicked voice button
            this, SLOT(ToggleVoice()));                                 // toggle voice enabled
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
            mpv, SLOT(FullScreen()));                                   // mpv full screen
    connect(ui->actionTake_Snapshot, SIGNAL(triggered()),               // View -> Take Snapshot
            mpv, SLOT(Snapshot()));                                     // mpv snapshot
    // todo: fit window menu
    // todo: aspect ratio menu
    connect(ui->actionShow_Subtitles, SIGNAL(triggered()),              // View -> Show Subtitles
            mpv, SLOT(ToggleSubs()));                                   // mpv show subs
    // todo: subtitle track menu
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
    // todo: chapters menu
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

    // todo: put baka mplayer options and such rather than just blindly treating all args as files
    for(auto arg = QCoreApplication::arguments().begin()+1; arg != QCoreApplication::arguments().end(); ++arg) // loop through arguments except first (executable name)
        playlist->LoadFile(*arg); // loadfile
}

MainWindow::~MainWindow()
{
    // save settings
    settings->setValue("last-file", mpv->GetFile());
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
    ui->previousButton->setEnabled(enable);
    ui->playButton->setEnabled(enable);
    ui->playButton->Update();
    ui->nextButton->setEnabled(enable);
    ui->playlistButton->setEnabled(enable);
    // menubar
    ui->action_Play->setEnabled(enable);
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable);
    ui->action_Playlist->setEnabled(enable);
}

void MainWindow::SetTime(time_t time)
{
    // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
    // the formula is a simple ratio seekBar's max * time/totalTime
    ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)time/mpv->GetTotalTime()));

    // set duration and remaining labels, QDateTime takes care of formatting for us
    ui->durationLabel->setText(QDateTime::fromTime_t(time).toUTC().toString("h:mm:ss"));
    ui->remainingLabel->setText(QDateTime::fromTime_t(mpv->GetTotalTime()-time).toUTC().toString("-h:mm:ss"));
}

void MainWindow::SetPlayState(Mpv::PlayState playState)
{
    // triggered when mpv playstate is changed so we can update controls accordingly
    switch(playState)
    {
    case Mpv::Started: // ignore, use loaded--dispite it's name loaded comes after started
        break;
    case Mpv::Loaded:
        SetPlaybackControls(true);
        break;
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
        SetTime(0);
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

void MainWindow::JumpToTime()
{
    mpv->Seek(JumpDialog::getTime(this));
}

void MainWindow::MediaInfo() // todo
{
//    MediaInfoDialog::show(this, mpv->MediaInfo());
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
    QDesktopServices::openUrl("file:///"+QDir::toNativeSeparators(QFileInfo(mpv->GetFile()).absoluteDir().absolutePath()));
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
