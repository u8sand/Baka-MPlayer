#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(UpdateManager *_updateManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog),
    updateManager(_updateManager)
{
    ui->setupUi(this);

    // todo: populate update information
//    updateManager->CheckForUpdates();

    connect(ui->downloadButton, &QPushButton::clicked,
            [=]
    {
        // todo: download, when downloading is done: accept()
    });
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
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
