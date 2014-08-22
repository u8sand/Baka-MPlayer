#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

#include "mpvhandler.h"

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(const Mpv::FileInfo &fileInfo, QWidget *parent = 0);
    ~InfoDialog();

    static void info(const Mpv::FileInfo &fileInfo, QWidget *parent = 0);
private:
    Ui::InfoDialog *ui;
    const Mpv::FileInfo &fileInfo;
};

#endif // INFODIALOG_H
