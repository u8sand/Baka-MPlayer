#include "updatedialog.h"
#include "ui_updatedialog.h"

#include "util.h"

#include <QDesktopServices>

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    init(false)
{
    ui->setupUi(this);

    updateManager = new UpdateManager(this);

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // no update support on unix/linux, we just show if one is available
    ui->updateButton->setVisible(false);
    ui->cancelButton->setText(tr("&CLOSE"));
    ui->cancelButton->setDefault(true);
#endif

    connect(updateManager, &UpdateManager::versionInfoReceived,
            [=](QMap<QString, QString> info)
            {
                ui->plainTextEdit->setPlainText(info["bugfixes"]);
                if(info["version"].trimmed() == BAKA_MPLAYER_VERSION)
                {
                    ui->updateButton->setEnabled(false);
                    ui->updateLabel->setText(tr("You have the latest version!"));
                }
                else
                {
                    ui->updateLabel->setText(tr("Update Available!\nVersion: %0").arg(info["version"]));
#if defined(Q_OS_WIN)
                    version = info["version"];
                    // url = info["url"];
                    url = "http://bakamplayer.u8sand.net/Baka%20MPlayer.7z";

                    if(!isVisible())
                        show();
#endif

                }
                ui->progressBar->setVisible(false);
                ui->timeRemainingLabel->setVisible(false);
            });

#if defined(Q_OS_WIN)
    connect(ui->updateButton, &QPushButton::clicked,
            [=]
            {
                QDesktopServices::openUrl(QUrl(Util::DownloadFileUrl()));
                /*
                avgSpeed = 0;
                lastSpeed = 0;
                lastProgress = 0;
                lastTime = 0;
                timer = new QTime();
                timer->start();
                ui->updateLabel->setText(tr("Downloading update..."));
                ui->progressBar->setVisible(true);
                ui->timeRemainingLabel->setVisible(true);
                ui->plainTextEdit->clear();
                updateManager->DownloadUpdate(url, version);
                */
            });
#endif

    connect(updateManager, &UpdateManager::progressSignal,
            [=](int percent)
            {
                ui->progressBar->setValue(percent);
                if(percent == 100)
                {
                    ui->updateLabel->setText(tr("Download Complete"));
                    ui->progressBar->setVisible(false);
                    ui->timeRemainingLabel->setVisible(false);
                    if(timer)
                    {
                        delete timer;
                        timer = nullptr;
                    }
                }
                else if(timer) // don't execute this if timer is not defined--this shouldn't happen though.. but it does
                {
                    avgSpeed = 0.005*lastSpeed + 0.995*avgSpeed;

                    if(avgSpeed > 0)
                        ui->timeRemainingLabel->setText(tr("About %0 second(s) remaining").arg(QString::number(1/(1000*avgSpeed))));
                    else
                        ui->timeRemainingLabel->setText(tr("Calculating..."));

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
                ui->plainTextEdit->appendPlainText(tr("error: %0\n").arg(msg));
            });

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
}

UpdateDialog::~UpdateDialog()
{
    delete updateManager;
    delete ui;
}

int UpdateDialog::exec()
{
    if(!init)
        CheckForUpdates();
    return QDialog::exec();
}

void UpdateDialog::CheckForUpdates()
{
    init = true;
    avgSpeed = 0;
    lastSpeed = 0;
    lastProgress = 0;
    lastTime = 0;
    timer = new QTime();
    timer->start();
    updateManager->CheckForUpdates();
}
