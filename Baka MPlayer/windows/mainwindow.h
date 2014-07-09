#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settingsmanager.h"
#include "mpvhandler.h"
//#include "playlistmanager.h"

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
    inline void EnableControls();

private slots:
    void OpenFile();

    void SetTime(time_t time);
    void SetControls(bool enable);
    void SetPlayState(Mpv::PlayState state);

    void HandleError(QString error);

    // todo: convert the rest of these to signals
    void on_rewindButton_clicked();
    void on_previousButton_clicked();
    void on_nextButton_clicked();
    void on_seekBar_sliderMoved(int position);
    void on_seekBar_sliderPressed();
    void on_playlistButton_clicked();
private:
    Ui::MainWindow *ui;
    SettingsManager *settings;
//    PlaylistManager *playlist;
};

#endif // MAINWINDOW_H
