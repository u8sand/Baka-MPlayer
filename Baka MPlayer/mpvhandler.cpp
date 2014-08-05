#include "mpvhandler.h"

#include <QCoreApplication>
#include <string>

static void wakeup(void *ctx)
{
    MpvHandler *mpvhandler = (MpvHandler*)ctx;
    QCoreApplication::postEvent(mpvhandler, new QEvent(QEvent::User));
}

MpvHandler::MpvHandler(int64_t wid, QObject *parent):
    QObject(parent),
    mpv(0),
    file(""),
    time(0),
    totalTime(0),
    volume(100),
    playState(Mpv::Stopped)
{
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    mpv_set_option_string(mpv, "input-default-bindings", "yes"); // mpv default key bindings
    // customize mpv keybindings and seek values etc...

    mpv_observe_property(mpv, 0, "path", MPV_FORMAT_STRING);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "length", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);

    mpv_set_wakeup_callback(mpv, wakeup, this);

    if(mpv_initialize(mpv) < 0)
        throw "Could not initialize mpv";
}

MpvHandler::~MpvHandler()
{
    if(mpv)
    {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
    }
}

QString MpvHandler::GetFile() const
{
    return file;
}

time_t MpvHandler::GetTime() const
{
    return time;
}

time_t MpvHandler::GetTotalTime() const
{
    return totalTime;
}

int MpvHandler::GetVolume() const
{
    return volume;
}

Mpv::PlayState MpvHandler::GetPlayState() const
{
    return playState;
}

bool MpvHandler::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        while(mpv)
        {
            mpv_event *event = mpv_wait_event(mpv, 0);
            if (event->event_id == MPV_EVENT_NONE)
                break;
            switch (event->event_id)
            {
            case MPV_EVENT_PROPERTY_CHANGE:
            {
                mpv_event_property *prop = (mpv_event_property*)event->data;
                if (strcmp(prop->name, "path") == 0)
                    if (prop->format == MPV_FORMAT_STRING)
                        SetFile(((std::string*)prop->data)->c_str());
                if (strcmp(prop->name, "time-pos") == 0)
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTime((time_t)*(double*)prop->data);
                if (strcmp(prop->name, "length") == 0)
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTotalTime((time_t)*(double*)prop->data);
                if (strcmp(prop->name, "volume") == 0)
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetVolume((int)*(double*)prop->data);
                break;
            }
//            case MPV_EVENT_FILE_LOADED: // essentially when i get this event it won't start
//                break;
            case MPV_EVENT_IDLE:
                SetTime(0);
                SetPlayState(Mpv::Idle);
                break;
            case MPV_EVENT_START_FILE:
                SetPlayState(Mpv::Started);
            case MPV_EVENT_UNPAUSE:
                SetPlayState(Mpv::Playing);
                break;
            case MPV_EVENT_PAUSE:
                SetPlayState(Mpv::Paused);
                break;
            case MPV_EVENT_END_FILE:
                SetPlayState(Mpv::Ended);
                SetPlayState(Mpv::Stopped);
                break;
            case MPV_EVENT_SHUTDOWN:
                QCoreApplication::quit();
                break;
            case MPV_EVENT_COMMAND_REPLY:
                if(event->error < 0)
                    emit ErrorSignal(mpv_error_string(event->error));
                break;
            default: // unhandled events
                break;
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MpvHandler::OpenFile(QString f)
{
    const char *args[] = {"loadfile", f.toUtf8().data(), NULL};
    AsyncCommand(args);
}

void MpvHandler::PlayPause(bool justPause)
{
    if(justPause)
    {
        const char *args[] = {"set", "pause", "yes", NULL};
        AsyncCommand(args);
    }
    else
    {
        const char *args[] = {"cycle", "pause", NULL};
        AsyncCommand(args);
    }
}

void MpvHandler::Seek(int pos, bool relative)
{
    const char *args[] = {"seek", QString::number(pos).toUtf8().data(), (relative ? "relative" : "absolute"), NULL};
    AsyncCommand(args);
}

void MpvHandler::Restart()
{
    const char *args[] = {"seek", "0", "absolute", NULL};
    AsyncCommand(args);
}

void MpvHandler::Stop()
{
    Restart();
    PlayPause(true);
}

void MpvHandler::SetChapter(char c)
{
    const char *args[] = {"set", "chapter", QString::number(c).toUtf8().data(), NULL};
    AsyncCommand(args);
}

void MpvHandler::NextChapter()
{
    const char *args[] = {"add", "chapter", "1", NULL};
    AsyncCommand(args);
}

void MpvHandler::PreviousChapter()
{
    const char *args[] = {"add", "chapter", "-1", NULL};
    AsyncCommand(args);
}

void MpvHandler::FrameStep()
{
    const char *args[] = {"frame_step", NULL};
    AsyncCommand(args);
}

void MpvHandler::FrameBackStep()
{
    const char *args[] = {"frame_back_step", NULL};
    AsyncCommand(args);
}

void MpvHandler::AdjustVolume(int level)
{
    const char *args[] = {"set", "volume", QString::number(level).toUtf8().data(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Snapshot(bool withSubs)
{
    const char *args[] = {"screenshot", (withSubs ? "subtitles" : "window"), NULL};
    AsyncCommand(args);
}

void MpvHandler::ToggleFullscreen()
{
    const char *args[] = {"cycle", "fullscreen", NULL};
    AsyncCommand(args);
}

void MpvHandler::ToggleSubs()
{
    const char *args[] = {"cycle", "sub-visibility", NULL};
    AsyncCommand(args);
}

void MpvHandler::AsyncCommand(const char *args[])
{
    // this could make things even nicer if qt had initializer_list support...
    // eg. AsyncCommand({"cycle", "fullscreen", NULL});

    if(mpv)
        mpv_command_async(mpv, 0, args);
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::SetFile(QString f)
{
    if(file != f)
    {
        file = f;
        emit FileChanged(f);
    }
}

void MpvHandler::SetTime(time_t t)
{
    if(time != t)
    {
        time = t;
        emit TimeChanged(t);
    }
}

void MpvHandler::SetTotalTime(time_t t)
{
    if(totalTime != t)
    {
        totalTime = t;
        emit TotalTimeChanged(t);
    }
}

void MpvHandler::SetVolume(int v)
{
    if(volume != v)
    {
        volume = v;
        emit VolumeChanged(v);
    }
}

void MpvHandler::SetPlayState(Mpv::PlayState s)
{
    if (playState != s)
    {
        playState = s;
        emit PlayStateChanged(s);
    }
}
