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
    seekBar = this->findChild<CustomSlider*>("seekBar");
    durationLabel = this->findChild<QLabel*>("durationLabel");
    remainingLabel = this->findChild<QLabel*>("remainingLabel");
    playButton = this->findChild<QPushButton*>("playButton");
    rewindButton = this->findChild<QPushButton*>("rewindButton");
    previousButton = this->findChild<QPushButton*>("previousButton");
    nextButton = this->findChild<QPushButton*>("nextButton");
    playlistWidget = this->findChild<QListWidget*>("playlistWidget");

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

inline void MainWindow::SetDurationLabel()
{
    durationLabel->setText(QDateTime::fromTime_t(mpv->GetTime()).toUTC().toString("h:mm:ss"));
}

inline void MainWindow::SetRemainingLabel()
{
    remainingLabel->setText(QDateTime::fromTime_t(mpv->GetTotalTime()-mpv->GetTime()).toUTC().toString("-h:mm:ss"));
}

inline void MainWindow::SetSeekBar()
{
    seekBar->setValue(seekBar->maximum()*((double)mpv->GetTime()/mpv->GetTotalTime()));
}

inline void MainWindow::SetPlayButton()
{
    if(mpv->GetState() == MpvHandler::Playing)
        playButton->setIcon(QIcon(":/img/default_pause.svg"));
    else
        playButton->setIcon(QIcon(":/img/default_play.svg"));
}

inline void MainWindow::EnableControls()
{
    playButton->setEnabled(true);
    rewindButton->setEnabled(true);
    previousButton->setEnabled(true);
    nextButton->setEnabled(true);
}

bool MainWindow::HandleMpvEvent(MpvHandler::MpvEvent event)
{
    switch(event)
    {
    case MpvHandler::FileOpened:
        EnableControls();
    case MpvHandler::FileEnded:
        SetDurationLabel();
        SetRemainingLabel();
        SetSeekBar();
        SetPlayButton();
//        playlist->PlayNext();
        break;
    case MpvHandler::TimeChanged:
        SetDurationLabel();
        SetRemainingLabel();
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
    mpv->Stop();
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
    mpv->Seek(((double)position/seekBar->maximum())*mpv->GetTotalTime());
}

void MainWindow::on_seekBar_sliderPressed()
{
    mpv->Seek(((double)seekBar->value()/seekBar->maximum())*mpv->GetTotalTime());
}

void MainWindow::on_playlistButton_clicked()
{
    // todo: playlist functionality
    playlistWidget->setVisible(!playlistWidget->isVisible());
}
