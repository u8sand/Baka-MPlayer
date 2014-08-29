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
    struct Chapter
    {
        QString title;
        int time;
    };
    struct Track
    {
        int id;
        QString type;
        int src_id;
        QString title;
        QString lang;
        unsigned albumart : 1,
                 _default : 1,
                 external : 1;
        QString external_filename;
        QString codec;
    };
    struct VideoParams
    {
        int width,
            height,
            dwidth,
            dheight;
        double aspect;
    };

    struct FileInfo
    {
        QString media_title;
        int length;
        VideoParams video_params;
        QList<Track> tracks; // audio, video, and subs
        QList<Chapter> chapters;
    };
}
Q_DECLARE_METATYPE(Mpv::PlayState) // so we can pass it with signals & slots
Q_DECLARE_METATYPE(Mpv::Chapter)
Q_DECLARE_METATYPE(Mpv::Track)
Q_DECLARE_METATYPE(Mpv::VideoParams)
Q_DECLARE_METATYPE(Mpv::FileInfo)

class MpvHandler : public QObject
{
    Q_OBJECT
public:
    explicit MpvHandler(QSettings *settings, int64_t wid, QObject *parent = 0);
    ~MpvHandler();

    const Mpv::FileInfo &GetFileInfo() const;   // return file info
    int GetTime() const;                        // return time
    int GetVolume() const;                      // return volume
    Mpv::PlayState GetPlayState() const;        // return playState

protected:
    virtual bool event(QEvent *event);          // QObject event function

public slots:
    void OpenFile(QString f);                   // open the file for mpv playing
    void Play();
    void Pause();
    void PlayPause();                           // toggle pause/unpause state
    void Seek(int pos, bool relative = false);  // seek to specific location
    void Restart();                             // seek to the beginning
    void Stop();                                // stop playback (and go to beginning)
    void SetVid(int vid);                       // set the video track
    void SetAid(int aid);                       // set the audio track
    void SetSid(int sid);                       // set the subtitle track
    void SetAspect(QString aspect);             // set the aspect ratio
    void AddSub(QString f);                     // add an external subtitle track
    void SetChapter(int chapter);               // seek to the specified chapter
    void NextChapter();                         // seek to next chapter
    void PreviousChapter();                     // seek to previous chapter
    void FrameStep();                           // frame step
    void FrameBackStep();                       // frame back step
    void AddVolume(int level);
    void AdjustVolume(int level);               // adjust the media volume
    void Screenshot(bool withSubs = false);     // take a screenshot
    void ToggleFullscreen();                    // toggle fullscreen
    void ToggleSubs();                          // toggle subtitles
    void AddSubScale(double scale);
    void SetSubScale(double scale);

    void LoadFileInfo();                        // load all the required file information
    void LoadTracks();
    void LoadChapters();
    void LoadVideoParams();
private slots:
    void AsyncCommand(const char *args[]);      // execute async mpv command


    void SetTime(int t);                        // set time, emit signal
    void SetVolume(int v);                      // set volume, emit signal
    void SetPlayState(Mpv::PlayState s);        // set playState, emit signal

signals:
    void TimeChanged(int t);                    // triggered on time changed
    void VolumeChanged(int l);                  // triggered on volume changed
    void PlayStateChanged(Mpv::PlayState s);    // triggered on playstate changed
    void ErrorSignal(QString e);                // triggered when an error occurs
    void DebugSignal(QString d);                // triggered when an mpv log message is send

private:
    QSettings *settings;                        // application-wide settings
    mpv_handle *mpv;                            // mpv client handle

    Mpv::FileInfo fileInfo;                     // the current file information
    int time,                                   // the current time-pos
        volume;                                 // the current volume
    Mpv::PlayState playState;                   // the current playstate
};

#endif // MPVHANDLER_H
