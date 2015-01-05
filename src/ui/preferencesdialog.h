#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include "settings.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(Settings *settings, QWidget *parent = 0);
    ~PreferencesDialog();

    static void showPreferences(Settings *settings, QWidget *parent = 0);

protected:
    void PopulateLangs();

private:
    Ui::PreferencesDialog *ui;
    Settings *settings;
    QString screenshotDir;
    int numberOfShortcuts;
};

#endif // PREFERENCESDIALOG_H
