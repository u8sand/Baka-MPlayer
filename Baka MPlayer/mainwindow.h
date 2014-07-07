#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLabel>

#include "mpvhandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual bool event(QEvent *event);
    bool HandleMpvEvent(MpvHandler::MpvEvent event);

private slots:
    void on_action_Open_File_triggered();

    void on_actionE_xit_triggered();

    void on_openButton_clicked();

    void on_playButton_clicked();

    void on_rewindButton_clicked();

    void on_previousButton_clicked();

    void on_nextButton_clicked();

private:
    Ui::MainWindow *ui;
    MpvHandler *mpv;

    QSlider *seekBar;
    QLabel *durationLabel,
           *remainingLabel;
};

#endif // MAINWINDOW_H
