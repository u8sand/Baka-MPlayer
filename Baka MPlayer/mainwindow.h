#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QAction>

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
    virtual bool event(QEvent *event);
    bool HandleMpvEvent(MpvHandler::MpvEvent event);

    inline void SetTimeLabels();
    inline void SetSeekBar();
    inline void SetPlayButton();
    inline void EnableControls();

private slots:
    void on_action_Open_File_triggered();
    void on_actionE_xit_triggered();
    void on_openButton_clicked();
    void on_playButton_clicked();
    void on_rewindButton_clicked();
    void on_previousButton_clicked();
    void on_nextButton_clicked();
    void on_volumeSlider_sliderMoved(int position);
    void on_seekBar_sliderMoved(int position);
    void on_seekBar_sliderPressed();
    void on_playlistButton_clicked();


    void on_action_Play_triggered();

private:
    Ui::MainWindow *ui;
    SettingsManager *settings;
    MpvHandler *mpv;
//    PlaylistManager *playlist;

    QSlider *seekBar;
    QLabel *durationLabel,
           *remainingLabel;
    QPushButton *playButton,
                *rewindButton,
                *previousButton,
                *nextButton;
    QListWidget *playlistWidget;
    QAction     *playAction;
};

#endif // MAINWINDOW_H
