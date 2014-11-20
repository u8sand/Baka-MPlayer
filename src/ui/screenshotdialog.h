#ifndef SCREENSHOTDIALOG_H
#define SCREENSHOTDIALOG_H

#include <QDialog>
#include <QString>

#include "mpvhandler.h"

namespace Ui {
class ScreenshotDialog;
}

class ScreenshotDialog : public QDialog
{
    Q_OBJECT

public://ScreenshotDialog::showDialog(screenshotDialog, true, mpv);
    explicit ScreenshotDialog(bool &always, bool screenshot, MpvHandler *mpv, QWidget *parent = 0);
    ~ScreenshotDialog();

    static void showScreenshotDialog(bool &always, bool screenshot, MpvHandler *mpv, QWidget *parent = 0);
private:
    Ui::ScreenshotDialog *ui;
    bool &always;
};

#endif // SCREENSHOTDIALOG_H
