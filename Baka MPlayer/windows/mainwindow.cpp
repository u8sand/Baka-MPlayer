#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QSplitter>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    locationDialog(new LocationDialog)
{
    ui->setupUi(this);

    settings = new SettingsManager();
    mpv = new MpvHandler(ui->mpvFrame->winId());
    playlist = new PlaylistManager(ui->playlistWidget);

    // setup ui
    ui->splitter->setSizes({ (int)((double)width()*0.70), (int)((double)width()*0.30) });
    ui->splitter->setStretchFactor(0, 1); // variable size during resize (mpvFrame)
    ui->splitter->setStretchFactor(1, 0); // fixed size during resize (playlistWidget)

    // todo: consistent naming: Fix up the whole different uses of Url and File

    // mpv updates
//    connect(mpv, SIGNAL(UrlChanged(QString)),
//            playlist, SLOT(Select(QString)));
    connect(mpv, SIGNAL(TimeChanged(time_t)),
            this, SLOT(SetTime(time_t)));
    connect(mpv, SIGNAL(PlayStateChanged(Mpv::PlayState)),
            this, SLOT(SetPlayState(Mpv::PlayState)));
    connect(mpv, SIGNAL(VolumeChanged(int)),
            ui->volumeSlider, SLOT(setValueNoSignal(int)));
    connect(mpv, SIGNAL(ErrorSignal(QString)),
            this, SLOT(HandleError(QString err)));

    // dialogs
    connect(locationDialog, SIGNAL(Done(QString)),
            playlist, SLOT(Select(QString)));

    // playlist
    connect(playlist, SIGNAL(PlayFile(QString)),
            mpv, SLOT(OpenFile(QString)));
    connect(playlist, SIGNAL(PlayUrl(QString)),
            mpv, SLOT(OpenUrl(QString)));

    // sliders
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)),
            mpv, SLOT(AdjustVolume(int)));
    connect(ui->seekBar, SIGNAL(valueChanged(int)),
            this, SLOT(SetSeekBar(int)));

    // buttons
    connect(ui->openButton, SIGNAL(clicked()),
            this, SLOT(OpenFile()));
    connect(ui->playButton, SIGNAL(clicked()),
            mpv, SLOT(PlayPause()));
    connect(ui->playlistButton, SIGNAL(clicked()),
            playlist, SLOT(ToggleVisibility()));
    connect(ui->previousButton, SIGNAL(clicked()),
            playlist, SLOT(PlayPrevious()));
    connect(ui->nextButton, SIGNAL(clicked()),
            playlist, SLOT(PlayNext()));
    connect(ui->rewindButton, SIGNAL(clicked()),
            this, SLOT(Rewind()));

    // file menu
    connect(ui->action_New_Player, SIGNAL(triggered()),
            this, SLOT(NewPlayer()));
    connect(ui->action_Open_File, SIGNAL(triggered()),
            this, SLOT(OpenFile()));
    connect(ui->actionOpen_URL, SIGNAL(triggered()),
            locationDialog, SLOT(exec()));
    connect(ui->actionOpen_Path_from_Clipboard, SIGNAL(triggered()),
            this, SLOT(OpenFileFromClipboard()));
    connect(ui->actionOpen_Last_File, SIGNAL(triggered()),
            this, SLOT(OpenLastFile()));
    connect(ui->actionE_xit, SIGNAL(triggered()),
            this, SLOT(close()));

    // playback menu
    connect(ui->action_Play, SIGNAL(triggered()),
            mpv, SLOT(PlayPause()));
    connect(ui->action_Stop, SIGNAL(triggered()),
            mpv, SLOT(Stop()));
    connect(ui->action_Rewind, SIGNAL(triggered()),
            mpv, SLOT(Rewind()));
//    connect(ui->actionR_estart, SIGNAL(triggered()),
//            mpv, SLOT(Restart()));
//    connect(ui->actionShu_ffle, SIGNAL(triggered()),
//            playlist, SLOT(Shuffle()));
    // repeat
//    connect(ui->actionStop_after_current, SIGNAL(triggered()),
//            playlist, SLOT(StopAfterCurrent()));
//    conect(ui->action_Frame_Step, SIGNAL(triggered()),
//           mpv, SLOT(Seek(int)));
//    conect(ui->actionFrame_Back_Step, SIGNAL(triggered()),
//           mpv, SLOT(Seek(int)));
//    connect(ui->action_Jump_To_Time, SIGNAL(triggered()),
//            this, SLOT(JumpToTime()));

    // media

    // subtitles

    // tools

    //options

    //help

    // todo: more arguments, not as primitive, files vs directories, etc.
    QStringList args = QCoreApplication::arguments();
    if(args.count() > 1) playlist->Select(*(QCoreApplication::arguments().begin()+1));
}

MainWindow::~MainWindow()
{
    delete playlist;
    delete mpv;
    delete settings;
    delete locationDialog;
    delete ui;
}

void MainWindow::SetTime(time_t time)
{
    ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*((double)time/mpv->GetTotalTime()));
    ui->durationLabel->setText(QDateTime::fromTime_t(time).toUTC().toString("h:mm:ss"));
    ui->remainingLabel->setText(QDateTime::fromTime_t(mpv->GetTotalTime()-time).toUTC().toString("-h:mm:ss"));
}

void MainWindow::SetControls(bool enable)
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
    ui->action_Rewind->setEnabled(enable);
    ui->actionR_estart->setEnabled(enable);
    ui->action_Jump_To_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->action_Show_Playlist->setEnabled(enable);
}

void MainWindow::SetPlayState(Mpv::PlayState playState)
{
    switch(playState)
    {
    case Mpv::Started:
        SetControls(true);
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
        SetControls(false);
        break;
    case Mpv::Ended:
        SetTime(0);
        SetControls(false);
        //playlist->PlayNext();
        break;
    }
}

void MainWindow::NewPlayer()
{
    (new MainWindow())->show();
}

void MainWindow::OpenFile()
{
    playlist->SelectFile(QFileDialog::getOpenFileName(this, "Open file"));
}

void MainWindow::OpenFileFromClipboard()
{
    playlist->Select(QApplication::clipboard()->text());
}

void MainWindow::OpenLastFile()
{
    // todo: open last file--comes from settings
}

void MainWindow::HandleError(QString err)
{
    QMessageBox::warning(this, "Error", err);
}

void MainWindow::SetSeekBar(int position)
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
            mpv->Rewind();
        else
            mpv->Stop();
    }
}
