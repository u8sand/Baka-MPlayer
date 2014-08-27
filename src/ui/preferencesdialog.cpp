#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QSettings *_settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    settings(_settings)
{
    ui->setupUi(this);

    connect(ui->changeButton, SIGNAL(clicked()),
            this, SLOT(ChangeScreenshotLocation()));
    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    // populate existing preferences
    QString ontop = settings->value("window/ontop").toString();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(settings->value("window/trayIcon").toBool());
    ui->hidePopupCheckBox->setChecked(settings->value("window/hidepopup").toBool());
    int autofit = settings->value("window/autofit").toInt();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    ui->formatComboBox->setCurrentText(settings->value("mpv/screenshot_format").toString());
    ui->templateLineEdit->setText(settings->value("mpv/screenshot_template").toString());

    // todo: setup combo boxes
    // todo: propigate changes instantaniously?
}

PreferencesDialog::~PreferencesDialog()
{
    // save preferences
    if(ui->neverRadioButton->isChecked())
        settings->setValue("window/ontop", "never");
    else if(ui->playingRadioButton->isChecked())
        settings->setValue("window/ontop", "playing");
    else if(ui->alwaysRadioButton->isChecked())
        settings->setValue("window/ontop", "always");
    settings->setValue("window/trayicon", ui->groupBox_2->isChecked());
    settings->setValue("window/hidepopup", ui->hidePopupCheckBox->isChecked());
    QString autofit = ui->comboBox->currentText();
    autofit.chop(1); // remove the '%'
    settings->setValue("window/autofit", ui->autoFitCheckBox->isChecked() ? autofit.toInt() : 0);
    settings->setValue("mpv/screenshot_format", ui->formatComboBox->currentText());
    settings->setValue("mpv/screenshot_template", ui->templateLineEdit->text());

    delete ui;
}

void PreferencesDialog::showPreferences(QSettings *settings, QWidget *parent)
{
    PreferencesDialog dialog(settings, parent);
    dialog.exec();
}

void PreferencesDialog::ChangeScreenshotLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Choose screenshot directory", settings->value("mpv/screenshot_dir").toString());
    if(dir.length() > 0)
        settings->setValue("mpv/screenshot_dir", dir);
}
