#include "inputdialog.h"
#include "ui_inputdialog.h"

#include <QString>

InputDialog::InputDialog(QString prompt, QString title, const std::function<bool (QString)> &_validation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog),
    validation(_validation)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->messageLabel->setText(prompt);

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

QString InputDialog::getInput(QString prompt, QString title, const std::function<bool (QString)> &validation, QWidget *parent)
{
    InputDialog dialog(prompt, title, validation, parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.ui->inputLineEdit->text();
    else
        return "";
}

void InputDialog::validate(QString input)
{
    ui->okButton->setEnabled(input != "" && validation(input));
}
