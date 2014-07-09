#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(new SettingsManager)
{
    ui->setupUi(this);

    // mpv updates
    connect(ui->mpv, SIGNAL(TimeChanged(time_t)),
            this, SLOT(SetTime(time_t)));
    connect(ui->mpv, SIGNAL(PlayStateChanged(Mpv::PlayState)),
            this, SLOT(SetPlayState(Mpv::PlayState)));
    connect(ui->mpv, SIGNAL(ErrorSignal(QString)),
            this, SLOT(HandleError(QString)));

    // slider
    connect(this->ui->volumeSlider, SIGNAL(valueChanged(int)),
            this->ui->mpv, SLOT(AdjustVolume(int)));

    // buttons
    connect(ui->openButton, SIGNAL(clicked()),
            this, SLOT(OpenFile()));
    connect(ui->playButton, SIGNAL(clicked()),
            ui->mpv, SLOT(PlayPause()));

    // menu
    connect(ui->action_Open_File, SIGNAL(triggered()),
            this, SLOT(OpenFile()));
    connect(ui->action_Play, SIGNAL(triggered()),
            ui->mpv, SLOT(PlayPause()));
    connect(ui->actionE_xit, SIGNAL(triggered()),
            QCoreApplication::instance(), SLOT(quit()));

//    QStringList args = QCoreApplication::arguments();
//    for(QStringList::iterator it = args.begin(); it != args.end(); ++it)
//        playlist->addItem(*it);
//    playlist->PlayNext();
}

MainWindow::~MainWindow()
{
    // todo: save form settings
    delete settings;
    delete ui;
}

void MainWindow::SetTime(time_t time)
{
    ui->seekBar->setValue(ui->seekBar->maximum()*((double)time/ui->mpv->GetTotalTime()));
    ui->durationLabel->setText(QDateTime::fromTime_t(time).toUTC().toString("h:mm:ss"));
    ui->remainingLabel->setText(QDateTime::fromTime_t(ui->mpv->GetTotalTime()-time).toUTC().toString("-h:mm:ss"));
}

void MainWindow::SetPlayState(Mpv::PlayState playState)
{
    switch(playState)
    {
    case Mpv::Started:
        ui->seekBar->setEnabled(true);
        // playback controls
        ui->rewindButton->setEnabled(true);
        ui->previousButton->setEnabled(true);
        ui->playButton->setEnabled(true);
        ui->nextButton->setEnabled(true);
        ui->playlistButton->setEnabled(true);
        // menubar
        ui->action_Play->setEnabled(true);
        ui->action_Stop->setEnabled(true);
        ui->action_Rewind->setEnabled(true);
        ui->actionR_estart->setEnabled(true);
        ui->action_Jump_To_Time->setEnabled(true);
        ui->actionMedia_Info->setEnabled(true);
        ui->action_Show_Playlist->setEnabled(true);
        break;
    case Mpv::Playing:
        ui->playButton->setIcon(QIcon(":/img/default_pause.svg"));
        ui->action_Play->setText("&Pause");
        break;
    case Mpv::Paused:
        ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
        ui->action_Play->setText("&Play");
        break;
    case Mpv::Stopped:
        ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
        ui->action_Play->setText("&Play");
        break;
    case Mpv::Idle:
    case Mpv::Ended:
        SetTime(0);
        ui->seekBar->setEnabled(false);
        // playback controls
        ui->rewindButton->setEnabled(false);
        ui->previousButton->setEnabled(false);
        ui->playButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
        ui->playlistButton->setEnabled(false);
        // menubar
        ui->action_Play->setEnabled(false);
        ui->action_Stop->setEnabled(false);
        ui->action_Rewind->setEnabled(false);
        ui->actionR_estart->setEnabled(false);
        ui->action_Jump_To_Time->setEnabled(false);
        ui->actionMedia_Info->setEnabled(false);
        ui->action_Show_Playlist->setEnabled(false);
        break;
    }
}

void MainWindow::HandleError(QString err)
{
    QMessageBox::warning(this, "Error", err);
}

void MainWindow::OpenFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    ui->mpv->OpenFile(filename);
}

// todo: convert the rest of these to signals

void MainWindow::on_rewindButton_clicked()
{
    // not sure why this clause is here; todo: ask
    if(ui->mpv->GetTime() < 3)
    {
        ui->mpv->Stop();
    }
    else
    {
        switch(ui->mpv->GetPlayState())
        {
            case Mpv::Playing:
                break;
            default:
                ui->mpv->Stop();
                break;
        }
    }
}

void MainWindow::on_previousButton_clicked()
{
    ui->mpv->Seek(-5, true);
}

void MainWindow::on_nextButton_clicked()
{
    ui->mpv->Seek(5, true);
}

void MainWindow::on_seekBar_sliderMoved(int position)
{
    ui->mpv->Seek(((double)position/ui->seekBar->maximum())*ui->mpv->GetTotalTime());
}

void MainWindow::on_seekBar_sliderPressed()
{
    ui->mpv->Seek(((double)ui->seekBar->value()/ui->seekBar->maximum())*ui->mpv->GetTotalTime());
}

void MainWindow::on_playlistButton_clicked()
{
    // todo: playlist functionality
    ui->playlistWidget->setVisible(!ui->playlistWidget->isVisible());
}
