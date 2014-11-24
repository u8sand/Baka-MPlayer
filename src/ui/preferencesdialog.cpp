#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QSettings *_settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    settings(_settings),
    screenshotDir("")
{
    ui->setupUi(this);

    // populate existing preferences
    QString ontop = settings->value("baka-mplayer/onTop").toString();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(settings->value("baka-mplayer/trayIcon").toBool());
    ui->hidePopupCheckBox->setChecked(settings->value("baka-mplayer/hidePopup").toBool());
    int autofit = settings->value("baka-mplayer/autoFit").toInt();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    ui->formatComboBox->setCurrentText(settings->value("mpv/screenshot-format").toString());

    QString screenshotTemplate = settings->value("mpv/screenshot-template", "./screenshot%#04n").toString();
    int i = screenshotTemplate.lastIndexOf('/');
    if(i != -1)
    {
        screenshotDir = screenshotTemplate.mid(0, i);
        ui->templateLineEdit->setText(screenshotTemplate.mid(i+1));
    }
    else
    {
        ui->templateLineEdit->setText(screenshotTemplate);
        screenshotDir = ".";
    }

    connect(ui->autoFitCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                ui->comboBox->setEnabled(b);
            });

    connect(ui->changeButton, &QPushButton::clicked,
            [=]
            {
                QString dir = QFileDialog::getExistingDirectory(this, "Choose screenshot directory", screenshotDir);
                if(dir != QString())
                    screenshotDir = dir;
            });

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

PreferencesDialog::~PreferencesDialog()
{
    if(ui->neverRadioButton->isChecked())
        settings->setValue("baka-mplayer/onTop", "never");
    else if(ui->playingRadioButton->isChecked())
        settings->setValue("baka-mplayer/onTop", "playing");
    else if(ui->alwaysRadioButton->isChecked())
        settings->setValue("baka-mplayer/onTop", "always");
    settings->setValue("baka-mplayer/trayIcon", ui->groupBox_2->isChecked());
    settings->setValue("baka-mplayer/hidePopup", ui->hidePopupCheckBox->isChecked());
    settings->setValue("baka-mplayer/autoFit", ui->comboBox->currentText().midRef(1).toInt());
    if(ui->autoFitCheckBox->isChecked())
        settings->setValue("baka-mplayer/autoFit", ui->comboBox->currentText().midRef(1).toInt());
    else
        settings->setValue("baka-mplayer/autoFit", 0);
    settings->setValue("mpv/screenshot-format", ui->formatComboBox->currentText());
    settings->setValue("mpv/screenshot-template", screenshotDir+"/"+ui->templateLineEdit->text());

    delete ui;
}

void PreferencesDialog::showPreferences(QSettings *settings, QWidget *parent)
{
    PreferencesDialog dialog(settings, parent);
    dialog.exec();
}
