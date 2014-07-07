#include "mpvhandler.h"

MpvHandler::MpvHandler(int64_t wid, void (*_wakeup)(void*), void *_win):
    mpv(NULL),
//    wakeup(_wakeup),
//    win(_win),
    volume(50),
    time(0),
    timeRemaining(0)
{
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv";
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);

    mpv_set_option_string(mpv, "input-default-bindings", "no");
    mpv_set_option_string(mpv, "idle", "yes");

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-remaining", MPV_FORMAT_DOUBLE);

    mpv_set_wakeup_callback(mpv, _wakeup, _win);

    if(mpv_initialize(mpv) < 0)
        throw "Could not initialize mpv";
}

MpvHandler::~MpvHandler()
{
    if(mpv)
     mpv_terminate_destroy(mpv);
    mpv = NULL;
}

MpvHandler::MpvEvent MpvHandler::HandleEvent()
{
    if(mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            return NoEvent;
        switch (event->event_id)
        {
        case MPV_EVENT_PROPERTY_CHANGE:
        {
            mpv_event_property *prop = (mpv_event_property*)event->data;
            if (strcmp(prop->name, "time-pos") == 0)
            {
                if (prop->format == MPV_FORMAT_DOUBLE)
                {
                    SetTime((time_t)*(double*)prop->data);
                    return TimeChanged;
                }
            }
            if (strcmp(prop->name, "time-remaining") == 0)
            {
                if (prop->format == MPV_FORMAT_DOUBLE)
                {
                    SetTimeRemaining((time_t)*(double*)prop->data);
                    return TimeRemainingChanged;
                }
            }
            break;
        }
        case MPV_EVENT_FILE_LOADED:
            return FileOpened;
            break;
        case MPV_EVENT_IDLE:
            SetTime(0);
            SetTimeRemaining(0);
            return FileEnded;
        case MPV_EVENT_SHUTDOWN:
            return Shutdown;
        default:
            return UnhandledEvent;
        }
    }
    return NoEvent;
}

bool MpvHandler::OpenFile(QString url)
{
    if(mpv)
    {
        const char *args[] = {"loadfile", url.toUtf8().data(), NULL};
        mpv_command_async(mpv, 0, args);
    }
    else
        return false;
    return true;
}

//bool MpvHandler::OpenFile(QString url, QString subFile)
//{
//    // externalSub = subFile;
//    return OpenFile(url);
//}

bool MpvHandler::PlayPause(bool justPause)
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
        return true;
    }
    return false;
}

bool MpvHandler::Stop()
{
    if(mpv)
    {
        Seek(0);
        PlayPause(true);
        return true;
    }
    return false;
}

bool MpvHandler::Seek(int pos, bool relative)
{
    if(mpv)
    {
        const char *args[] = {"seek",
                              QString::number(pos).toUtf8().data(),
                              relative ? "relative" : "absolute",
                              NULL};
        mpv_command_async(mpv, 0, args);
        return true;
    }
    return false;
}

bool MpvHandler::Volume(int level)
{
    if(mpv)
    {
        const char *args[] = {"set", "volume",
                              QString::number(level).toUtf8().data(),
                              NULL};
        mpv_command_async(mpv, 0, args);
        return true;
    }
    return false;
}
