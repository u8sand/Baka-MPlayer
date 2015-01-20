#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QTime>


namespace Ui {
class UpdateDialog;
}

class BakaEngine;

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(BakaEngine *baka, QWidget *parent = 0);
    ~UpdateDialog();

    static void CheckForUpdates(BakaEngine *baka, QWidget *parent = 0);

public slots:

private:
    Ui::UpdateDialog *ui;
    BakaEngine *baka;

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
