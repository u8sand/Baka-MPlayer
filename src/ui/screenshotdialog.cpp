#include "screenshotdialog.h"
#include "ui_screenshotdialog.h"

#include <QFileDialog>
#include <QString>

ScreenshotDialog::ScreenshotDialog(bool &_always, bool screenshot, MpvHandler *mpv, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenshotDialog),
    always(_always)
{
    ui->setupUi(this);

    ui->showCheckBox->setChecked(always);
    ui->subtitlesCheckBox->setChecked(screenshot);
    ui->templateEdit->setText(mpv->getScreenshotTemplate());
    ui->formatComboBox->setCurrentText(mpv->getScreenshotFormat());

    QString screenshotTemplate = mpv->getScreenshotTemplate();
    int i = screenshotTemplate.lastIndexOf('/');
    if(i != -1)
    {
        ui->locationEdit->setText(screenshotTemplate.mid(0, i));
        ui->templateEdit->setText(screenshotTemplate.mid(i+1));
    }
    else
    {
        ui->templateEdit->setText(screenshotTemplate);
        ui->locationEdit->setText(".");
    }

    connect(ui->browseButton, &QPushButton::clicked,
            [=]
            {
                QString dir = QFileDialog::getExistingDirectory(this, "Choose screenshot directory", ui->locationEdit->text());
                if(dir != QString())
                    ui->locationEdit->setText(dir);
            });

    connect(ui->saveButton, &QPushButton::clicked,
            [=]
            {
                mpv->ScreenshotFormat(ui->formatComboBox->currentText());
                mpv->ScreenshotTemplate(ui->locationEdit->text()+"/"+ui->templateEdit->text());
                always = ui->showCheckBox->isChecked();
                mpv->Screenshot(ui->subtitlesCheckBox->isChecked());
                accept();
            });
}

ScreenshotDialog::~ScreenshotDialog()
{
    delete ui;
}

void ScreenshotDialog::showScreenshotDialog(bool &always, bool screenshot, MpvHandler *mpv, QWidget *parent)
{
    ScreenshotDialog dialog(always, screenshot, mpv, parent);
    dialog.exec();
}
