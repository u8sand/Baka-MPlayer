#include "infodialog.h"
#include "ui_infodialog.h"

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::info(QWidget *parent) // todo
{
    InfoDialog dialog(parent);
    dialog.exec();
}
