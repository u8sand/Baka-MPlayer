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
    explicit UpdateDialog(QWidget *parent = 0);
    ~UpdateDialog();

    int exec();

public slots:
    void CheckForUpdates();

private:
    Ui::UpdateDialog *ui;
    UpdateManager *updateManager;

    QTime *timer;
    double avgSpeed = 1,
           lastSpeed=0;
    int lastProgress,
        lastTime;
    bool init;
#if defined(Q_OS_WIN)
    QString version,
            url;
#endif
};

#endif // UPDATEDIALOG_H
