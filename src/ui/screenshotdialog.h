#ifndef SCREENSHOTDIALOG_H
#define SCREENSHOTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ScreenshotDialog;
}

class ScreenshotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenshotDialog(QString t, QWidget *parent = 0);
    ~ScreenshotDialog();

    static QString showScreenshotDialog(QString t, QWidget *parent = 0);
private:
    Ui::ScreenshotDialog *ui;
};

#endif // SCREENSHOTDIALOG_H
