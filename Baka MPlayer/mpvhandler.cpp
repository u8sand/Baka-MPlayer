#include "mpvhandler.h"

#include <QCoreApplication>

static void wakeup(void *ctx)
{
    MpvHandler *mpvhandler = (MpvHandler*)ctx;
    QCoreApplication::postEvent(mpvhandler, new QEvent(QEvent::User));
}

MpvHandler::MpvHandler( int64_t wid, QObject *parent):
    QObject(parent),
    mpv(0),
//    url(""),
    time(0),
    totalTime(0),
    playState(Mpv::Stopped)
{
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    mpv_set_option_string(mpv, "input-default-bindings", "no");

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "length", MPV_FORMAT_DOUBLE);

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
                if (strcmp(prop->name, "time-pos") == 0)
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTime((time_t)*(double*)prop->data);
                if (strcmp(prop->name, "length") == 0)
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        SetTotalTime((time_t)*(double*)prop->data);
                break;
            }
            case MPV_EVENT_IDLE:
                SetTime(0);
                SetPlayState(Mpv::Idle);
                break;
            case MPV_EVENT_FILE_LOADED:
                //SetFile(event->data);
                //SetFile((QString)*(char*)event->data);
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
                SetTime(0);
                SetPlayState(Mpv::Ended);
                SetPlayState(Mpv::Stopped);
                break;
            default:
                // unhandled events
                break;
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MpvHandler::OpenFile(QString url/*, QString subFile = ""*/)
{
    if(mpv)
    {
//        externalSub = subFile;
        const char *args[] = {"loadfile", url.toUtf8().data(), NULL};
        mpv_command(mpv, args);
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::PlayPause(bool justPause)
{
    if(mpv)
    {
        if(justPause)
        {
            const char *args[] = {"set", "pause", "yes", NULL};
            mpv_command_async(mpv, 0, args);
        }
        else
        {
            const char *args[] = {"cycle", "pause", NULL};
            mpv_command_async(mpv, 0, args);
        }
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::Stop()
{
    if(mpv)
    {
        Seek(0);
        PlayPause(true);
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::Rewind()
{
    if(mpv)
    {
        const char *args[] = {"seek", "0", "absolute", NULL};
        mpv_command_async(mpv, 0, args);
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::Seek(int pos, bool relative)
{
    if(mpv)
    {
        const char *args[] = {"seek",
                              QString::number(pos).toUtf8().data(),
                              relative ? "relative" : "absolute",
                              NULL};
        mpv_command_async(mpv, 0, args);
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

void MpvHandler::AdjustVolume(int level)
{
    if(mpv)
    {
        const char *args[] = {"set", "volume",
                              QString::number(level).toUtf8().data(),
                              NULL};
        mpv_command_async(mpv, 0, args);
    }
    else
        emit ErrorSignal("mpv was not initialized");
}

