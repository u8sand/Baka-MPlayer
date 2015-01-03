#include "locationdialog.h"
#include "ui_locationdialog.h"

#include <QClipboard>
#include <QDir>

#include "platform.h"

LocationDialog::LocationDialog(QString path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationDialog)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()),    // ok button clicked
            this, SLOT(accept()));              // accept
    connect(ui->cancelButton, SIGNAL(clicked()),// cancel button clicked
            this, SLOT(reject()));              // reject
    connect(ui->urlEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validate(QString)));

    if(Platform::IsValidFile(path))
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

void LocationDialog::on_pasteButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    ui->urlEdit->setText(clipboard->text());
}

void LocationDialog::on_copyButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->urlEdit->text());
}

void LocationDialog::on_clearButton_clicked()
{
    ui->urlEdit->setText("");
}

void LocationDialog::validate(QString input)
{
    if(Platform::IsValidLocation(input))
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
