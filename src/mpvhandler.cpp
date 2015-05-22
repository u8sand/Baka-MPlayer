#include "mpvhandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>

#include "util.h"

static void wakeup(void *ctx)
{
    MpvHandler *mpvhandler = (MpvHandler*)ctx;
    QCoreApplication::postEvent(mpvhandler, new QEvent(QEvent::User));
}

MpvHandler::MpvHandler(int64_t wid, QObject *parent):
    QObject(parent)
{
    // create mpv
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    // set mpv options
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
    mpv_set_option_string(mpv, "input-cursor", "no");   // no mouse handling
    mpv_set_option_string(mpv, "cursor-autohide", "no");// no cursor-autohide, we handle that
    mpv_set_option_string(mpv, "ytdl", "yes"); // youtube-dl support

    // get updates when these properties change
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "sid", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "aid", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "sub-visibility", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "mute", MPV_FORMAT_FLAG);

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

QString MpvHandler::getMediaInfo()
{
    QFileInfo fi(path+file);

    QList<QPair<QString, QString>> items = {
        {tr("Media Title"), fileInfo.media_title},
        {tr("File name"), fi.fileName()},
        {tr("File size"), Util::HumanSize(fi.size())},
        {tr("Date created"), fi.created().toString()},
        {tr("Media length"), Util::FormatTime(fileInfo.length, fileInfo.length)},
        {QString(), QString()}
    };

    if(fileInfo.video_params.codec != QString())
        items.append({
            {tr("[Video]"), QString()},
            {tr("Codec"), fileInfo.video_params.codec},
            {tr("Bitrate"), fileInfo.video_params.bitrate},
            {tr("Dimensions"), QString("%0 x %1 (%2)").arg(QString::number(fileInfo.video_params.width),
                                                           QString::number(fileInfo.video_params.height),
                                                           Util::Ratio(fileInfo.video_params.width, fileInfo.video_params.height))},
            {QString(), QString()}
        });

    if(fileInfo.audio_params.codec != QString())
        items.append({
            {tr("[Audio]"), QString()},
            {tr("Codec"), fileInfo.audio_params.codec},
            {tr("Bitrate"), fileInfo.audio_params.bitrate},
            {tr("Samplerate"), fileInfo.audio_params.samplerate},
            {tr("Channels"), fileInfo.audio_params.channels},
            {QString(), QString()}
        });

    if(fileInfo.tracks.length() > 0)
    {
        items.append({tr("[Track List]"), QString()});
        for(auto &track : fileInfo.tracks)
            items.append({QString::number(track.id), QString("%0[%1:%2] %3").arg(track.title, track.type, track.lang, track.external_filename)});
        items.append({QString(), QString()});
    }
    if(fileInfo.chapters.length() > 0)
    {
        items.append({tr("[Chapter List]"), QString()});
        for(auto &chapter : fileInfo.chapters)
            items.append({chapter.title, Util::FormatTime(chapter.time, fileInfo.length)});
        items.append({QString(), QString()});
    }
    if(fileInfo.metadata.size() > 0)
    {
        items.append({tr("[Metadata]"), QString()});
        for(auto data = fileInfo.metadata.begin(); data != fileInfo.metadata.end(); ++data)
            items.append({data.key(), *data});
        items.append({QString(), QString()});
    }

    QString info;
    int spacing = 0;
    for(auto iter = items.begin(); iter != items.end(); ++iter)
    {
        int len = iter->first.length();
        if(len > spacing)
            spacing = len;
    }
    for(auto iter = items.begin(); iter != items.end(); ++iter)
    {
        int len = iter->first.length();
        info += iter->first + (iter->second == QString() ? QString() : ": ");
        while(len++ < spacing)
            info += ' ';
        info += iter->second + '\n';
    }
    return info;
}

bool MpvHandler::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        while(mpv)
        {
            mpv_event *event = mpv_wait_event(mpv, 0);
            if(event == nullptr ||
               event->event_id == MPV_EVENT_NONE)
            {
                break;
            }
            if(event->error < 0)
            {
                ShowText(mpv_error_string(event->error));
                emit messageSignal(mpv_error_string(event->error));
            }
            switch (event->event_id)
            {
            case MPV_EVENT_PROPERTY_CHANGE:
            {
                mpv_event_property *prop = (mpv_event_property*)event->data;
                if(QString(prop->name) == "time-pos")
                {
                    if(prop->format == MPV_FORMAT_DOUBLE)
                    {
                        setTime((int)*(double*)prop->data);
                        lastTime = time;
                    }
                }
                else if(QString(prop->name) == "volume")
                {
                    if(prop->format == MPV_FORMAT_DOUBLE)
                        setVolume((int)*(double*)prop->data);
                }
                else if(QString(prop->name) == "sid")
                {
                    if(prop->format == MPV_FORMAT_INT64)
                        setSid(*(int*)prop->data);
                }
                else if(QString(prop->name) == "aid")
                {
                    if(prop->format == MPV_FORMAT_INT64)
                        setAid(*(int*)prop->data);
                }
                else if(QString(prop->name) == "sub-visibility")
                {
                    if(prop->format == MPV_FORMAT_FLAG)
                        setSubtitleVisibility((bool)*(unsigned*)prop->data);
                }
                else if(QString(prop->name) == "mute")
                {
                    if(prop->format == MPV_FORMAT_FLAG)
                        setMute((bool)*(unsigned*)prop->data);
                }
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
                setPlayState(Mpv::Started);
                LoadFileInfo();
                SetProperties();
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
            {
                mpv_event_log_message *message = static_cast<mpv_event_log_message*>(event->data);
                if(message != nullptr)
                    emit messageSignal(message->text);
                break;
            }
            default: // unhandled events
                break;
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MpvHandler::AddOverlay(int id, int x, int y, QString file, int offset, int w, int h)
{
    QByteArray tmp_id = QString::number(id).toUtf8(),
               tmp_x = QString::number(x).toUtf8(),
               tmp_y = QString::number(y).toUtf8(),
               tmp_file = file.toUtf8(),
               tmp_offset = QString::number(offset).toUtf8(),
               tmp_w = QString::number(w).toUtf8(),
               tmp_h = QString::number(h).toUtf8(),
               tmp_stride = QString::number(4*w).toUtf8();

    const char *args[] = {"overlay_add",
                       tmp_id.constData(),
                       tmp_x.constData(),
                       tmp_y.constData(),
                       tmp_file.constData(),
                       tmp_offset.constData(),
                       "bgra",
                       tmp_w.constData(),
                       tmp_h.constData(),
                       tmp_stride.constData(),
                       NULL};
    Command(args);
}

void MpvHandler::RemoveOverlay(int id)
{
    QByteArray tmp = QString::number(id).toUtf8();
    const char *args[] = {"overlay_remove", tmp.constData(), NULL};
    AsyncCommand(args);
}

bool MpvHandler::FileExists(QString f)
{
    if(Util::IsValidUrl(f)) // web url
        return true;
    return QFile(f).exists();
}

void MpvHandler::LoadFile(QString f)
{
    PlayFile(LoadPlaylist(f));
}

QString MpvHandler::LoadPlaylist(QString f)
{
    if(f == QString()) // ignore empty file name
        return QString();

    if(f == "-")
    {
        setPath("");
        setPlaylist({f});
    }
    else if(Util::IsValidUrl(f)) // web url
    {
        setPath("");
        setPlaylist({f});
    }
    else // local file
    {
        QFileInfo fi(f);
        if(!fi.exists()) // file doesn't exist
        {
            ShowText(tr("File does not exist")); // tell the user
            return QString(); // don't do anything more
        }
        else if(fi.isDir()) // if directory
        {
            setPath(QDir::toNativeSeparators(fi.absoluteFilePath()+"/")); // set new path
            return PopulatePlaylist();
        }
        else if(fi.isFile()) // if file
        {
            setPath(QDir::toNativeSeparators(fi.absolutePath()+"/")); // set new path
            PopulatePlaylist();
            return fi.fileName();
        }
    }
    return f;
}

void MpvHandler::PlayFile(QString f)
{
    if(f == QString()) // ignore if file doesn't exist
        return;

    if(path == QString()) // web url
    {
        OpenFile(f);
        setFile(f);
    }
    else
    {
        QFile qf(path+f);
        if(qf.exists())
        {
            OpenFile(path+f);
            setFile(f);
            Play();
        }
        else
        {
            ShowText(tr("File no longer exists")); // tell the user
            // todo: refresh the playlist?
            Stop();
        }
    }
}

void MpvHandler::Play()
{
    if(playState > 0 && mpv)
    {
        int f = 0;
        mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &f);
    }
}

void MpvHandler::Pause()
{
    if(playState > 0 && mpv)
    {
        int f = 1;
        mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &f);
    }
}

void MpvHandler::Stop()
{
    Restart();
    Pause();
}

void MpvHandler::PlayPause(QString fileIfStopped)
{
    if(playState < 0) // not playing, play plays the selected playlist file
        PlayFile(fileIfStopped);
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

void MpvHandler::Mute(bool m)
{
    if(mute == m)
        return;

    if(playState > 0)
    {
        const char *args[] = {"osd-msg", "set", "mute", m ? "yes" : "no", NULL};
        AsyncCommand(args);
    }
    else
        setMute(m);
}

void MpvHandler::Seek(int pos, bool relative, bool osd)
{
    if(playState > 0)
    {
        if(relative)
        {
            const QByteArray tmp = (((pos >= 0) ? "+" : QString())+QString::number(pos)).toUtf8();
            if(osd)
            {
                const char *args[] = {"osd-msg", "seek", tmp.constData(), NULL};
                AsyncCommand(args);
            }
            else
            {
                const char *args[] = {"seek", tmp.constData(), NULL};
                AsyncCommand(args);
            }
        }
        else
        {
            const QByteArray tmp = QString::number(pos).toUtf8();
            if(osd)
            {
                const char *args[] = {"osd-msg", "seek", tmp.constData(), "absolute", NULL};
                AsyncCommand(args);
            }
            else
            {
                const char *args[] = {"seek", tmp.constData(), "absolute", NULL};
                AsyncCommand(args);
            }
        }
    }
}

int MpvHandler::Relative(int pos)
{
    int ret = pos - lastTime;
    lastTime = pos;
    return ret;
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
    if(mpv)
        mpv_set_property_async(mpv, 0, "chapter", MPV_FORMAT_INT64, &c);
//    const QByteArray tmp = QString::number(c).toUtf8();
//    const char *args[] = {"set", "chapter", tmp.constData(), NULL};
//    AsyncCommand(args);
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

void MpvHandler::Volume(int level, bool osd)
{
    if(level > 100) level = 100;
    else if(level < 0) level = 0;

    if(playState > 0)
    {
        if(osd)
        {
            QString levelStr = QString::number(level);
            const QByteArray tmp = levelStr.toUtf8();
            const char *args[] = {"set", "volume", tmp.constData(), NULL};
            AsyncCommand(args);
            ShowText(tr("Volume: %0%").arg(levelStr));
        }
        else
        {
            double v = level;
            mpv_set_property_async(mpv, 0, "volume", MPV_FORMAT_DOUBLE, &v);
        }
    }
    else
        setVolume(level);
}

void MpvHandler::Speed(double d)
{
    if(playState > 0)
        mpv_set_property_async(mpv, 0, "speed", MPV_FORMAT_DOUBLE, &d);
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
        const QByteArray tmp = (screenshotDir+"/"+s).toUtf8();
        mpv_set_option_string(mpv, "screenshot-template", tmp.data());
    }
    setScreenshotTemplate(s);
}

void MpvHandler::ScreenshotDirectory(QString s)
{
    setScreenshotDir(s);
}

void MpvHandler::AddSubtitleTrack(QString f)
{
    if(f == QString())
        return;
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"sub_add", tmp.constData(), NULL};
    Command(args);
    // this could be more efficient if we saved tracks in a bst
    auto old = fileInfo.tracks; // save the current track-list
    LoadTracks(); // load the new track list
    auto current = fileInfo.tracks;
    for(auto track : old) // remove the old tracks in current
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvHandler::ShowSubtitles(bool b)
{
    const char *args[] = {"set", "sub-visibility", b ? "yes" : "no", NULL};
    AsyncCommand(args);
}

void MpvHandler::SubtitleScale(double scale, bool relative)
{
    const QByteArray tmp = QString::number(scale).toUtf8();
    const char *args[] = {relative?"add":"set", "sub-scale", tmp.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::Debug(QString level)
{
    if(mpv)
    {
        QByteArray tmp = level.toUtf8();
        mpv_request_log_messages(mpv, tmp.constData());
    }
}

void MpvHandler::ShowText(QString text, int duration, int level)
{
    const QByteArray tmp1 = text.toUtf8(),
                     tmp2 = QString::number(duration).toUtf8(),
                     tmp3 = QString::number(level).toUtf8();
    const char *args[] = {"show_text", tmp1.constData(), tmp2.constData(), tmp3.constData(), NULL};
    AsyncCommand(args);
}

void MpvHandler::LoadFileInfo()
{
    // get media-title
    fileInfo.media_title = mpv_get_property_string(mpv, "media-title");
    // get length
    double len;
    mpv_get_property(mpv, "length", MPV_FORMAT_DOUBLE, &len);
    fileInfo.length                  = (int)len;

    fileInfo.video_params.codec      = mpv_get_property_string(mpv, "video-codec");
    fileInfo.video_params.bitrate    = mpv_get_property_string(mpv, "video-bitrate");
    fileInfo.audio_params.codec      = mpv_get_property_string(mpv, "audio-codec");
    fileInfo.audio_params.bitrate    = mpv_get_property_string(mpv, "audio-bitrate");
    fileInfo.audio_params.samplerate = mpv_get_property_string(mpv, "audio-samplerate");
    fileInfo.audio_params.channels   = mpv_get_property_string(mpv, "audio-channels");

    LoadTracks();
    LoadChapters();
    LoadVideoParams();
    LoadMetadata();

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

    emit trackListChanged(fileInfo.tracks);
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
    emit chaptersChanged(fileInfo.chapters);
}

void MpvHandler::LoadVideoParams()
{
    mpv_get_property(mpv, "width",        MPV_FORMAT_INT64, &fileInfo.video_params.width);
    mpv_get_property(mpv, "height",       MPV_FORMAT_INT64, &fileInfo.video_params.height);
    mpv_get_property(mpv, "dwidth",       MPV_FORMAT_INT64, &fileInfo.video_params.dwidth);
    mpv_get_property(mpv, "dheight",      MPV_FORMAT_INT64, &fileInfo.video_params.dheight);
    // though this has become useless, removing it causes a segfault--no clue:
    mpv_get_property(mpv, "video-aspect", MPV_FORMAT_INT64, &fileInfo.video_params.aspect);

    emit videoParamsChanged(fileInfo.video_params);
}

void MpvHandler::LoadMetadata()
{
    fileInfo.metadata.clear();
    mpv_node node;
    mpv_get_property(mpv, "metadata", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_MAP)
        for(int n = 0; n < node.u.list->num; n++)
            if(node.u.list->values[n].format == MPV_FORMAT_STRING)
                fileInfo.metadata[node.u.list->keys[n]] = node.u.list->values[n].u.string;
}

void MpvHandler::LoadOsdSize()
{
    if(mpv)
    {
        mpv_get_property(mpv, "osd-width", MPV_FORMAT_INT64, &osdWidth);
        mpv_get_property(mpv, "osd-height", MPV_FORMAT_INT64, &osdHeight);
    }
}

void MpvHandler::Command(const QStringList &strlist)
{
    // convert input string into char array
    int len = strlist.length();
    char **data = new char*[len+1];
    for(int i = 0; i < len; ++i)
    {
        data[i] = new char[strlist[i].length()+1];
        memcpy(data[i], QByteArray(strlist[i].toUtf8()).begin(), strlist[i].length()+1);
    }
    data[len] = NULL;
    AsyncCommand(const_cast<const char**>(data));
    for(int i = 0; i < len; ++i)
        delete [] data[i];
    delete [] data;

//    const QByteArray tmp = str.toUtf8();
//    mpv_command_string(mpv, tmp.constData());
}

void MpvHandler::SetOption(QString key, QString val)
{
    if(mpv)
    {
        QByteArray tmp1 = key.toUtf8(),
                   tmp2 = val.toUtf8();
        mpv_set_option_string(mpv, tmp1.constData(), tmp2.constData());
    }
}

void MpvHandler::OpenFile(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
    Command(args);
}

QString MpvHandler::PopulatePlaylist()
{
    if(path != "")
    {
        QStringList playlist;
        QDir root(path);
        QStringList filter = Mpv::media_filetypes;
        if(path != QString() && file != QString())
            filter.append(QString("*.%1").arg(file.split(".").last()));
        QFileInfoList flist;
        flist = root.entryInfoList(filter, QDir::Files);
        for(auto &i : flist)
            playlist.push_back(i.fileName()); // add files to the list
        setPlaylist(playlist);
        if(playlist.empty())
            return QString();
        return playlist.first();
    }
    return QString();
}

void MpvHandler::SetProperties()
{
    Volume(volume);
    Speed(speed);
    Mute(mute);
}

void MpvHandler::AsyncCommand(const char *args[])
{
    if(mpv)
        mpv_command_async(mpv, 0, args);
    else
        NotInitialized();
}

void MpvHandler::Command(const char *args[])
{
    if(mpv)
        mpv_command(mpv, args);
    else
        NotInitialized();
}

void MpvHandler::NotInitialized()
{
    //emit messageSignal(tr("mpv was not initialized\n"));
}
