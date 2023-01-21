#include "screenshotdialog.h"
#include "ui_screenshotdialog.h"

#include <QFileDialog>
#include <QString>

ScreenshotDialog::ScreenshotDialog(bool &_always, bool &_screenshot, MpvHandler *mpv, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenshotDialog),
    always(_always),
    screenshot(_screenshot)
{
    ui->setupUi(this);

    ui->showCheckBox->setChecked(always);
    ui->subtitlesCheckBox->setChecked(screenshot);
    ui->templateEdit->setText(mpv->getScreenshotTemplate());
    ui->formatComboBox->setCurrentText(mpv->getScreenshotFormat());

    ui->locationEdit->setText(QDir::toNativeSeparators(mpv->getScreenshotDir()));
    ui->templateEdit->setText(mpv->getScreenshotTemplate());

    connect(ui->browseButton, &QPushButton::clicked,
            this, [=]
            {
                QString dir = QFileDialog::getExistingDirectory(this, tr("Choose screenshot directory"), ui->locationEdit->text());
                if(dir != QString())
                    ui->locationEdit->setText(dir);
            });

    connect(ui->saveButton, &QPushButton::clicked,
            this, [=]
            {
                mpv->ScreenshotFormat(ui->formatComboBox->currentText());
                mpv->ScreenshotDirectory(QDir::fromNativeSeparators(ui->locationEdit->text()));
                mpv->ScreenshotTemplate(ui->templateEdit->text());
                always = ui->showCheckBox->isChecked();
                screenshot = ui->subtitlesCheckBox->isChecked();
                mpv->Screenshot(screenshot);
                accept();
            });
}

ScreenshotDialog::~ScreenshotDialog()
{
    delete ui;
}

int ScreenshotDialog::showScreenshotDialog(bool &always, bool &screenshot, MpvHandler *mpv, QWidget *parent)
{
    ScreenshotDialog dialog(always, screenshot, mpv, parent);
    return dialog.exec();
}
