#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QMetaType>
#include <QObject>
#include <QString>

#include <mpv/client.h>

#include <initializer_list>

// Mpv::PlayState enum
namespace Mpv
{
    enum PlayState
    {
        Idle,
        Started,
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
    explicit MpvHandler(int64_t wid, QObject *parent = 0);
    ~MpvHandler();

    QString GetFile() const;                    // return file
    time_t GetTime() const;                     // return time
    time_t GetTotalTime() const;                // return totalTime
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
    void SetChapter(char chapter);              // seek to the specified chapter
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
    void SetTime(time_t t);                     // set time, emit signal
    void SetTotalTime(time_t t);                // set totalTime, emit signal
    void SetVolume(int v);                      // set volume, emit signal
    void SetPlayState(Mpv::PlayState s);        // set playState, emit signal

signals:
    void FileChanged(QString f);
    void TimeChanged(time_t t);
    void TotalTimeChanged(time_t t);
    void VolumeChanged(int l);
    void PlayStateChanged(Mpv::PlayState s);
    void ErrorSignal(QString e);

private:
    mpv_handle *mpv; // mpv client handle

    QString file;
    time_t time,
           totalTime;
    int chapter,
        volume;
    Mpv::PlayState playState;
};

#endif // MPVHANDLER_H
