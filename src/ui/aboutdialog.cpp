#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QString version, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText("Baka MPlayer "+version);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::about(QString version, QWidget *parent)
{
    AboutDialog dialog(version, parent);
    dialog.exec();
}
