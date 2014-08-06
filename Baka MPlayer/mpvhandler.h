#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QObject>
#include <QMetaType>
#include <QSettings>
#include <QString>

#include <mpv/client.h>

// Mpv::PlayState enum
namespace Mpv
{
    enum PlayState
    {
        Idle,
        Started,
        Loaded,
        Playing,
        Paused,
        Stopped,
        Ended
    };
}
Q_DECLARE_METATYPE(Mpv::PlayState) // so we can pass it with signals & slots


class MpvHandler : public QObject
{
    Q_OBJECT
public:
    explicit MpvHandler(QSettings *settings, int64_t wid, QObject *parent = 0);
    ~MpvHandler();

    QString GetFile() const;                    // return file
    int GetTime() const;                        // return time
    int GetTotalTime() const;                   // return totalTime
    int GetVolume() const;                      // return volume
    Mpv::PlayState GetPlayState() const;        // return playState

protected:
    virtual bool event(QEvent *event);          // QObject event function

public slots:
    void OpenFile(QString f);                   // open the file for mpv playing
    void PlayPause(bool justPause = false);     // toggle pause/unpause state
    void Seek(int pos, bool relative = false);  // seek to specific location
    void Restart();                             // seek to the beginning
    void Stop();                                // stop playback (and go to beginning)
    void SetChapter(int chapter);               // seek to the specified chapter
    void NextChapter();                         // seek to next chapter
    void PreviousChapter();                     // seek to previous chapter
    void FrameStep();                           // frame step
    void FrameBackStep();                       // frame back step
    void AdjustVolume(int level);               // adjust the media volume
    void Snapshot(bool withSubs = false);       // take a snapshot
    void ToggleFullscreen();                    // toggle fullscreen
    void ToggleSubs();                          // toggle subtitles

private slots:
    void AsyncCommand(const char *args[]);      // execute async mpv command
    void SetFile(QString f);                    // set file, emit signal
    void SetTime(int t);                        // set time, emit signal
    void SetTotalTime(int t);                   // set totalTime, emit signal
    void SetVolume(int v);                      // set volume, emit signal
    void SetPlayState(Mpv::PlayState s);        // set playState, emit signal

signals:
    void FileChanged(QString f);                // triggered on file changed
    void TimeChanged(int t);                 // triggered on time changed
    void TotalTimeChanged(int t);            // triggered on totalTime changed
    void VolumeChanged(int l);                  // triggered on volume changed
    void PlayStateChanged(Mpv::PlayState s);    // triggered on playstate changed
    void ErrorSignal(QString e);                // triggered when an error occurs
    void DebugSignal(QString d);                // triggered when an mpv log message is send

private:
    QSettings *settings;                        // application-wide settings
    mpv_handle *mpv;                            // mpv client handle

    QString file;                               // the current file path
    int time,                                   // the current time-pos
        totalTime,                              // the current total time
        volume;                                 // the current volume
    Mpv::PlayState playState;                   // the current playstate
};

#endif // MPVHANDLER_H
