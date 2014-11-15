#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QSettings *_settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    settings(_settings),
    screenshotTemplate(""),
    screenshotDir("")
{
    ui->setupUi(this);

    connect(ui->changeButton, SIGNAL(clicked()),
            this, SLOT(ChangeScreenshotLocation()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

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
    screenshotDir = settings->value("mpv/screenshot-template", "./screenshot%#04n").toString();
    int i = screenshotDir.lastIndexOf('/');
    if(i != -1)
    {
        screenshotTemplate = screenshotDir.remove(0, i+1);
        screenshotDir.truncate(i);
    }
    else
    {
        screenshotTemplate = screenshotDir;
        screenshotDir = ".";
    }
    ui->templateLineEdit->setText(screenshotTemplate);

    // propigate changes to data
    connect(ui->neverRadioButton, &QRadioButton::clicked,
            [=]
            {
                settings->setValue("baka-mplayer/onTop", "never");
            });
    connect(ui->playingRadioButton, &QRadioButton::clicked,
            [=]
            {
                settings->setValue("baka-mplayer/onTop", "playing");
            });
    connect(ui->alwaysRadioButton, &QRadioButton::clicked,
            [=]
            {
                settings->setValue("baka-mplayer/onTop", "always");
            });
    connect(ui->groupBox_2, &QGroupBox::clicked,
            [=](bool b)
            {
                settings->setValue("baka-mplayer/trayIcon", b);
            });
    connect(ui->hidePopupCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                settings->setValue("baka-mplayer/hidePopup", b);
            });
    connect(ui->comboBox, &QComboBox::currentTextChanged,
            [=](QString s)
            {
                s.chop(1);
                settings->setValue("baka-mplayer/autoFit", s.toInt());
            });
    connect(ui->autoFitCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                if(!b)
                {
                    ui->comboBox->setEnabled(false);
                    settings->setValue("baka-mplayer/autoFit", 0);
                }
                else
                {
                    ui->comboBox->setEnabled(true);
                    QString autofit = ui->comboBox->currentText();
                    autofit.chop(1);
                    settings->setValue("baka-mplayer/autoFit", autofit.toInt());
                }
            });
    connect(ui->formatComboBox, &QComboBox::currentTextChanged,
            [=](QString s)
            {
                settings->setValue("mpv/screenshot-format", s);
            });
    connect(ui->templateLineEdit, &QLineEdit::textChanged,
            [=](QString s)
            {
                screenshotTemplate = s;
                settings->setValue("mpv/screenshot-template", screenshotDir+"/"+screenshotTemplate);
            });
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::showPreferences(QSettings *settings, QWidget *parent)
{
    PreferencesDialog dialog(settings, parent);
    dialog.exec();
}

void PreferencesDialog::ChangeScreenshotLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Choose screenshot directory", screenshotDir);
    if(dir != QString())
    {
        screenshotDir = dir;
        settings->setValue("mpv/screenshot-template", screenshotDir+"/"+screenshotTemplate);
    }
}
