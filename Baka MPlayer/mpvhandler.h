#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QString>

#include <mpv/client.h>

class MpvHandler
{
public:
    enum MpvEvent
    {
        NoEvent,
        Idling,
        FileOpened,
        FileEnded,
        StateChanged,
        TimeChanged,
        UnhandledEvent
    };
    enum MpvState
    {
        Playing,
        Paused,
        Stopped
    };

    MpvHandler(int64_t wid, void (*wakeup)(void*), void *win);
    ~MpvHandler();

    MpvEvent HandleEvent();

    bool OpenFile(QString url);
    //bool OpenFile(QString url, QString subFile);
    bool PlayPause(bool justPause = false);
    bool Seek(int pos, bool relative = false);
    bool Volume(int level);
    bool Stop();

    inline int GetVolume() const { return volume; }
    inline time_t GetTime() const { return time; }
    inline time_t GetTotalTime() const { return totalTime; }
    inline MpvState GetState() const { return state; }
protected:
    inline void SetVolume(int v) { volume = v;  }
    inline void SetTime(time_t t) { time = t; }
    inline void SetTotalTime(time_t t) { totalTime = t; }
    inline void SetState(MpvState s) { state = s; }
private:
    mpv_handle *mpv;
//    int64_t wid;
//    void (*wakeup)(void*);
//    void *win;

    int64_t volume;
    time_t time,
           totalTime;
    MpvState state;
};

#endif // MPVHANDLER_H
