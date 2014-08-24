#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QModelIndex>
#include <QStringList>
#include <QMouseEvent>
#include <QPoint>

#include "mpvhandler.h"
#include "playlistmanager.h"
#include "lightdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QSettings *settings, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void HandleError(QString err);                  // triggered by any aspect to display an error message

protected:
    void dragEnterEvent(QDragEnterEvent *event);    // drag file into
    void dropEvent(QDropEvent *event);              // drop file into
    void mousePressEvent(QMouseEvent *event);       // pressed mouse down
    void mouseReleaseEvent(QMouseEvent *event);     // released mouse up
    void mouseMoveEvent(QMouseEvent *event);        // moved mouse on the form
    void mouseDoubleClickEvent(QMouseEvent *event); // double clicked the form

    inline void SetPlaybackControls(bool enable);   // macro to enable/disable playback controls

private slots:
    QString FormatTime(int time);                   // format the time the way we want to display it
    void SetTime(int time);                         // triggered by mpv, updates time labels and seekbar
    void SetPlayState(Mpv::PlayState state);        // triggered by mpv, updates controls based on changes of playstate
    void Seek(int position);                        // converts seekbar location to mpv->Seek command
    void PlayPause();                               // mpv playpause
    void Rewind();                                  // rewind macro, goes back to beginning or stops playback
    void NewPlayer();                               // creates a new MainWindow
    void OpenFile();                                // open file dialog
    void OpenUrl();                                 // open url dialog
    void FullScreen(bool fs);                       // makes window fullscreen
    void BossMode();                                // run away from the boss
    void JumpToTime();                              // jump to time dialog
    void MediaInfo();                               // media info dialog
    void OpenFileFromClipboard();                   // opens file from cliboard text
    void OpenLastFile();                            // opens last open file (from settings)
    void ShowInFolder();                            // opens explorer to current file's path
    void UpdatePlaylist(QStringList list);          // updates the playlistWidget with the new playlist
    void UpdatePlaylistIndex(int index);            // updates the playlistWidget selection
    void UpdatePlaylistSelectionIndex(int index);   // updates the indexLabel of the playlist
    void PlaylistSelectCurrent();                   // selects the current playlist index
    void PlayIndex(QModelIndex index);              // plays the selected file in the playlist
    void SetPlaylist(bool visible);                 // sets the playlist visibility
    void TogglePlaylist();                          // toggle playlist
    void GetPlaylistIndex();                        // selects the file index to play
    void HideAlbumArt(bool show);                   // sets album art visibility (mpvFrame)
    void Debug(QString msg);                        // outputs debugging messages
    void OnlineHelp();                              // loads online help
    void CheckForUpdates();                         // checks for program updates
    void AboutQt();                                 // shows qt license information
    void About();                                   // shows baka mplayer about dialog
    void SeekForward();                             // seek +5
    void SeekBack();                                // seek -5
    void AddSubtitleTrack();                        // add a new external subtitle track
    void FitWindow(double percent);                 // fit the window the the specified percent
    void SetAspectRatio(double ratio);              // set the aspect ratio to specified proportions
    void IncreaseFontSize();                        // increase the subtitle font size
    void DecreaseFontSize();                        // decrease the subtitle font size
    void ResetFontSize();                           // reset the subtitle font size
    void DimLights(bool dim);                       // grays out the rest of the screen with LightDialog
    void IncreaseVolume();                          // increases the volume by a set amount
    void DecreaseVolume();                          // decreases the volume by a set amount
    void ShowInTray(bool show);                     // sets visibility of tray icon
    void HidePopup(bool hide);                      // enables/disables the tray popups

#ifdef Q_OS_WIN
    void SetAlwaysOnTop(bool ontop);
    void AlwaysOnTop(bool ontop);
    void AlwaysOnTopWhenPlaying(bool ontop);
    void NeverOnTop(bool ontop);
#endif

private:
    Ui::MainWindow *ui;
    QSettings *settings;
    MpvHandler *mpv;
    PlaylistManager *playlist;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    LightDialog *light;
    bool dragging;
    QPoint lastMousePos;
};

#endif // MAINWINDOW_H
