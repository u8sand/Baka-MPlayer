#include "mpvhandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QRegExp>

#include <algorithm> // for std::random_shuffle and std::sort

static void wakeup(void *ctx)
{
    MpvHandler *mpvhandler = (MpvHandler*)ctx;
    QCoreApplication::postEvent(mpvhandler, new QEvent(QEvent::User));
}

MpvHandler::MpvHandler(int64_t wid, QObject *parent):
    QObject(parent),
    mpv(0)
{
    // create mpv
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    // set mpv options
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
    mpv_set_option_string(mpv, "input-cursor", "no");   // no mouse handling
    mpv_set_option_string(mpv, "af", "scaletempo");     // make sure audio tempo is scaled (when speed is changing)
    mpv_set_option_string(mpv, "cursor-autohide", "no");

    // get updates when these properties change
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);

    // setup callback event handling
    mpv_set_wakeup_callback(mpv, wakeup, this);

    // initialize mpv
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
            if(event->error < 0)
                emit errorSignal(mpv_error_string(event->error));
            switch (event->event_id)
            {
            case MPV_EVENT_PROPERTY_CHANGE:
            {
                mpv_event_property *prop = (mpv_event_property*)event->data;
                if(QString(prop->name) == "time-pos")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        setTime((int)*(double*)prop->data);
                if(QString(prop->name) =="volume")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        setVolume((int)*(double*)prop->data);
                break;
            }
            case MPV_EVENT_IDLE:
                fileInfo.length = 0;
                setTime(0);
                setPlayState(Mpv::Idle);
                break;
                // these two look like they're reversed but they aren't. the names are misleading.
            case MPV_EVENT_START_FILE:
                setPlayState(Mpv::Loaded);
                break;
            case MPV_EVENT_FILE_LOADED:
                LoadFileInfo();
                AdjustVolume(volume);
                setPlayState(Mpv::Started);
            case MPV_EVENT_UNPAUSE:
                setPlayState(Mpv::Playing);
                break;
            case MPV_EVENT_PAUSE:
                setPlayState(Mpv::Paused);
                break;
            case MPV_EVENT_END_FILE:
                setPlayState(Mpv::Ended);
                setPlayState(Mpv::Stopped);
                break;
            case MPV_EVENT_SHUTDOWN:
                QCoreApplication::quit();
                break;
            case MPV_EVENT_LOG_MESSAGE:
                emit debugSignal(QString(((mpv_event_log_message*)event->data)->text));
                break;
            default: // unhandled events
                break;
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MpvHandler::LoadFile(QString f)
{
    if(f == "") return; // ignore empty file name

    int i;
    QRegExp rx("^(https?://.+\\.[a-z]+)", Qt::CaseInsensitive);

    if(rx.indexIn(f) != -1) // web url
    {
        i = 0;
        path = "";
        playlist.clear();
        playlist.push_back(f);
        emit playlistChanged(playlist);
    }
    else // local file
    {
        QFileInfo fi(f);
        if(path != fi.absolutePath() || // path is the same
          (i = playlist.indexOf(fi.fileName())) != -1) // file doesn't exists in the list
        {
            path = QString(fi.absolutePath()+"/"); // get path
            suffix = fi.suffix();
            Populate();
            Sort();
            emit playlistChanged(playlist);
            i = playlist.indexOf(fi.fileName()); // get index
        }
    }
    if(playlist.size() > 1) // open up the playlist only if there is more than one item
        setPlaylistVisible(true);
    PlayIndex(i);
}

void MpvHandler::PlayIndex(int i)
{
    if(i >= 0 && i < playlist.size())
    {
        setIndex(i);
        if(path == "") // web url
            OpenFile(getFile());
        else
        {
            QFile f(getFile());
            if(f.exists())
            {
                OpenFile(getFile());
                Play();
            }
            else
                Stop();
        }
    }
    else // out of bounds
        Stop();
}

void MpvHandler::NextFile()
{
    PlayIndex(index+1);
}

void MpvHandler::PreviousFile()
{
    PlayIndex(index-1);
}

void MpvHandler::Populate()
{
    if(path != "")
    {
        playlist.clear(); // clear existing list
        QDir root(path);
        QFileInfoList flist;
        if(suffix == "")
            flist = root.entryInfoList({"*.mkv", "*.mp4", "*.avi", "*.mp3", "*.ogm"}, QDir::Files); // todo: pass more file-types (get from settings)
        else
            flist = root.entryInfoList({QString("*.").append(suffix)}, QDir::Files);
        for(auto &i : flist)
            playlist.push_back(i.fileName()); // add files to the list
    }
}

void MpvHandler::Refresh()
{
    setShuffle(false);
    setSearch("");
    Populate();
    Sort();
    emit playlistChanged(playlist);
}

void MpvHandler::Sort()
{
    if(shuffle) // shuffle list
        std::random_shuffle(playlist.begin(), playlist.end());
    else        // sort list
        std::sort(playlist.begin(), playlist.end());
}

void MpvHandler::OpenFile(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Play()
{
    const char *args[] = {"set", "pause", "no", NULL};
    AsyncCommand(args);
}

void MpvHandler::Pause()
{
    const char *args[] = {"set", "pause", "yes", NULL};
    AsyncCommand(args);
}

void MpvHandler::PlayPause()
{
    const char *args[] = {"cycle", "pause", NULL};
    AsyncCommand(args);
}

void MpvHandler::Seek(int pos, bool relative)
{
    const QByteArray tmp = QString::number(pos).toUtf8();
    if(relative)
    {
        const char *args[] = {"seek", tmp.constData(), NULL};
        AsyncCommand(args);
    }
    else
    {
        const char *args[] = {"seek", tmp.constData(), "absolute", NULL};
        AsyncCommand(args);
    }
}

void MpvHandler::Restart()
{
    const char *args[] = {"seek", "0", "absolute", NULL};
    AsyncCommand(args);
}

void MpvHandler::Stop()
{
    Restart();
    Pause();
}

void MpvHandler::SetVid(int vid)
{
    const QByteArray tmp = QString::number(vid).toUtf8();
    const char *args[] = {"set", "vid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::SetAid(int aid)
{
    const QByteArray tmp = QString::number(aid).toUtf8();
    const char *args[] = {"set", "aid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::SetSid(int sid)
{
    const QByteArray tmp = QString::number(sid).toUtf8();
    const char *args[] = {"set", "sid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::SetAspect(QString aspect)
{
    const QByteArray tmp = aspect.toUtf8();
    const char *args[] = {"set", "video-aspect", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::AddSub(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"sub_add", tmp.constData(), NULL};
    AsyncCommand(args);
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

void MpvHandler::AddVolume(int level)
{
    const QByteArray tmp = QString::number(level).toUtf8();
    const char *args[] = {"add", "volume", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::AdjustVolume(int level)
{
    const QByteArray tmp = QString::number(level).toUtf8();
    const char *args[] = {"set", "volume", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Screenshot(bool withSubs)
{
    const char *args[] = {"screenshot", (withSubs ? "subtitles" : "window"), NULL};
    AsyncCommand(args);
}

void MpvHandler::ToggleFullscreen()
{
    const char *args[] = {"cycle", "fullscreen", NULL};
    AsyncCommand(args);
}

void MpvHandler::SetSubs(bool b)
{
    const char *args[] = {"set", "sub-visibility", b?"1":"0", NULL};
    AsyncCommand(args);
}

void MpvHandler::AddSubScale(double scale)
{
    const QByteArray tmp = QString::number(scale).toUtf8();
    const char *args[] = {"add", "sub-scale", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::SetSubScale(double scale)
{
    const QByteArray tmp = QString::number(scale).toUtf8();
    const char *args[] = {"set", "sub-scale", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Debug(bool b)
{
    if(mpv)
        mpv_request_log_messages(mpv, b ? "debug" : "no");
}

void MpvHandler::CursorAutoHide(bool b)
{
    if(mpv)
        mpv_set_option_string(mpv, "cursor-autohide", b?"yes":"no");
}

void MpvHandler::AsyncCommand(const char *args[])
{
    if(mpv)
        mpv_command_async(mpv, 0, args);
    else
        emit errorSignal("mpv was not initialized");
}

void MpvHandler::LoadFileInfo()
{
    // get media-title
    fileInfo.media_title = mpv_get_property_string(mpv, "media-title");
    // get length
    double len;
    mpv_get_property(mpv, "length", MPV_FORMAT_DOUBLE, &len);
    fileInfo.length = (int)len;

    LoadTracks();
    LoadChapters();
    LoadVideoParams();
}

void MpvHandler::LoadTracks()
{
    fileInfo.tracks.clear();
    mpv_node node;
    mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_ARRAY)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
            {
                Mpv::Track track;
                for(int n = 0; n < node.u.list->values[i].u.list->num; n++)
                {
                    if(QString(node.u.list->values[i].u.list->keys[n]) == "id")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "type")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.type = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "src-id")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.src_id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "title")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "lang")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.lang = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "albumart")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.albumart = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "default")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track._default = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "external")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.external = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "external-filename")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.external_filename = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "codec")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.codec = node.u.list->values[i].u.list->values[n].u.string;
                    }
                }
                fileInfo.tracks.push_back(track);
            }
        }
    }
}

void MpvHandler::LoadChapters()
{
    fileInfo.chapters.clear();
    mpv_node node;
    mpv_get_property(mpv, "chapter-list", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_ARRAY)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
            {
                Mpv::Chapter ch;
                for(int n = 0; n < node.u.list->values[i].u.list->num; n++)
                {
                    if(QString(node.u.list->values[i].u.list->keys[n]) == "title")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            ch.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "time")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_DOUBLE)
                            ch.time = (int)node.u.list->values[i].u.list->values[n].u.double_;
                    }
                }
                fileInfo.chapters.push_back(ch);
            }
        }
    }
}

void MpvHandler::LoadVideoParams()
{
    mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &fileInfo.video_params.width);
    mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &fileInfo.video_params.height);
    mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &fileInfo.video_params.dwidth);
    mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &fileInfo.video_params.dheight);
    mpv_get_property(mpv, "video-aspect", MPV_FORMAT_INT64, &fileInfo.video_params.aspect);
}
