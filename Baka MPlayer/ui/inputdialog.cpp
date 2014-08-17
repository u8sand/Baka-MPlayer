#include "inputdialog.h"
#include "ui_inputdialog.h"

#include <QString>

InputDialog::InputDialog(int _max, QDialog *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog),
    max(_max)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(ui->inputLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validate(QString)));
}

InputDialog::~InputDialog()
{
    delete ui;
}

int InputDialog::getIndex(int max, QDialog *parent)
{
    InputDialog dialog(max, parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.ui->inputLineEdit->text().toInt();
    else
        return 0;
}

void InputDialog::validate(QString input)
{
    int num = input.toInt();
    ui->okButton->setEnabled(!(num > max || num < 1));
}
