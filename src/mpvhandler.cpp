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
    mpv_set_option_string(mpv, "cursor-autohide", "no");// we'll handle hiding the cursor
    mpv_set_option_string(mpv, "af", "scaletempo");     // make sure audio tempo is scaled (when speed is changing)

    // get updates when these properties change
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "sub-visibility", MPV_FORMAT_FLAG);
    // todo: observe tracklist

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
                if(QString(prop->name) == "volume")
                    if (prop->format == MPV_FORMAT_DOUBLE)
                        setVolume((int)*(double*)prop->data);
                if(QString(prop->name) == "sub-visibility")
                    if (prop->format == MPV_FORMAT_FLAG)
                        setSubtitleVisibility((bool)*(unsigned*)prop->data);
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
                setPlayState(Mpv::Started);
                // set properties
                Volume(volume);
                Speed(speed);
            case MPV_EVENT_UNPAUSE:
                setPlayState(Mpv::Playing);
                break;
            case MPV_EVENT_PAUSE:
                setPlayState(Mpv::Paused);
                break;
            case MPV_EVENT_END_FILE:
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

void MpvHandler::LoadSettings(QSettings *settings)
{
    setLastFile(settings->value("mpv/lastFile", "").toString());
    ShowAllPlaylist(settings->value("mpv/showAll", false).toBool());
    ScreenshotFormat(settings->value("mpv/screenshotFormat", "png").toString());
    ScreenshotTemplate(settings->value("mpv/screenshotTemplate", "screenshot%#04n").toString());
    ScreenshotDirectory(settings->value("mpv/screenshotDir", "").toString());
    Speed(settings->value("mpv/speed", 1.0).toDouble());
    Volume(settings->value("mpv/volume", 100).toInt());
    Debug(settings->value("common/debug", false).toBool());
}

void MpvHandler::SaveSettings(QSettings *settings)
{
    settings->setValue("mpv/lastFile", lastFile);
    settings->setValue("mpv/showAll", showAll);
    settings->setValue("mpv/screenshotFormat", screenshotFormat);
    settings->setValue("mpv/screenshotTemplate", screenshotTemplate);
    settings->setValue("mpv/screenshotDir", screenshotDir);
    settings->setValue("mpv/speed", speed);
    settings->setValue("mpv/volume", volume);
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
        setPlaylist();
    }
    else // local file
    {
        QFileInfo fi(f);
        if(path != fi.absolutePath() || // path is the same
          (i = playlist.indexOf(fi.fileName())) != -1) // file doesn't exists in the list
        {
            path = QString(fi.absolutePath()+"/"); // get path
            suffix = fi.suffix();
            PopulatePlaylist();
            SortPlaylist();
            setPlaylist();
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
        if(path == "") // web url
        {
            if(getFile() != "")
                setLastFile(getFile());
            OpenFile(playlist[i]);
        }
        else
        {
            QFile f(path+playlist[i]);
            if(f.exists())
            {
                if(getFile() != "")
                    setLastFile(getFile());
                OpenFile(path+playlist[i]);
                Play();
            }
            else
                Stop();
        }
        setIndex(i);
    }
    else // out of bounds
    {
        Stop();
        if(i < 0)
            setIndex(0);
        else
            setIndex(playlist.size()-1);
    }
}

void MpvHandler::NextFile()
{
    PlayIndex(index+1);
}

void MpvHandler::PreviousFile()
{
    PlayIndex(index-1);
}

void MpvHandler::RefreshPlaylist()
{
    setShuffle(false);
    setSearch("");
    PopulatePlaylist();
    SortPlaylist();
    setPlaylist();
}

void MpvHandler::SearchPlaylist(QString s)
{
    QStringList tmplist;
    for(QStringList::iterator item = playlist.begin(); item != playlist.end(); ++item)
        if(item->contains(s, Qt::CaseInsensitive))
            tmplist.push_back(*item);
    emit playlistChanged(tmplist);
}

void MpvHandler::ShufflePlaylist(bool b)
{
    setShuffle(b);
    SortPlaylist();
    setPlaylist();
}

void MpvHandler::ShowAllPlaylist(bool b)
{
    showAll = b;
    if(!b)
        suffix = QFileInfo(getFile()).suffix();
    else
        suffix = "";
    PopulatePlaylist();
    SortPlaylist();
    setPlaylist();
    setSearch("");
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

void MpvHandler::Stop()
{
    Restart();
    Pause();
}

void MpvHandler::PlayPause(int indexIfStopped)
{
    if(playState == Mpv::Idle) // if idle, play plays the selected playlist file
        PlayIndex(indexIfStopped);
    else
    {
        const char *args[] = {"cycle", "pause", NULL};
        AsyncCommand(args);
    }
}

void MpvHandler::Restart()
{
    Seek(0);
    Play();
}

void MpvHandler::Rewind()
{
    // if user presses rewind button twice within 3 seconds, stop video
    if(time < 3)
    {
        Stop();
    }
    else
    {
        if(playState == Mpv::Playing)
            Restart();
        else
            Stop();
    }
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

void MpvHandler::Chapter(int c)
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

void MpvHandler::Volume(int level)
{
    if(level > 100) level = 100;
    else if(level < 0) level = 0;

    if(playState > 0)
    {
        const QByteArray tmp = QString::number(level).toUtf8();
        const char *args[] = {"set", "volume", tmp.constData(), NULL};
        AsyncCommand(args);
    }
    else
        setVolume(level);
}

void MpvHandler::Speed(double d)
{
    if(playState > 0)
    {
        const QByteArray tmp = QString::number(d).toUtf8();
        const char *args[] = {"set", "speed", tmp.constData(), NULL};
        AsyncCommand(args);
    }
    else
        setSpeed(d);
}

void MpvHandler::Aspect(QString aspect)
{
    const QByteArray tmp = aspect.toUtf8();
    const char *args[] = {"set", "video-aspect", tmp.constData(), NULL};
    AsyncCommand(args);
}


void MpvHandler::Vid(int vid)
{
    const QByteArray tmp = QString::number(vid).toUtf8();
    const char *args[] = {"set", "vid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Aid(int aid)
{
    const QByteArray tmp = QString::number(aid).toUtf8();
    const char *args[] = {"set", "aid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Sid(int sid)
{
    const QByteArray tmp = QString::number(sid).toUtf8();
    const char *args[] = {"set", "sid", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Screenshot(bool withSubs)
{
    const char *args[] = {"screenshot", (withSubs ? "subtitles" : "video"), NULL};
    AsyncCommand(args);
}

void MpvHandler::ScreenshotFormat(QString s)
{
    if(mpv)
    {
        const QByteArray tmp = s.toUtf8();
        mpv_set_option_string(mpv, "screenshot-format", tmp.data());
    }
    setScreenshotFormat(s);
}

void MpvHandler::ScreenshotTemplate(QString s)
{
    if(mpv)
    {
        s = screenshotDir+s;
        const QByteArray tmp = s.toUtf8();
        mpv_set_option_string(mpv, "screenshot-template", tmp.data());
    }
    setScreenshotTemplate(s);
}

void MpvHandler::ScreenshotDirectory(QString s)
{
    if(mpv)
    {
        s += screenshotTemplate;
        const QByteArray tmp = s.toUtf8();
        mpv_set_option_string(mpv, "screenshot-template", tmp.data());
    }
    setScreenshotDir(s);
}

void MpvHandler::AddSubtitleTrack(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"sub_add", tmp.constData(), NULL};
    AsyncCommand(args);
    // todo: add track to tracklist
    // todo: select this track
}

void MpvHandler::ShowSubtitles(bool b)
{
    const char *args[] = {"set", "sub-visibility", b?"yes":"no", NULL};
    AsyncCommand(args);
}

void MpvHandler::SubtitleScale(double scale, bool relative)
{
    const QByteArray tmp = QString::number(scale).toUtf8();
    const char *args[] = {relative?"add":"set", "sub-scale", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Debug(bool b)
{
    if(mpv)
    {
        mpv_request_log_messages(mpv, b ? "debug" : "no");
        setDebug(b);
    }
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

    emit fileInfoChanged(fileInfo);
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

void MpvHandler::OpenFile(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
    AsyncCommand(args);
    setFile(f);
}

void MpvHandler::PopulatePlaylist()
{
    if(path != "")
    {
        playlist.clear(); // clear existing list
        QDir root(path);
        QFileInfoList flist;
        if(suffix == "")
            flist = root.entryInfoList(Mpv::media_filetypes, QDir::Files);
        else
            flist = root.entryInfoList({QString("*.").append(suffix)}, QDir::Files);
        for(auto &i : flist)
            playlist.push_back(i.fileName()); // add files to the list
    }
}

void MpvHandler::SortPlaylist()
{
    if(shuffle) // shuffle list
        std::random_shuffle(playlist.begin(), playlist.end());
    else        // sort list
        std::sort(playlist.begin(), playlist.end());
}

void MpvHandler::AsyncCommand(const char *args[])
{
    if(mpv)
        mpv_command_async(mpv, 0, args);
    else
        emit errorSignal("mpv was not initialized");
}
