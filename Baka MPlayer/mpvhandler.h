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
        FileOpened,
        FileEnded,
        TimeChanged,
        TimeRemainingChanged,
        Shutdown,
        UnhandledEvent
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
    inline time_t GetTimeRemaining() const { return timeRemaining; }
protected:
    inline void SetVolume(int v) { volume = v;  }
    inline void SetTime(time_t t) { time = t; }
    inline void SetTimeRemaining(time_t t) { timeRemaining = t; }
private:
    mpv_handle *mpv;
//    void (*wakeup)(void*);
//    void *win;

    int volume;
    time_t time,
           timeRemaining;
};

#endif // MPVHANDLER_H
