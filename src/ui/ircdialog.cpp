#include "ircdialog.h"
#include "ui_ircdialog.h"

IrcDialog::IrcDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IrcDialog)
{
    ui->setupUi(this);
}

IrcDialog::~IrcDialog()
{
    delete ui;
}
