#include "locationdialog.h"
#include "ui_locationdialog.h"

#include <QClipboard>
#include <QDir>

#include "util.h"

LocationDialog::LocationDialog(QString path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(ui->urlEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validate(QString)));
    connect(ui->pasteButton, &QPushButton::clicked,
            [=]
            {
                ui->urlEdit->setText(QApplication::clipboard()->text());
            });

    if(Util::IsValidFile(path))
        ui->urlEdit->setText(QDir::toNativeSeparators(path));
    else
        ui->urlEdit->setText(path);
}

LocationDialog::~LocationDialog()
{
    delete ui;
}

QString LocationDialog::getUrl(QString path, QWidget *parent)
{
    LocationDialog dialog(path, parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.ui->urlEdit->text();
    else
        return QString();
}

void LocationDialog::validate(QString input)
{
    if(Util::IsValidLocation(input))
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
