#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>


namespace Ui {
class PreferencesDialog;
}

class BakaEngine;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(BakaEngine *baka, QWidget *parent = 0);
    ~PreferencesDialog();

    static void showPreferences(BakaEngine *baka, QWidget *parent = 0);

protected:
    void PopulateLangs();

private:
    Ui::PreferencesDialog *ui;
    BakaEngine *baka;

    QString screenshotDir;
    int numberOfShortcuts;
};

#endif // PREFERENCESDIALOG_H
