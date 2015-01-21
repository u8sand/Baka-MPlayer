#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "bakaengine.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"
#include "ui/keydialog.h"

#include <QFileDialog>
#include <QMessageBox>

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
    PopulateShortcuts();

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
                SelectKey(true);
            });

    connect(ui->changeKeyButton, &QPushButton::clicked,
            [=]
            {
                int i = ui->infoWidget->currentRow();
                SelectKey(false,
                    {ui->infoWidget->item(i, 0) != nullptr ?
                        ui->infoWidget->item(i, 0)->text() : QString(),
                    {ui->infoWidget->item(i, 1) != nullptr ?
                        ui->infoWidget->item(i, 1)->text() : QString(),
                     ui->infoWidget->item(i, 2) != nullptr ?
                        ui->infoWidget->item(i, 2)->text() : QString()}});
            });

    connect(ui->resetKeyButton, &QPushButton::clicked,
            [=]
            {
                if(QMessageBox::question(this, tr("Reset Keybinding"), tr("Are you sure you want to reset the shortcut keys to original bindings?")) == QMessageBox::Yes)
                {
                    baka->input = baka->default_input;
                    ui->infoWidget->clearContents();
                    PopulateShortcuts();
                }
            });

    connect(ui->removeKeyButton, &QPushButton::clicked,
            [=]
            {
                int row = ui->infoWidget->currentRow();
                if(row == -1)
                    return;

                if(ui->infoWidget->item(row, 0))
                {
                    baka->input[ui->infoWidget->item(row, 0)->text()] = {QString(), QString()};
                    ui->infoWidget->removeRow(row); // remove the row
                    for(int i = row+1; i < numberOfShortcuts-1; ++i) // push the rest of the rows up
                    {
                        ui->infoWidget->setItem(i-1, 0, ui->infoWidget->item(i, 0));
                        ui->infoWidget->setItem(i-1, 1, ui->infoWidget->item(i, 1));
                        ui->infoWidget->setItem(i-1, 2, ui->infoWidget->item(i, 2));
                    }
                    --numberOfShortcuts;
                }
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
    baka->window->MapShortcuts();
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

void PreferencesDialog::PopulateShortcuts()
{
    numberOfShortcuts = 0;
    for(auto iter = baka->input.begin(); iter != baka->input.end(); ++iter)
    {
        if(iter->first == QString() || iter->second == QString())
            continue;
        ui->infoWidget->insertRow(numberOfShortcuts);
        ui->infoWidget->setItem(numberOfShortcuts, 0, new QTableWidgetItem(iter.key()));
        ui->infoWidget->setItem(numberOfShortcuts, 1, new QTableWidgetItem(iter->first));
        ui->infoWidget->setItem(numberOfShortcuts, 2, new QTableWidgetItem(iter->second));
        ++numberOfShortcuts;
    }
}

void PreferencesDialog::SelectKey(bool add, QPair<QString, QPair<QString, QString>> init)
{
    KeyDialog dialog(this);
    int status = 0;
    while(status != 2)
    {
        QPair<QString, QPair<QString, QString>> result = dialog.SelectKey(add, init);
        if(result == QPair<QString, QPair<QString, QString>>()) // cancel
            return;
        for(int i = 0; i < numberOfShortcuts; ++i)
            if(ui->infoWidget->item(i, 0) && ui->infoWidget->item(i, 0)->text() == result.first)
            {
                if(QMessageBox::question(&dialog,
                       tr("Existing keybinding"),
                       tr("%0 is already being used. Would you like to change its function?").arg(
                           result.first)) == QMessageBox::Yes)
                {
                    ui->infoWidget->item(i, 1)->setText(result.second.first);
                    ui->infoWidget->item(i, 2)->setText(result.second.second);
                    baka->input[result.first] = result.second;
                    status = 2;
                }
                else
                {
                    init = result;
                    status = 1;
                }
                break;
            }
        if(status == 0)
        {
            ui->infoWidget->insertRow(numberOfShortcuts);
            ui->infoWidget->setItem(numberOfShortcuts, 0, new QTableWidgetItem(result.first));
            ui->infoWidget->setItem(numberOfShortcuts, 1, new QTableWidgetItem(result.second.first));
            ui->infoWidget->setItem(numberOfShortcuts, 2, new QTableWidgetItem(result.second.second));
            baka->input[result.first] = result.second;
            ++numberOfShortcuts;
        }
    }
}
