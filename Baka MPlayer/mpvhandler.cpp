#include "mpvhandler.h"

#include <QCoreApplication>

#include <string>

static void wakeup(void *ctx)
{
    MpvHandler *mpvhandler = (MpvHandler*)ctx;
    QCoreApplication::postEvent(mpvhandler, new QEvent(QEvent::User));
}

MpvHandler::MpvHandler(QSettings *_settings, int64_t wid, QObject *parent):
    QObject(parent),
    settings(_settings),
    mpv(0),
    file(""),
    time(0),
    totalTime(0),
    playState(Mpv::Stopped)
{
    volume = settings->value("mpv/volume", 100).toInt();

    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
    mpv_set_option_string(mpv, "input-default-bindings", "no");

    mpv_observe_property(mpv, 0, "media-title", MPV_FORMAT_STRING);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "length", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);

    mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, (double*)&volume);

    if(settings->value("debug/mpv", false).toBool())
        mpv_request_log_messages(mpv, "debug");
    else
        mpv_request_log_messages(mpv, "no");

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

int MpvHandler::GetTime() const
{
    return time;
}

int MpvHandler::GetTotalTime() const
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
            if(event->error < 0)
                emit ErrorSignal(mpv_error_string(event->error));
            switch (event->event_id)
            {
            case MPV_EVENT_PROPERTY_CHANGE:
            {
                mpv_event_property *prop = (mpv_event_property*)event->data;
                if(QString(prop->name) == "media-title")
                    if (prop->format == MPV_FORMAT_STRING)
                        SetFile(((std::string*)prop->data)->c_str());
                if(QString(prop->name) == "time-pos")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTime((int)*(double*)prop->data);
                if(QString(prop->name) == "length")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTotalTime((int)*(double*)prop->data);
                if(QString(prop->name) =="volume")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetVolume((int)*(double*)prop->data);
                break;
            }
            case MPV_EVENT_IDLE:
                SetTime(0);
                SetFile("");
                SetPlayState(Mpv::Idle);
                break;
            case MPV_EVENT_FILE_LOADED:
                SetPlayState(Mpv::Loaded);
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
            case MPV_EVENT_LOG_MESSAGE:
                emit DebugSignal(QString(((mpv_event_log_message*)event->data)->text));
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
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
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
    const QByteArray tmp = QString::number(pos).toUtf8();
    const char *args[] = {"seek", tmp.constData(), (relative ? "relative" : "absolute"), NULL};
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

void MpvHandler::SetChapter(int c)
{
    const QByteArray tmp = QString::number(c).toUtf8();
    const char *args[] = {"set", "chapter", tmp.constData(), NULL};
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
    const QByteArray tmp = QString::number(level).toUtf8();
    const char *args[] = {"set", "volume", tmp.constData(), NULL};
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

void MpvHandler::SetTime(int t)
{
    if(time != t)
    {
        time = t;
        emit TimeChanged(t);
    }
}

void MpvHandler::SetTotalTime(int t)
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
