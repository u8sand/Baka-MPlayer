#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "bakaengine.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(BakaEngine *baka, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    baka(baka),
    screenshotDir("")
{
    ui->setupUi(this);

    PopulateLangs();

    QString ontop = baka->window->getOnTop();
    if(ontop == "never")
        ui->neverRadioButton->setChecked(true);
    else if(ontop == "playing")
        ui->playingRadioButton->setChecked(true);
    else if(ontop == "always")
        ui->alwaysRadioButton->setChecked(true);
    ui->groupBox_2->setChecked(baka->sysTrayIcon->isVisible());
    ui->hidePopupCheckBox->setChecked(baka->window->getHidePopup());
    ui->gestureCheckBox->setChecked(baka->window->getGestures());
    ui->langComboBox->setCurrentText(baka->window->getLang());
    int autofit = baka->window->getAutoFit();
    ui->autoFitCheckBox->setChecked((bool)autofit);
    ui->comboBox->setCurrentText(QString::number(autofit)+"%");
    ui->formatComboBox->setCurrentText(baka->mpv->getScreenshotFormat());
    screenshotDir = QDir::toNativeSeparators(baka->mpv->getScreenshotDir());
    ui->templateLineEdit->setText(baka->mpv->getScreenshotTemplate());

    // add shortcuts
    numberOfShortcuts = 0;
    for(auto iter = baka->input.begin(); iter != baka->input.end(); ++iter)
    {
        if(iter->first == QString())
            continue;
        ui->infoWidget->insertRow(numberOfShortcuts);
        ui->infoWidget->setItem(numberOfShortcuts, 0, new QTableWidgetItem(iter.key()));
        ui->infoWidget->setItem(numberOfShortcuts, 1, new QTableWidgetItem(iter->first));
        ui->infoWidget->setItem(numberOfShortcuts, 2, new QTableWidgetItem(iter->second));
        ++numberOfShortcuts;
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

    connect(ui->addKeyButton, &QPushButton::clicked,
            [=]
            {
                // todo: disallow duplicate shortcuts
                ui->infoWidget->insertRow(numberOfShortcuts);
                ui->infoWidget->setItem(numberOfShortcuts, 0, new QTableWidgetItem(ui->keySequenceEdit->keySequence().toString()));
                ui->infoWidget->setItem(numberOfShortcuts, 1, new QTableWidgetItem(ui->lineEdit->text()));
                ui->infoWidget->setItem(numberOfShortcuts, 2, new QTableWidgetItem(ui->lineEdit_2->text()));
                ++numberOfShortcuts;
            });

    connect(ui->removeKeyButton, &QPushButton::clicked,
            [=]
            {
                // todo: deal with removing default entries
                int row = ui->infoWidget->currentRow();
                if(row == -1)
                    return;
                ui->infoWidget->removeRow(row); // remove the row
                // push the rest of the rows up
                for(int i = row+1; i < numberOfShortcuts-1; ++i)
                {
                    ui->infoWidget->setItem(i-1, 0, ui->infoWidget->item(i, 0));
                    ui->infoWidget->setItem(i-1, 1, ui->infoWidget->item(i, 1));
                    ui->infoWidget->setItem(i-1, 2, ui->infoWidget->item(i, 2));
                }
                --numberOfShortcuts;
            });

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

PreferencesDialog::~PreferencesDialog()
{
    if(ui->neverRadioButton->isChecked())
        baka->window->setOnTop("never");
    else if(ui->playingRadioButton->isChecked())
        baka->window->setOnTop("playing");
    else if(ui->alwaysRadioButton->isChecked())
        baka->window->setOnTop("always");
    baka->sysTrayIcon->setVisible(ui->groupBox_2->isChecked());
    baka->window->setHidePopup(ui->hidePopupCheckBox->isChecked());
    baka->window->setGestures(ui->gestureCheckBox->isChecked());
    baka->window->setLang(ui->langComboBox->currentText());
    if(ui->autoFitCheckBox->isChecked())
        baka->window->setAutoFit(ui->comboBox->currentText().left(ui->comboBox->currentText().length()-1).toInt());
    else
        baka->window->setAutoFit(0);
    baka->mpv->ScreenshotFormat(ui->formatComboBox->currentText());
    baka->mpv->ScreenshotDirectory(screenshotDir);
    baka->mpv->ScreenshotTemplate(ui->templateLineEdit->text());
    baka->input.clear();
    for(int i = 0; i < numberOfShortcuts; i++)
        if(ui->infoWidget->item(i, 0) && ui->infoWidget->item(i, 1) && ui->infoWidget->item(i, 0)->text() != QString())
        {
            QPair<QString, QString> pair;
            pair.first = ui->infoWidget->item(i, 1)->text();
            if(ui->infoWidget->item(i, 2))
                pair.second = ui->infoWidget->item(i, 2)->text();
            baka->input[ui->infoWidget->item(i, 0)->text()] = pair;
        }

    delete ui;
}

void PreferencesDialog::showPreferences(BakaEngine *baka, QWidget *parent)
{
    PreferencesDialog dialog(baka, parent);
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
