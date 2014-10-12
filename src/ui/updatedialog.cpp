#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateManager *updateManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // no update support on unix/linux, we just show if one is available
    //ui->updateButton->setVisible(false);
    //ui->cancelButton->setText("&Close");
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
                    ui->updateLabel->setText("Update available\n"+info["version"]+" Released "+info["date"]);
                ui->progressBar->setVisible(false);
                ui->timeRemainingLabel->setVisible(false);
            });

//#if defined(Q_OS_WIN)
    connect(ui->updateButton, &QPushButton::clicked,
            [=]
            {
                ui->updateLabel->setText("Downloading update...");
                ui->progressBar->setVisible(true);
                ui->timeRemainingLabel->setVisible(true);
                ui->plainTextEdit->clear();
                updateManager->DownloadUpdate(); // todo: update progressbar/timeRemaining
            });
//#endif

    connect(updateManager, &UpdateManager::progressSignal,
            [=](int percent)
            {
                ui->progressBar->setValue(percent);
                if(percent == 100)
                {
                    ui->updateLabel->setText("Download Complete");
                    ui->progressBar->setVisible(false);
                    ui->timeRemainingLabel->setVisible(false);
                    updateManager->ApplyUpdate();
                }
                else
                {
                    // calculate time remaining and update timeRemainingLabel
                }
            });

    connect(updateManager, &UpdateManager::verboseSignal,
            [=](QString msg)
            {
                ui->plainTextEdit->appendPlainText(msg+"\n");
            });

    connect(updateManager, &UpdateManager::errorSignal,
            [=](QString msg)
            {
                ui->plainTextEdit->appendPlainText("error: "+msg+"\n");
                // cancel current process
            });

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));

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
