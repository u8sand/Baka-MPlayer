#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateManager *_update, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    update(_update)
{
    ui->setupUi(this);

    connect(update, &UpdateManager::UpdateAvailable,
            [=](QMap<QString, QString> info)
            {
                ui->versionLabel->setText(info["version"]+" released "+info["date"]);
                ui->plainTextEdit->setPlainText(info["bugfixes"]);
                if(info["version"] == BAKA_MPLAYER_VERSION)
                {
                    ui->downloadButton->setEnabled(false);
                    ui->updateLabel->setText("You have the latest version!");
                }

            });

    connect(ui->downloadButton, &QPushButton::clicked,
            [=]
            {
                update->DownloadUpdate();
            });

    connect(update, &UpdateManager::Downloaded,
            [=](int percent)
            {
                if(percent == 100)
                   accept();
                else
                {
                    // update progress bar with percent
                }
            });

    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));


    update->CheckForUpdates();
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
