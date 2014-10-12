#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateManager *updateManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    ui->updateButton->setVisible(false); // no update support on unix/linux, we just show if one is available
#endif

    connect(updateManager, &UpdateManager::Update,
            [=](QMap<QString, QString> info)
            {
                ui->updateLabel->setText(info["version"]+" Released "+info["date"]);
                ui->plainTextEdit->setPlainText(info["bugfixes"]);
                if(info["version"] == BAKA_MPLAYER_VERSION)
                {
                    ui->updateButton->setEnabled(false);
                    ui->updateLabel->setText("You have the latest version!");
                }

            });
//#if defined(Q_OS_WIN)
    connect(ui->updateButton, &QPushButton::clicked,
            [=]
            {
                updateManager->DownloadUpdate();
            });

    connect(updateManager, &UpdateManager::Downloaded,
            [=](int percent)
            {
                if(percent == 100)
                   accept();
                else
                {
                    // update progress bar with percent
                }
            });
//#endif
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));

    updateManager->CheckForUpdates(); // todo: do this after dialog is opened
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
