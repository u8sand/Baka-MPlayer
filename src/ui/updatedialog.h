#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QTime>

#include "updatemanager.h"

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(UpdateManager *update, QWidget *parent = 0);
    ~UpdateDialog();

    static int update(UpdateManager *update, QWidget *parent = 0);

private:
    Ui::UpdateDialog *ui;

    QTime *timer;
    double avgSpeed = 1,
           lastSpeed=0;
    int lastProgress,
        lastTime;
#if defined(Q_OS_WIN)
    QString version,
            url;
#endif
};

#endif // UPDATEDIALOG_H
