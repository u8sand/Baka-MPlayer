#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "locationdialog.h"

#include "mpvhandler.h"
#include "settingsmanager.h"
#include "playlistmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void HandleError(QString err);

protected:
    inline void SetControls(bool enable);

//    virtual void dragEnterEvent(QDragEnterEvent *event);
//    virtual void dropEvent(QDropEvent *event);
private slots:
    void SetTime(time_t time);
    void SetPlayState(Mpv::PlayState state);
    void SetSeekBar(int position);
    void NewPlayer();
    void OpenFile();
    void OpenFileFromClipboard();
    void OpenLastFile();
    void Rewind();
    void TogglePlaylistVisibility();

private:
    Ui::MainWindow *ui;
    LocationDialog *locationDialog;

    SettingsManager *settings;
    MpvHandler *mpv;
    PlaylistManager *playlist;
};

#endif // MAINWINDOW_H
