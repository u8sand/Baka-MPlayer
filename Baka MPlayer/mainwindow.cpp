#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

static void wakeup(void *ctx)
{
    MainWindow *mainwindow = (MainWindow*)ctx;
    QCoreApplication::postEvent(mainwindow, new QEvent(QEvent::User));
}

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings = new SettingsManager();
    // todo: apply form settings

//    playlist = new PlaylistManager(playlistWidget);

    // todo: forward mpv-related settings to the mpv handler
    mpv = new MpvHandler(this->findChild<QFrame*>("outputFrame")->winId(), wakeup, this);

//    QStringList args = QCoreApplication::arguments();
//    for(QStringList::iterator it = args.begin(); it != args.end(); ++it)
//        playlist->addItem(*it);
//    playlist->PlayNext();
}

MainWindow::~MainWindow()
{
    // todo: save form settings
    delete settings;
    delete mpv;
    delete ui;
}

inline void MainWindow::SetTimeLabels()
{
    ui->durationLabel->setText(QDateTime::fromTime_t(mpv->GetTime()).toUTC().toString("h:mm:ss"));
    ui->remainingLabel->setText(QDateTime::fromTime_t(mpv->GetTotalTime()-mpv->GetTime()).toUTC().toString("-h:mm:ss"));
}

inline void MainWindow::SetSeekBar()
{
    ui->seekBar->setValue(ui->seekBar->maximum()*((double)mpv->GetTime()/mpv->GetTotalTime()));
}

inline void MainWindow::SetPlayButton()
{
    if(mpv->GetPlayState() == MpvHandler::Playing)
    {
        ui->playButton->setIcon(QIcon(":/img/default_pause.svg"));
        ui->action_Play->setText("&Pause");
    }
    else
    {
        ui->playButton->setIcon(QIcon(":/img/default_play.svg"));
        ui->action_Play->setText("&Play");
    }
}

inline void MainWindow::EnableControls()
{
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
}

bool MainWindow::HandleMpvEvent(MpvHandler::MpvEvent event)
{
    switch(event)
    {
    case MpvHandler::FileOpened:
        EnableControls();
    case MpvHandler::FileEnded:
        SetTimeLabels();
        SetSeekBar();
        SetPlayButton();
//        playlist->PlayNext();
        break;
    case MpvHandler::TimeChanged:
        SetTimeLabels();
        SetSeekBar();
        break;
    case MpvHandler::StateChanged:
        SetPlayButton();
        break;
    case MpvHandler::NoEvent:
        return false;
    default:
        break;
    }
    return true;
}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        while(HandleMpvEvent(mpv->HandleEvent()));
        return true;
    }
    return QMainWindow::event(event);
}

void MainWindow::on_action_Open_File_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    mpv->OpenFile(filename);
}

void MainWindow::on_actionE_xit_triggered()
{
    QCoreApplication::exit();
}

void MainWindow::on_openButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open file");
    mpv->OpenFile(filename);
}

void MainWindow::on_playButton_clicked()
{
    mpv->PlayPause();
}

void MainWindow::on_rewindButton_clicked()
{
    if(mpv->GetTime() < 3)
    {
        mpv->Stop();
    }
    else
    {
        switch(mpv->GetPlayState())
        {
            case MpvHandler::Playing:
                break;
            default:
                mpv->Stop();
                break;
        }
    }
}

void MainWindow::on_previousButton_clicked()
{
    mpv->Seek(-5, true);
}

void MainWindow::on_nextButton_clicked()
{
    mpv->Seek(5, true);
}

void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    mpv->Volume(position);
}

void MainWindow::on_seekBar_sliderMoved(int position)
{
    mpv->Seek(((double)position/ui->seekBar->maximum())*mpv->GetTotalTime());
}

void MainWindow::on_seekBar_sliderPressed()
{
    mpv->Seek(((double)ui->seekBar->value()/ui->seekBar->maximum())*mpv->GetTotalTime());
}

void MainWindow::on_playlistButton_clicked()
{
    // todo: playlist functionality
    ui->playlistWidget->setVisible(!ui->playlistWidget->isVisible());
}

void MainWindow::on_action_Play_triggered()
{
    mpv->PlayPause();
}
