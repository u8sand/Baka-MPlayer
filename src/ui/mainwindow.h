#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QModelIndex>
#include <QStringList>
#include <QMouseEvent>
#include <QEvent>
#include <QPoint>

#include "mpvhandler.h"
#include "updatemanager.h"
#include "lightdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Load(QString f);

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
    QString FormatTime(int time);                   // format the time the way we want to display it

private slots:

    void FullScreen(bool fs);                       // makes window fullscreen
    void SetPlaylist(bool visible);                 // sets the playlist visibility
    void FitWindow(int percent);                    // fit the window the the specified percent
    void SetAspectRatio(QString aspect);            // set the aspect ratio to specified proportions
    void DimLights(bool dim);                       // grays out the rest of the screen with LightDialog

#ifdef Q_OS_WIN
    void SetAlwaysOnTop(bool ontop);
    void AlwaysOnTop(bool ontop);
    void AlwaysOnTopWhenPlaying(bool ontop);
    void NeverOnTop(bool ontop);
#endif

private:
    Ui::MainWindow  *ui;
    QSettings       *settings;
    MpvHandler      *mpv;
    UpdateManager   *update;

    QPoint          lastMousePos;
    bool            dragging,
                    init;

    QSystemTrayIcon *sysTrayIcon;
    QMenu           *trayIconMenu;
    LightDialog     *light;

    // variables
    QString onTop;
    int autoFit;
    bool trayIcon,
         hidePopup,
         debug;
public:
    QString getOnTop()          { return onTop; }
    int getAutoFit()            { return autoFit; }
    bool getTrayIcon()          { return trayIcon; }
    bool getHidePopup()         { return hidePopup; }
    bool getDebug()             { return debug; }
public slots:
    void setOnTop(QString s)    { emit onTopChanged(onTop = s); }
    void setAutoFit(int b)      { emit autoFitChanged(autoFit = b); }
    void setTrayIcon(bool b)    { emit trayIconChanged(trayIcon = b); }
    void setHidePopup(bool b)   { emit hidePopupChanged(hidePopup = b); }
    void setDebug(bool b)       { emit debugChanged(debug = b); }
signals:
    void onTopChanged(QString);
    void autoFitChanged(int);
    void trayIconChanged(bool);
    void hidePopupChanged(bool);
    void debugChanged(bool);
};

#endif // MAINWINDOW_H
