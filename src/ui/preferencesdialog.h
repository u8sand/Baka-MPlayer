#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QString>
#include <QPair>

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
    void PopulateShortcuts();
    void AddRow(QString first, QString second, QString third);
    void ModifyRow(int row, QString first, QString second, QString third);
    void RemoveRow(int row);
    void SelectKey(bool add, QPair<QString, QPair<QString, QString>> init = (QPair<QString, QPair<QString, QString>>()));

private:
    Ui::PreferencesDialog *ui;
    BakaEngine *baka;
    QHash<QString, QPair<QString, QString>> saved;

    QString screenshotDir;
    int numberOfShortcuts;
};

#endif // PREFERENCESDIALOG_H
