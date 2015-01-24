#include "updatedialog.h"
#include "ui_updatedialog.h"

#include "bakaengine.h"
#include "updatemanager.h"
#include "util.h"

#include <QDesktopServices>

UpdateDialog::UpdateDialog(BakaEngine *baka, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    baka(baka),
    timer(nullptr),
    init(false)
{
    ui->setupUi(this);

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // no update support on unix/linux, we just show if one is available
    ui->updateButton->setVisible(false);
    ui->cancelButton->setText(tr("&CLOSE"));
    ui->cancelButton->setDefault(true);
#endif

    connect(baka->update, &UpdateManager::progressSignal,
            [=](int percent)
            {
                ui->progressBar->setValue(percent);
                if(percent == 100)
                {
                    ui->updateLabel->setText(tr("Download Complete"));
                    ui->progressBar->setVisible(false);
                    ui->timeRemainingLabel->setVisible(false);
                    if(timer != nullptr)
                    {
                        delete timer;
                        timer = nullptr;
                    }

                    if(!init)
                    {
                        ShowInfo();
                        init = false;
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

    connect(baka->update, &UpdateManager::messageSignal,
            [=](QString msg)
            {
                ui->plainTextEdit->appendPlainText(msg);
            });

#if defined(Q_OS_WIN)
    connect(ui->updateButton, &QPushButton::clicked,
            [=]
            {
                baka->update->DownloadUpdate(Util::DownloadFileUrl(), baka->update->info["version"]);
            });
#endif

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));

    if(baka->update->getInfo().empty())
    {
        Prepare();
        baka->update->CheckForUpdates();
    }
    else
    {
        init = false;
        ShowInfo();
    }
}

UpdateDialog::~UpdateDialog()
{
    if(timer != nullptr)
        delete timer;
    delete ui;
}

void UpdateDialog::CheckForUpdates(BakaEngine *baka, QWidget *parent)
{
    UpdateDialog *dialog = new UpdateDialog(baka, parent);
    dialog->exec();
}

void UpdateDialog::Prepare()
{
    avgSpeed = 0;
    lastSpeed = 0;
    lastProgress = 0;
    lastTime = 0;
    if(timer != nullptr)
        delete timer;
    timer = new QTime();
    timer->start();
}

void UpdateDialog::ShowInfo()
{
    auto info = baka->update->getInfo();
    ui->plainTextEdit->setPlainText(info["bugfixes"]);
    if(info["version"].trimmed() == BAKA_MPLAYER_VERSION)
    {
#if defined(Q_OS_WIN)
        ui->updateButton->setEnabled(false);
#endif
        ui->updateLabel->setText(tr("You have the latest version!"));
    }
    else
    {
        ui->updateLabel->setText(tr("Update Available!\nVersion: %0").arg(info["version"]));
#if defined(Q_OS_WIN)
        ui->updateButton->setEnabled(true);
#endif
    }
    ui->progressBar->setVisible(false);
    ui->timeRemainingLabel->setVisible(false);
}
