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

    PopulateLangs();

    settings->beginGroup("baka-mplayer");
    QString ontop = settings->value("onTop").toString();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(settings->value("trayIcon").toBool());
    ui->hidePopupCheckBox->setChecked(settings->value("hidePopup").toBool());
    ui->gestureCheckBox->setChecked(settings->value("gestures").toBool());
    ui->langComboBox->setCurrentText(settings->value("lang").toString());

    int autofit = settings->value("autoFit").toInt();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    settings->endGroup();
    ui->formatComboBox->setCurrentText(settings->value("mpv/screenshot-format").toString());

    QString screenshotTemplate = settings->value("mpv/screenshot-template", tr("screenshot%#04n")).toString();
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
    settings->setValue("trayIcon", ui->groupBox_2->isChecked());
    settings->setValue("hidePopup", ui->hidePopupCheckBox->isChecked());
    settings->setValue("gestures", ui->gestureCheckBox->isChecked());
    settings->setValue("lang", ui->langComboBox->currentText());

    if(ui->autoFitCheckBox->isChecked())
        settings->setValue("autoFit", ui->comboBox->currentText().left(ui->comboBox->currentText().length()-1).toInt());
    else
        settings->setValue("autoFit", 0);
    settings->endGroup();
    settings->setValue("mpv/screenshot-format", ui->formatComboBox->currentText());
    settings->setValue("mpv/screenshot-template", screenshotDir+"/"+ui->templateLineEdit->text());

    delete ui;
}

void PreferencesDialog::showPreferences(QSettings *settings, QWidget *parent)
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
        // right 5: xx.qm
        // left 2:  xx
        ui->langComboBox->addItem(i.fileName().mid(13,i.fileName().length()-16));
    }
}
