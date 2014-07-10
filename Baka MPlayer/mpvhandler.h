#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QMetaType>
#include <QObject>
#include <QString>

#include <mpv/client.h>

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
Q_DECLARE_METATYPE(Mpv::PlayState)

class MpvHandler : public QObject
{
    Q_OBJECT
public:
    explicit MpvHandler(int64_t wid, QObject *parent = 0);
    ~MpvHandler();

//    inline QString GetUrl() const { return url; }
    inline time_t GetTime() const { return time; }
    inline time_t GetTotalTime() const { return totalTime; }
    inline Mpv::PlayState GetPlayState() const { return playState; }

protected:
    virtual bool event(QEvent *event);

public slots:
    void OpenFile(QString s);
    void PlayPause(bool justPause = false);
    void Stop();
    void Rewind();
    void Seek(int pos, bool relative = false);
    void AdjustVolume(int level);

private slots:
    void SetFile(QString s) { /*url = s;*/ emit FileChanged(s); }
    void SetTime(time_t t) { time = t; emit TimeChanged(t); }
    void SetTotalTime(time_t t) { totalTime = t; emit TotalTimeChanged(t); }
    void SetPlayState(Mpv::PlayState s) { playState = s; emit PlayStateChanged(s); }

signals:
    void FileChanged(QString url);
    void TimeChanged(time_t t);
    void TotalTimeChanged(time_t t);
    void PlayStateChanged(Mpv::PlayState playState);
    void ErrorSignal(QString error);

private:
    mpv_handle *mpv;

//    QString url;
    time_t time,
           totalTime;
    Mpv::PlayState playState;
};

#endif // MPVHANDLER_H
