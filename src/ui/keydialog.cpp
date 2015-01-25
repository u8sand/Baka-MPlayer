#include "keydialog.h"
#include "ui_keydialog.h"

KeyDialog::KeyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyDialog)
{
    ui->setupUi(this);
    connect(ui->keySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            [=](const QKeySequence&)
            {
                SetButtons();
            });

    connect(ui->commandLineEdit, &QLineEdit::textChanged,
            [=](const QString&)
            {
                SetButtons();
            });

    connect(ui->clearButton, &QPushButton::pressed,
            [=]
            {
                ui->keySequenceEdit->clear();
            });

    connect(ui->addButton, SIGNAL(pressed()),
            this, SLOT(accept()));
    connect(ui->changeButton, SIGNAL(pressed()),
            this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(pressed()),
            this, SLOT(reject()));
}

KeyDialog::~KeyDialog()
{
    delete ui;
}

QPair<QString, QPair<QString, QString>> KeyDialog::SelectKey(bool add, QPair<QString, QPair<QString, QString>> init)
{
    this->add = add;
    ui->keySequenceEdit->setKeySequence(QKeySequence(init.first));
    ui->commandLineEdit->setText(init.second.first);
    ui->labelLineEdit->setText(init.second.second);
    SetButtons();
    if(exec() == QDialog::Rejected)
        return QPair<QString, QPair<QString, QString>>();
    return QPair<QString, QPair<QString, QString>>({
        ui->keySequenceEdit->keySequence().toString(),
        {ui->commandLineEdit->text(),
         ui->labelLineEdit->text()}
    });
}

void KeyDialog::SetButtons()
{
    bool enabled =
        (!ui->keySequenceEdit->keySequence().isEmpty() &&
         !ui->commandLineEdit->text().isEmpty());

    if(add)
    {
        ui->changeButton->setVisible(false);
        ui->addButton->setVisible(true);
        ui->addButton->setEnabled(enabled);
    }
    else
    {
        ui->addButton->setVisible(false);
        ui->changeButton->setVisible(true);
        ui->changeButton->setEnabled(enabled);
    }
}
