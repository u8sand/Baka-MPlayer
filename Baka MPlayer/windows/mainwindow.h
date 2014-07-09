#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "mpvhandler.h"
#include "settingsmanager.h"

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
    inline void SetTimeLabels();
    inline void SetSeekBar();
    inline void SetPlayButton();
    inline void SetControls(bool enable);

private slots:
    void OpenFile();

    void SetTime(time_t time);
    void SetPlayState(Mpv::PlayState state);

    void HandleError(QString error);

    // todo: convert the rest of these to signals
    void on_rewindButton_clicked();
    void on_previousButton_clicked();
    void on_nextButton_clicked();
    void on_seekBar_sliderMoved(int position);
    void on_seekBar_sliderPressed();
private:
    Ui::MainWindow *ui;
    SettingsManager *settings;
};

#endif // MAINWINDOW_H
