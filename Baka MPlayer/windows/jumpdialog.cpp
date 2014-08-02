#include "jumpdialog.h"
#include "ui_jumpdialog.h"

JumpDialog::JumpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JumpDialog)
{
    ui->setupUi(this);
}

JumpDialog::~JumpDialog()
{
    delete ui;
}

int JumpDialog::getTime(QWidget *parent)
{
    JumpDialog dialog(parent);
    dialog.exec();
//    if(dialog.exec() == QDialog::Accepted)
//        return dialog.ui->time->text();
//    else
        return -1;
}
