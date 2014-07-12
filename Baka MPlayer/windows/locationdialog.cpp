#include "locationdialog.h"
#include "ui_locationdialog.h"

#include <QClipboard>
#include <QRegExp>

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

void LocationDialog::on_urlEdit_textChanged(const QString &arg1)
{
    // todo: make more complex validation
#ifdef Q_OS_WIN
    QRegExp rx("^(http://.+\\.[a-z]+|C:/)", Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QRegExp rx("^(http://.+\\.[a-z]+|\\.{0,2}/)", Qt::CaseInsensitive);
#endif
    if(rx.indexIn(arg1) != -1)
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
