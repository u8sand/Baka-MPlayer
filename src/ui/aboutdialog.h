#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QString>
#include <QScopedPointer>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QString version, QWidget *parent = 0);
    ~AboutDialog();

    static void about(QString version, QWidget *parent = 0);

private:
    QScopedPointer<Ui::AboutDialog> ui;
};

#endif // ABOUTDIALOG_H
