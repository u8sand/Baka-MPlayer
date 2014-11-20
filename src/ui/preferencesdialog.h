#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QSettings *settings, QWidget *parent = 0);
    ~PreferencesDialog();

    static void showPreferences(QSettings *settings, QWidget *parent = 0);

private:
    Ui::PreferencesDialog *ui;
    QSettings *settings;
    QString screenshotDir;
};

#endif // PREFERENCESDIALOG_H
