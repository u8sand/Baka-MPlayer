#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(Settings *_settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    settings(_settings),
    screenshotDir("")
{
    ui->setupUi(this);

    PopulateLangs();

    settings->beginGroup("baka-mplayer");
    QString ontop = settings->value("onTop");
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(settings->valueBool("trayIcon"));
    ui->hidePopupCheckBox->setChecked(settings->valueBool("hidePopup"));
    ui->gestureCheckBox->setChecked(settings->valueBool("gestures"));
    ui->langComboBox->setCurrentText(settings->value("lang"));

    int autofit = settings->valueInt("autoFit");
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    settings->endGroup();
    settings->beginGroup("mpv");
    ui->formatComboBox->setCurrentText(settings->value("screenshot-format"));

    QString screenshotTemplate = settings->value("screenshot-template", tr("screenshot%#04n"));
    settings->endGroup();
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
                QString dir = QFileDialog::getExistingDirectory(this, tr("Choose screenshot directory"), screenshotDir);
                if(dir != QString())
                    screenshotDir = dir;
            });

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

PreferencesDialog::~PreferencesDialog()
{
    settings->beginGroup("baka-mplayer");
    if(ui->neverRadioButton->isChecked())
        settings->setValue("onTop", "never");
    else if(ui->playingRadioButton->isChecked())
        settings->setValue("onTop", "playing");
    else if(ui->alwaysRadioButton->isChecked())
        settings->setValue("onTop", "always");
    settings->setValueBool("trayIcon", ui->groupBox_2->isChecked());
    settings->setValueBool("hidePopup", ui->hidePopupCheckBox->isChecked());
    settings->setValueBool("gestures", ui->gestureCheckBox->isChecked());
    settings->setValue("lang", ui->langComboBox->currentText());

    if(ui->autoFitCheckBox->isChecked())
        settings->setValueInt("autoFit", ui->comboBox->currentText().left(ui->comboBox->currentText().length()-1).toInt());
    else
        settings->setValueInt("autoFit", 0);
    settings->endGroup();
    settings->beginGroup("mpv");
    settings->setValue("screenshot-format", ui->formatComboBox->currentText());
    settings->setValue("screenshot-template", screenshotDir+"/"+ui->templateLineEdit->text());
    settings->endGroup();
    settings->Save();

    delete ui;
}

void PreferencesDialog::showPreferences(Settings *settings, QWidget *parent)
{
    PreferencesDialog dialog(settings, parent);
    dialog.exec();
}

void PreferencesDialog::PopulateLangs()
{
    // open the language directory
    QDir root(BAKA_MPLAYER_LANG_PATH);
    // get files in the directory with .qm extension
    QFileInfoList flist;
    flist = root.entryInfoList({"*.qm"}, QDir::Files);
    // add the languages to the combo box
    ui->langComboBox->addItem("auto");
    for(auto &i : flist)
    {
        QString lang = i.fileName().mid(i.fileName().indexOf("_") + 1); // baka-mplayer_....
        lang.chop(3); // -  .qm
        ui->langComboBox->addItem(lang);
    }
}
