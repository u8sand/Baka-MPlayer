#include "locationdialog.h"
#include "ui_locationdialog.h"

#include <QClipboard>

LocationDialog::LocationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationDialog)
{
    ui->setupUi(this);
}

LocationDialog::~LocationDialog()
{
    delete ui;
}

void LocationDialog::on_okButton_clicked()
{
    emit Done(ui->urlEdit->text());
    close();
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

void LocationDialog::on_cancelButton_clicked()
{
    emit Done();
    close();
}
