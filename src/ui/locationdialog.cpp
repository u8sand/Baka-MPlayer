#include "locationdialog.h"
#include "ui_locationdialog.h"

#include <QClipboard>
#include <QRegExp>
#include <QDir>

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
    ui->urlEdit->setText(QDir::toNativeSeparators(path));
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
#ifdef Q_OS_WIN
    QRegExp rx("^(https?://.+\\.[a-z]+|[a-z]:\\\\)", Qt::CaseInsensitive);
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QRegExp rx("^(https?://.+\\.[a-z]+|\\.{0,2}/)", Qt::CaseInsensitive);
#endif
    if(rx.indexIn(input) != -1)
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
