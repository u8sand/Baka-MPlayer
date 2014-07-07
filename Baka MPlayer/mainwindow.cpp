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
    seekBar = this->findChild<QSlider*>("seekBar");
    durationLabel = this->findChild<QLabel*>("durationLabel");
    remainingLabel = this->findChild<QLabel*>("remainingLabel");
    playButton = this->findChild<QPushButton*>("playButton");

    mpv = new MpvHandler(this->findChild<QFrame*>("outputFrame")->winId(), wakeup, this);
}

MainWindow::~MainWindow()
{
    delete mpv;
    delete ui;
}

inline void MainWindow::SetDurationLabel()
{
    durationLabel->setText(QDateTime::fromTime_t(mpv->GetTime()).toUTC().toString("hh:mm:ss"));
}

inline void MainWindow::SetRemainingLabel()
{
    remainingLabel->setText(QDateTime::fromTime_t(mpv->GetTotalTime()-mpv->GetTime()).toUTC().toString("-hh:mm:ss"));
}

inline void MainWindow::SetSeekBar()
{
    seekBar->setValue(seekBar->maximum()*((double)mpv->GetTime()/mpv->GetTotalTime()));
}

inline void MainWindow::SetPlayButton()
{
    if(mpv->GetState() == MpvHandler::Playing)
        playButton->setText("ll");
    else
        playButton->setText("►");
}

bool MainWindow::HandleMpvEvent(MpvHandler::MpvEvent event)
{
    switch(event)
    {
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

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    mpv->Volume(value);
}
