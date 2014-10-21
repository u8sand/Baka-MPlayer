#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateManager *updateManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // no update support on unix/linux, we just show if one is available
    ui->updateButton->setVisible(false);
    ui->cancelButton->setText("&Close");
#endif

    connect(updateManager, &UpdateManager::versionInfoReceived,
            [=](QMap<QString, QString> info)
            {
                ui->plainTextEdit->setPlainText(info["bugfixes"]);
                if(info["version"] == BAKA_MPLAYER_VERSION)
                {
                    ui->updateButton->setEnabled(false);
                    ui->updateLabel->setText("You are up to date!\n"+info["version"]+" Released "+info["date"]);
                }
                else
                {
                    ui->updateLabel->setText("Update available\n"+info["version"]+" Released "+info["date"]);
#if defined(Q_OS_WIN)
                    version = info["version"];
                    // url = info["url"];
                    url = "http://bakamplayer.u8sand.net/Baka%20MPlayer.7z";
#endif
                }
                ui->progressBar->setVisible(false);
                ui->timeRemainingLabel->setVisible(false);
            });

#if defined(Q_OS_WIN)
    connect(ui->updateButton, &QPushButton::clicked,
            [=]
            {
                avgSpeed = 0;
                lastSpeed = 0;
                lastProgress = 0;
                lastTime = 0;
                timer = new QTime();
                timer->start();
                ui->updateLabel->setText("Downloading update...");
                ui->progressBar->setVisible(true);
                ui->timeRemainingLabel->setVisible(true);
                ui->plainTextEdit->clear();
                updateManager->DownloadUpdate(url, version);
            });
#endif

    connect(updateManager, &UpdateManager::progressSignal,
            [=](int percent)
            {
                ui->progressBar->setValue(percent);
                if(percent == 100)
                {
                    ui->updateLabel->setText("Download Complete");
                    ui->progressBar->setVisible(false);
                    ui->timeRemainingLabel->setVisible(false);
                    if(timer)
                    {
                        delete timer;
                        timer = nullptr;
                    }
                }
                else
                {
                    avgSpeed = 0.005*lastSpeed + 0.995*avgSpeed;

                    if(avgSpeed > 0)
                        ui->timeRemainingLabel->setText("About "+QString::number(1/(1000*avgSpeed))+" second(s) remaining");
                    else
                        ui->timeRemainingLabel->setText("Calculating...");

                    int time = timer->elapsed();
                    if(time != lastTime) // prevent cases when we're too fast haha
                        lastSpeed = (percent-lastProgress)/(time-lastTime);

                    lastTime = time;
                    lastProgress = percent;
                }
            });

    connect(updateManager, &UpdateManager::verboseSignal,
            [=](QString msg)
            {
                ui->plainTextEdit->appendPlainText(msg);
            });

    connect(updateManager, &UpdateManager::errorSignal,
            [=](QString msg)
            {
                ui->plainTextEdit->appendPlainText("error: "+msg+"\n");
            });

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));


    avgSpeed = 0;
    lastSpeed = 0;
    lastProgress = 0;
    lastTime = 0;
    timer = new QTime();
    timer->start();
    updateManager->CheckForUpdates();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

int UpdateDialog::update(UpdateManager *updateManager, QWidget *parent)
{
    UpdateDialog dialog(updateManager, parent);
    return dialog.exec();
}
