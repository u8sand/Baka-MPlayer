#include "mpvhandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QRegExp>

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
                emit messageSignal(QString("[mpv]: %0").arg(mpv_error_string(event->error)));
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
                emit messageSignal(QString("[mpv]: %0").arg(static_cast<mpv_event_log_message*>(event->data)->text));
                break;
            default: // unhandled events
                break;
            }
        }
        return true;
    }
    return QObject::event(event);
}

void MpvHandler::LoadSettings(QSettings *settings, QString version)
{
    if(settings)
    {
        if(version == "2.0.2" || version == "2.0.1" || version == "2.0.0")
        {
            Debug(settings->value("baka-mplayer/debug", false).toBool());

            settings->beginGroup("mpv");
            for(auto &key : settings->childKeys())
            {
                if(key == "volume") // exception--we want to update our ui accordingly
                    Volume(settings->value(key).toInt());
                else if(key == "speed")
                    Speed(settings->value(key).toDouble());
                else if(key == "screenshot-template")
                {
                    QString temp = settings->value(key).toString();
                    int i = temp.lastIndexOf('/');
                    if(i != -1)
                    {
                        ScreenshotDirectory(temp.mid(0, i));
                        ScreenshotTemplate(temp.mid(i+1));
                    }
                    else
                    {
                        ScreenshotDirectory(".");
                        ScreenshotTemplate(temp);
                    }
                }
                else
                {
                    QByteArray tmp1 = key.toUtf8(),
                               tmp2;
                    // fix to qsettings reading comma-separation as a list
                    QVariant tmp = settings->value(key);
                    if(tmp.type() == QVariant::String)
                        tmp2 = tmp.toString().toUtf8();
                    else if(tmp.type() == QVariant::StringList)
                        tmp2 = tmp.toStringList().join(",").toUtf8();
                    else if(tmp.type() == QVariant::Int)
                        tmp2 = QString::number(tmp.toInt()).toUtf8();
                    else if(tmp.type() == QVariant::Double)
                        tmp2 = QString::number(tmp.toDouble()).toUtf8();
                    else
                        emit messageSignal(QString("[Baka-MPlayer]: %0\n").arg(tr("Setting type was parsed as %0").arg(tmp.type())));

                    if(tmp2 != QByteArray())
                        mpv_set_option_string(mpv, tmp1.constData(), tmp2.constData());
                }
            }
            settings->endGroup();
        }
        else if(version == "1.9.9")
        {
            settings->beginGroup("mpv");
            ScreenshotFormat(settings->value("screenshotFormat", "jpg").toString());
            ScreenshotDirectory(settings->value("screenshotDir", ".").toString());
            ScreenshotTemplate(settings->value("screenshotTemplate", tr("screenshot%#04n")).toString());
            Speed(settings->value("speed", 1.0).toDouble());
            Volume(settings->value("volume", 100).toInt());
            settings->endGroup();

            Debug(settings->value("common/debug", false).toBool());
        }

        if(!init)
        {
            // setup callback event handling
            mpv_set_wakeup_callback(mpv, wakeup, this);

            // initialize mpv
            if(mpv_initialize(mpv) < 0)
                throw "Could not initialize mpv";

            init = true;
        }
    }
}

bool MpvHandler::FileExists(QString f)
{
    QRegExp rx("^(https?://.+\\.[a-z]+)", Qt::CaseInsensitive);
    if(rx.indexIn(f) != -1) // web url
        return true;
    return QFile(f).exists();
}

void MpvHandler::SaveSettings(QSettings *settings)
{
    if(settings)
    {
        settings->setValue("mpv/volume", volume);
        settings->setValue("mpv/speed", speed);
        if(screenshotFormat != "")
            settings->setValue("mpv/screenshot-format", screenshotFormat);
        if(screenshotTemplate != "")
            settings->setValue("mpv/screenshot-template", screenshotDir+"/"+screenshotTemplate);
    }
}

void MpvHandler::LoadFile(QString f)
{
    if(f == "")
        return;
    PlayFile(LoadPlaylist(f));
}

QString MpvHandler::LoadPlaylist(QString f)
{
    if(f == "") // ignore empty file name
        return QString();

    QRegExp rx("^(https?://.+\\.[a-z]+)", Qt::CaseInsensitive);

    if(f == "-")
    {
        setPath("");
        setPlaylist({f});
    }
    else if(rx.indexIn(f) != -1) // web url
    {
        setPath("");
        setPlaylist({f});
    }
    else // local file
    {
        QFileInfo fi(f);
        if(!fi.exists()) // ignore if file doesn't exist
            return QString();
        else if(fi.isDir()) // if directory
        {
            setPath(QString(fi.absoluteFilePath()+"/")); // set new path
            return PopulatePlaylist();
        }
        else if(fi.isFile()) // if file
        {
            setPath(QString(fi.absolutePath()+"/")); // set new path
            PopulatePlaylist();
            return fi.fileName();
        }
    }
    return f;
}

void MpvHandler::PlayFile(QString f)
{
    if(path == "") // web url
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
            Stop();
    }
}

void MpvHandler::Play()
{
    if(playState > 0)
    {
        int f = 0;
        mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &f);
    }
}

void MpvHandler::Pause()
{
    if(playState > 0)
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
            const QByteArray tmp = QString::number(level).toUtf8();
            const char *args[] = {"osd-msg", "set", "volume", tmp.constData(), NULL};
            AsyncCommand(args);
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
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"sub_add", tmp.constData(), NULL};
    Command(args);
    LoadTracks(); // reload track list
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

void MpvHandler::Debug(bool b)
{
    if(mpv)
    {
        mpv_request_log_messages(mpv, b ? "debug" : "no");
        setDebug(b);
    }
}

void MpvHandler::ShowText(QString text, int duration, int level)
{
    if(mpv)
    {
        const QByteArray tmp1 = text.toUtf8(),
                         tmp2 = QString::number(duration).toUtf8(),
                         tmp3 = QString::number(level).toUtf8();
        const char *args[] = {"show_text", tmp1.constData(), tmp2.constData(), tmp3.constData(), NULL};
        AsyncCommand(args);
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

    fileInfo.video_params.codec = mpv_get_property_string(mpv, "video-codec");
    fileInfo.video_params.format = mpv_get_property_string(mpv, "video-format");
    fileInfo.video_params.bitrate = mpv_get_property_string(mpv, "video-bitrate");
    fileInfo.audio_params.codec = mpv_get_property_string(mpv, "audio-codec");
    fileInfo.audio_params.format = mpv_get_property_string(mpv, "audio-format");
    fileInfo.audio_params.bitrate = mpv_get_property_string(mpv, "audio-bitrate");
    fileInfo.audio_params.samplerate = mpv_get_property_string(mpv, "audio-samplerate");
    fileInfo.audio_params.channels = mpv_get_property_string(mpv, "audio-channels");

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
    mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &fileInfo.video_params.width);
    mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &fileInfo.video_params.height);
    mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &fileInfo.video_params.dwidth);
    mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &fileInfo.video_params.dheight);
    mpv_get_property(mpv, "video-aspect", MPV_FORMAT_INT64, &fileInfo.video_params.aspect);

    emit videoParamsChanged(fileInfo.video_params);
}

void MpvHandler::CommandString(QString str)
{
    const QByteArray tmp = str.toUtf8();
    mpv_command_string(mpv, tmp.constData());
}

void MpvHandler::OpenFile(QString f)
{
    const QByteArray tmp = f.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
    AsyncCommand(args);
}

QString MpvHandler::PopulatePlaylist()
{
    if(path != "")
    {
        QStringList playlist;
        QDir root(path);
        QFileInfoList flist;
        flist = root.entryInfoList(Mpv::media_filetypes, QDir::Files);
        for(auto &i : flist)
            playlist.push_back(i.fileName()); // add files to the list
        setPlaylist(playlist);
        if(playlist.length() > 1)
            setPlaylistVisible(true);
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
    emit messageSignal(QString("[mpv]: %0\n").arg(tr("mpv was not initialized")));
}
