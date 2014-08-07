#include "jumpdialog.h"
#include "ui_jumpdialog.h"

JumpDialog::JumpDialog(int _maxTime, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JumpDialog),
    time(0),
    maxTime(_maxTime)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->hourBox, SIGNAL(valueChanged(int)),
            this, SLOT(validate()));
    connect(ui->minBox, SIGNAL(valueChanged(int)),
            this, SLOT(validate()));
    connect(ui->secBox, SIGNAL(valueChanged(int)),
            this, SLOT(validate()));

    if(maxTime > 3600)
        ui->hourBox->setFocus();
    else if(maxTime > 60)
        ui->minBox->setFocus();
    else
        ui->secBox->setFocus();
}

JumpDialog::~JumpDialog()
{
    delete ui;
}

int JumpDialog::getTime(int maxTime, QWidget *parent)
{
    JumpDialog dialog(maxTime, parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.time;
    return -1;
}

void JumpDialog::validate()
{
    time = ui->hourBox->value()*3600+
            ui->minBox->value()*60+
            ui->secBox->value();

    if(time < maxTime)
    {
        ui->validEntryLabel->setPixmap(QPixmap(":/img/exists.svg"));
        ui->okButton->setEnabled(true);
    }
    else
    {
        ui->validEntryLabel->setPixmap(QPixmap(":/img/not_exists.svg"));
        ui->okButton->setEnabled(false);
    }
}
