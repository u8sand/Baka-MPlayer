#include "screenshotdialog.h"
#include "ui_screenshotdialog.h"

ScreenshotDialog::ScreenshotDialog(QString t, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenshotDialog)
{
    ui->setupUi(this);

    ui->templateEdit->setText(t);
}

ScreenshotDialog::~ScreenshotDialog()
{
    delete ui;
}

QString ScreenshotDialog::showScreenshotDialog(QString t, QWidget *parent)
{
    ScreenshotDialog dialog(t, parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.ui->templateEdit->text();
    return QString();
}
