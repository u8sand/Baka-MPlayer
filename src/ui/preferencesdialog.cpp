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
    QString ontop = settings->value("window/onTop").toString();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(settings->value("window/trayIcon").toBool());
    ui->hidePopupCheckBox->setChecked(settings->value("window/hidePopup").toBool());
    int autofit = settings->value("window/autoFit").toInt();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    ui->formatComboBox->setCurrentText(settings->value("mpv/screenshotFormat").toString());
    ui->templateLineEdit->setText(settings->value("mpv/screenshotTemplate").toString());

    // propigate changes to data
    connect(ui->neverRadioButton, &QRadioButton::clicked,
            [=]()
            {
                settings->setValue("window/onTop", "never");
            });
    connect(ui->playingRadioButton, &QRadioButton::clicked,
            [=]()
            {
                settings->setValue("window/onTop", "playing");
            });
    connect(ui->alwaysRadioButton, &QRadioButton::clicked,
            [=]()
            {
                settings->setValue("window/onTop", "always");
            });
    connect(ui->groupBox_2, &QGroupBox::clicked,
            [=](bool b)
            {
                settings->setValue("window/trayIcon", b);
            });
    connect(ui->hidePopupCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                settings->setValue("window/hidePopup", b);
            });
    connect(ui->comboBox, &QComboBox::currentTextChanged,
            [=](QString s)
            {
                s.chop(1);
                settings->setValue("window/autoFit", s.toInt());
            });
    connect(ui->autoFitCheckBox, &QCheckBox::clicked,
            [=](bool b)
            {
                if(!b)
                {
                    ui->comboBox->setEnabled(false);
                    settings->setValue("window/autoFit", 0);
                }
                else
                {
                    ui->comboBox->setEnabled(true);
                    QString autofit = ui->comboBox->currentText();
                    autofit.chop(1);
                    settings->setValue("window/autoFit", autofit.toInt());
                }
            });
    connect(ui->formatComboBox, &QComboBox::currentTextChanged,
            [=](QString s)
            {
                settings->setValue("mpv/screenshotFormat", s);
            });
    connect(ui->templateLineEdit, &QLineEdit::textChanged,
            [=](QString s)
            {
                // todo: template should be prefixed with directory
                settings->setValue("mpv/screenshotTemplate", s);
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
    // todo: template should be prefixed with directory
    QString dir = QFileDialog::getExistingDirectory(this, "Choose screenshot directory", settings->value("mpv/screenshotDir").toString());
    if(dir.length() > 0)
        settings->setValue("mpv/screenshotDir", dir);
}
