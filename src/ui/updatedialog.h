#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QElapsedTimer>

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

protected slots:
    void ShowInfo();

private:
    Ui::UpdateDialog *ui;
    BakaEngine *baka;

    QElapsedTimer *timer;
    double avgSpeed = 1,
           lastSpeed = 0;
    int lastProgress,
        lastTime,
        state;
    bool init;
};

#endif // UPDATEDIALOG_H
