#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSignalMapper>
#include <QSettings>
#include <QModelIndex>
#include <QStringList>
#include <QMouseEvent>
#include <QEvent>
#include <QPoint>
#include <QTimer>

#include "mpvhandler.h"
#include "updatemanager.h"
#include "widgets/dimdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Load(QString f = QString());

protected:
    void LoadSettings();
    void SaveSettings();

    void dragEnterEvent(QDragEnterEvent *event);    // drag file into
    void dropEvent(QDropEvent *event);              // drop file into
    void mousePressEvent(QMouseEvent *event);       // pressed mouse down
    void mouseReleaseEvent(QMouseEvent *event);     // released mouse up
    void mouseMoveEvent(QMouseEvent *event);        // moved mouse on the form
    void mouseDoubleClickEvent(QMouseEvent *event); // double clicked the form
    bool eventFilter(QObject *obj, QEvent *event);  // event filter (get mouse move events from mpvFrame)

    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
    bool SetScreenshotTemplate();                   // prompt the user to select a screenshot directory

private slots:

    void FullScreen(bool fs);                       // makes window fullscreen
    void ShowPlaylist(bool visible);                // sets the playlist visibility
    void HideAlbumArt(bool hide);                   // hides the album art
    void FitWindow(int percent);                    // fit the window the the specified percent
    void SetAspectRatio(QString aspect);            // set the aspect ratio to specified proportions
    void DimDesktop(bool dim);                      // grays out the rest of the screen with LightDialog
    void AlwaysOnTop(bool ontop);

private:
    Ui::MainWindow  *ui;
    QSettings       *settings;
    MpvHandler      *mpv;
    UpdateManager   *update;

    QPoint          lastMousePos;
    bool            move,
                    pathChanged,
                    menuVisible,
                    init;
    QTimer          *autohide;

    QSystemTrayIcon *sysTrayIcon;
    QMenu           *trayIconMenu;
    DimDialog       *dimDialog;
    QSignalMapper   *chaptersSignalMapper;

    // variables
    QString onTop;
    int autoFit;
    bool hidePopup,
         remaining,
         debug;
public:
    QString getOnTop()          { return onTop; }
    int getAutoFit()            { return autoFit; }
    bool getHidePopup()         { return hidePopup; }
    bool getRemaining()         { return remaining; }
    bool getDebug()             { return debug; }
public slots:
    void setOnTop(QString s)    { emit onTopChanged(onTop = s); }
    void setAutoFit(int b)      { emit autoFitChanged(autoFit = b); }
    void setHidePopup(bool b)   { emit hidePopupChanged(hidePopup = b); }
    void setRemaining(bool b)   { emit remainingChanged(remaining = b); }
    void setDebug(bool b)       { emit debugChanged(debug = b); }
signals:
    void onTopChanged(QString);
    void autoFitChanged(int);
    void hidePopupChanged(bool);
    void remainingChanged(bool b);
    void debugChanged(bool);
};

#endif // MAINWINDOW_H
