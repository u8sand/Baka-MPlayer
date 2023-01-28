#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>

#include "bakaengine.h"
#include "mpvtypes.h"
#include "overlayhandler.h"
#include "util.h"
#include "mpvhandler.h"
#include <stdexcept>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>

#include <QElapsedTimer>
#include <QDebug>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvHandler*)ctx, "onMpvEvents", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

static inline QVariant node_to_variant(const mpv_node *node)
{
    switch (node->format) {
    case MPV_FORMAT_STRING:
        return QVariant(QString::fromUtf8(node->u.string));
    case MPV_FORMAT_FLAG:
        return QVariant(static_cast<bool>(node->u.flag));
    case MPV_FORMAT_INT64:
        return QVariant(static_cast<qlonglong>(node->u.int64));
    case MPV_FORMAT_DOUBLE:
        return QVariant(node->u.double_);
    case MPV_FORMAT_NODE_ARRAY: {
        mpv_node_list *list = node->u.list;
        QVariantList qlist;
        for (int n = 0; n < list->num; n++)
            qlist.append(node_to_variant(&list->values[n]));
        return QVariant(qlist);
    }
    case MPV_FORMAT_NODE_MAP: {
        mpv_node_list *list = node->u.list;
        QVariantMap qmap;
        for (int n = 0; n < list->num; n++) {
            qmap.insert(QString::fromUtf8(list->keys[n]),
                        node_to_variant(&list->values[n]));
        }
        return QVariant(qmap);
    }
    default: // MPV_FORMAT_NONE, unknown values (e.g. future extensions)
        return QVariant();
    }
}

static inline QVariant mpv_data_as_variant(void *data, mpv_format format) {
    if (data == nullptr) {
        return QVariant();
    }
    switch(format) {
    case MPV_FORMAT_STRING:
        return QVariant(QString::fromUtf8(static_cast<const char*>(data)));
    case MPV_FORMAT_FLAG:
        return QVariant(*static_cast<bool*>(data));
    case MPV_FORMAT_INT64:
        return QVariant(*static_cast<qlonglong*>(data));
    case MPV_FORMAT_DOUBLE:
        return QVariant(*static_cast<double*>(data));
    case MPV_FORMAT_NODE:
        return node_to_variant(static_cast<mpv_node*>(data));
    default:
        return QVariant();
    }
}

MpvHandler::MpvHandler(QWidget *parent):
    QOpenGLWidget(parent)
{
    // create mpv
    mpv = mpv_create();
    if(!mpv)
        throw "Could not create mpv object";

    // set mpv options
    mpv_set_option_string(mpv, "input-cursor", "no");   // no mouse handling
    mpv_set_option_string(mpv, "cursor-autohide", "no");// no cursor-autohide, we handle that
    mpv_set_option_string(mpv, "ytdl", "yes"); // youtube-dl support
    mpv_set_option_string(mpv, "sub-auto", "fuzzy"); // Automatic subfile detection
    mpv_set_option_string(mpv, "audio-client-name", "baka-mplayer"); // show correct icon in e.g. pavucontrol
    mpv_set_option_string(mpv, "hwdec", "auto");

    // get updates when these properties change
    ObserveProperty("playback-time", MPV_FORMAT_DOUBLE, [=](QVariant &prop) {
        setTime((int)prop.toDouble());
        lastTime = time;
    });
    ObserveProperty("duration", MPV_FORMAT_DOUBLE, [=](QVariant &prop) {
        fileInfo.length = prop.toDouble();
        setTime(time);
    });
    ObserveProperty("volume", MPV_FORMAT_DOUBLE, [=](QVariant &prop) {
        setVolume((int)prop.toDouble());
    });
    ObserveProperty("sid", MPV_FORMAT_INT64, [=](QVariant &prop) {
        setSid(prop.toULongLong());
    });
    ObserveProperty("aid", MPV_FORMAT_INT64, [=](QVariant &prop) {
        setAid(prop.toULongLong());
    });
    ObserveProperty("sub-visibility", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        setSubtitleVisibility(prop.toBool());
    });
    ObserveProperty("mute", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        setMute(prop.toBool());
    });
    ObserveProperty("core-idle", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        bool coreIdle = prop.toBool();
        if(coreIdle && playState == Mpv::Playing)
            ShowText(tr("Buffering..."), 0);
        else
            ShowText(QString(), 0);
    });
    ObserveProperty("idle-active", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        bool idleActive = prop.toBool();
        if(idleActive)
        {
            fileInfo.length = 0;
            setTime(0);
            setPlayState(Mpv::Idle);
        }
    });
    ObserveProperty("pause", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        bool pause = prop.toBool();
        if(pause)
        {
            setPlayState(Mpv::Paused);
            ShowText(QString(), 0);
        }
        else
            setPlayState(Mpv::Playing);
    });
    ObserveProperty("paused-for-cache", MPV_FORMAT_FLAG, [=](QVariant &prop) {
        bool pausedForCache = prop.toBool();
        if(pausedForCache && playState == Mpv::Playing)
            ShowText(tr("Your network is slow or stuck, please wait a bit"), 0);
        else
            ShowText(QString(), 0);
    });

    // setup callback event handling
    mpv_set_wakeup_callback(mpv, wakeup, this);

    connect(this, &MpvHandler::playStateChanged,
            [=](Mpv::PlayState playState)
            {
                switch(playState)
                {
                case Mpv::Started:
                    this->LoadFileInfo();
                    setPlayState(Mpv::Playing);
                    break;
                default:
                    break;
                }
            });
}

MpvHandler::~MpvHandler()
{
    makeCurrent();
    if (mpv_gl)
        mpv_render_context_free(mpv_gl);
    mpv_terminate_destroy(mpv);
}

void MpvHandler::Initialize(BakaEngine *baka)
{
    if(mpv_initialize(mpv) < 0)
        throw "Could not initialize mpv";

    this->baka = baka;
}

void MpvHandler::initializeGL()
{
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    int advancedControlEnable = 1;
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advancedControlEnable}
    };

    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, MpvHandler::on_update, reinterpret_cast<void *>(this));
}

void MpvHandler::paintGL()
{
    mpv_opengl_fbo mpfbo{static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    int flip_y{1};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
}

void MpvHandler::getMediaInfo(std::function<void (QString)> cb)
{
    QFileInfo fi(path+file);
    QList<Mpv::Property> properties{
        {"avsync", MPV_FORMAT_DOUBLE},
        {"estimated-vf-fps", MPV_FORMAT_DOUBLE},
        {"video-bitrate", MPV_FORMAT_DOUBLE},
        {"audio-bitrate", MPV_FORMAT_DOUBLE},
        {"current-vo", MPV_FORMAT_STRING},
        {"current-ao", MPV_FORMAT_STRING},
        {"hwdec-active", MPV_FORMAT_STRING}
    };
    GetProperties(properties, [=](QSharedPointer<QVariantMap> property_values) {
        double avsync = property_values->value("avsync").toDouble();
        double fps = property_values->value("estimated-vf-fps").toDouble();
        double vbitrate = property_values->value("video-bitrate").toDouble();
        double abitrate = property_values->value("audio-bitrate").toDouble();
        QString current_vo = property_values->value("current-vo").toString();
        QString current_ao = property_values->value("current-ao").toString();
        QString hwdec_active = property_values->value("hwdec-active").toString();
        int vtracks = 0,
            atracks = 0;

        for(auto &track : fileInfo.tracks)
        {
            if(track.type == "video")
                ++vtracks;
            else if(track.type == "audio")
                ++atracks;
        }

        const QString outer = "%0: %1\n", inner = "    %0: %1\n";

        QString out = outer.arg(tr("File"), fi.fileName()) +
                inner.arg(tr("Title"), fileInfo.media_title) +
                inner.arg(tr("File size"), Util::HumanSize(fi.size())) +
                inner.arg(tr("Date created"), fi.birthTime().toString()) +
                inner.arg(tr("Media length"), Util::FormatTime(fileInfo.length, fileInfo.length)) + '\n';
        if(fileInfo.video_params.codec != QString())
            out += outer.arg(tr("Video (x%0)").arg(QString::number(vtracks)), fileInfo.video_params.codec) +
                inner.arg(tr("Video Output"), QString("%0 (hwdec %1)").arg(current_vo, hwdec_active)) +
                inner.arg(tr("Resolution"), QString("%0 x %1 (%2)").arg(QString::number(fileInfo.video_params.width),
                                                                        QString::number(fileInfo.video_params.height),
                                                                        Util::Ratio(fileInfo.video_params.width, fileInfo.video_params.height))) +
                inner.arg(tr("FPS"), QString::number(fps)) +
                inner.arg(tr("A/V Sync"), QString::number(avsync)) +
                inner.arg(tr("Bitrate"), tr("%0 kbps").arg(vbitrate/1000)) + '\n';
        if(fileInfo.audio_params.codec != QString())
            out += outer.arg(tr("Audio (x%0)").arg(QString::number(atracks)), fileInfo.audio_params.codec) +
                inner.arg(tr("Audio Output"), current_ao) +
                inner.arg(tr("Sample Rate"), QString::number(fileInfo.audio_params.samplerate)) +
                inner.arg(tr("Channels"), QString::number(fileInfo.audio_params.channels)) +
                inner.arg(tr("Bitrate"), tr("%0 kbps").arg(abitrate)) + '\n';

        if(fileInfo.chapters.length() > 0)
        {
            out += outer.arg(tr("Chapters"), QString());
            int n = 1;
            for(auto &chapter : fileInfo.chapters)
                out += inner.arg(QString::number(n++), chapter.title);
            out += '\n';
        }

        if(fileInfo.metadata.size() > 0)
        {
            out += outer.arg(tr("Metadata"), QString());
            for(auto data = fileInfo.metadata.begin(); data != fileInfo.metadata.end(); ++data)
                out += inner.arg(data.key(), *data);
            out += '\n';
        }

        cb(out);
    });
}

void MpvHandler::onMpvEvents()
{
    while(mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if(event == nullptr ||
            event->event_id == MPV_EVENT_NONE)
        {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvHandler::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id)
    {
    case MPV_EVENT_COMMAND_REPLY:
    {
        // qDebug() << "commandReply " << event->reply_userdata;
        auto cb = reply_callbacks.find(event->reply_userdata);
        if (cb != reply_callbacks.end())
        {
            cb.value()(event);
            reply_callbacks.remove(event->reply_userdata);
        }
        break;
    }
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        // qDebug() << "propertyChange " << event->reply_userdata;
        auto cb = reply_callbacks.find(event->reply_userdata);
        if (cb != reply_callbacks.end())
        {
            cb.value()(event);
        }
        break;
    }
    // these two look like they're reversed but they aren't. the names are misleading.
    case MPV_EVENT_START_FILE:
        setPlayState(Mpv::Loaded);
        break;
    case MPV_EVENT_FILE_LOADED:
        setPlayState(Mpv::Started);
        break;
    case MPV_EVENT_END_FILE:
        if(playState == Mpv::Loaded)
            ShowText(tr("File couldn't be opened"));
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

// Make Qt invoke mpv_render_context_render() to draw a new/updated video frame.
void MpvHandler::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's render API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    makeCurrent();
    mpv_render_context_update(mpv_gl);
    paintGL();
    context()->swapBuffers(context()->surface());
    doneCurrent();
}

void MpvHandler::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvHandler*)ctx, "maybeUpdate");
}

void MpvHandler::AddOverlay(int id, int x, int y, QString file, int offset, int w, int h)
{
    Command({
        "overlay_add",
        QString::number(id),
        QString::number(x),
        QString::number(y),
        file,
        QString::number(offset),
        "bgra",
        QString::number(w),
        QString::number(h),
        QString::number(4*w)
    });
}

void MpvHandler::RemoveOverlay(int id)
{
    Command({"overlay_remove", QString::number(id)});
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

bool MpvHandler::PlayFile(QString f)
{
    if(f == QString()) // ignore if file doesn't exist
        return false;

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
            return false;
        }
    }
    return true;
}

void MpvHandler::Play()
{
    if(playState > 0 && mpv)
    {
        int f = 0;
        mpv_set_property_async(mpv, reply_userdata++, "pause", MPV_FORMAT_FLAG, &f);
    }
}

void MpvHandler::Pause()
{
    if(playState > 0 && mpv)
    {
        int f = 1;
        mpv_set_property_async(mpv, reply_userdata++, "pause", MPV_FORMAT_FLAG, &f);
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
        Command({"cycle", "pause"});
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
    if(playState > 0)
    {
        Command({"set", "mute", m ? "yes" : "no"});
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
            const QString tmp = ((pos >= 0) ? "+" : QString())+QString::number(pos);
            if(osd)
            {
                Command({"osd-msg", "seek", tmp});
            }
            else
            {
                Command({"seek", tmp});
            }
        }
        else
        {
            if(osd)
            {
                Command({"osd-msg", "seek", QString::number(pos), "absolute"});
            }
            else
            {
                Command({"seek", QString::number(pos), "absolute"});
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
    Command({"frame-step"});
}

void MpvHandler::FrameBackStep()
{
    Command({"frame-back-step"});
}

void MpvHandler::Chapter(int c)
{
    mpv_set_property_async(mpv, reply_userdata++, "chapter", MPV_FORMAT_INT64, &c);
//    const QByteArray tmp = QString::number(c).toUtf8();
//    const char *args[] = {"set", "chapter", tmp.constData(), NULL};
//    Command(args);
}

void MpvHandler::NextChapter()
{
    Command({"add", "chapter", "1"});
}

void MpvHandler::PreviousChapter()
{
    Command({"add", "chapter", "-1"});
}

void MpvHandler::Volume(int level, bool osd)
{
    if(level > 100) level = 100;
    else if(level < 0) level = 0;
    double v = level;

    if(playState > 0)
    {
        mpv_set_property_async(mpv, reply_userdata++, "volume", MPV_FORMAT_DOUBLE, &v);
        if(osd)
            ShowText(tr("Volume: %0%").arg(QString::number(level)));
    }
    else
    {
        mpv_set_option(mpv, "volume", MPV_FORMAT_DOUBLE, &v);
        setVolume(level);
    }
}

void MpvHandler::Speed(double d)
{
    if(playState > 0)
        mpv_set_property_async(mpv, reply_userdata++, "speed", MPV_FORMAT_DOUBLE, &d);
    setSpeed(d);
}

void MpvHandler::Aspect(QString aspect)
{
    Command({"set", "video-aspect-override", aspect});
}


void MpvHandler::Vid(int vid)
{
    Command({"set", "vid", QString::number(vid)});
}

void MpvHandler::Aid(int aid)
{
    Command({"set", "aid", QString::number(aid)});
}

void MpvHandler::Sid(int sid)
{
    Command({"set", "sid", QString::number(sid)});
}

void MpvHandler::Screenshot(bool withSubs)
{
    Command({"screenshot", (withSubs ? "subtitles" : "video")});
}

void MpvHandler::ScreenshotFormat(QString s)
{
    SetOption("screenshot-format", s);
    setScreenshotFormat(s);
}

void MpvHandler::ScreenshotTemplate(QString s)
{
    SetOption("screenshot-template", s);
    setScreenshotTemplate(s);
}

void MpvHandler::ScreenshotDirectory(QString s)
{
    SetOption("screenshot-directory", s);
    setScreenshotDir(s);
}

void MpvHandler::AddSubtitleTrack(QString f)
{
    if(f == QString())
        return;
    Command({"sub-add", f});
    // this could be more efficient if we saved tracks in a bst
    auto old = fileInfo.tracks; // save the current track-list
    LoadTracks(); // load the new track list
    auto current = fileInfo.tracks;
    for(auto &track : old) // remove the old tracks in current
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvHandler::AddAudioTrack(QString f)
{
    if(f == QString())
        return;
    Command({"audio-add", f});
    auto old = fileInfo.tracks;
    LoadTracks();
    auto current = fileInfo.tracks;
    for(auto &track : old)
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvHandler::ShowSubtitles(bool b)
{
    Command({"set", "sub-visibility", b ? "yes" : "no"});
}

void MpvHandler::SubtitleScale(double scale, bool relative)
{
    Command({relative?"add":"set", "sub-scale", QString::number(scale)});
}

void MpvHandler::Deinterlace(bool deinterlace)
{
    HandleErrorCode(mpv_set_property_string(mpv, "deinterlace", deinterlace ? "yes" : "auto"), "deinterlace");
    ShowText(tr("Deinterlacing: %0").arg(deinterlace ? tr("enabled") : tr("disabled")));
}

void MpvHandler::Interpolate(bool interpolate)
{
    GetProperty("current-vo", MPV_FORMAT_STRING, [=](const QVariant &prop) {
        vo = prop.toString();
        QStringList vos = vo.split(',');
        for(auto &o : vos)
        {
            int i = o.indexOf(":interpolation");
            if(interpolate && i == -1)
                o.append(":interpolation");
            else if(i != -1)
                o.remove(i, QString(":interpolation").length());
        }
        setVo(vos.join(','));
        SetOption("vo", vo);
        ShowText(tr("Motion Interpolation: %0").arg(interpolate ? tr("enabled") : tr("disabled")));
    });
}

void MpvHandler::Vo(QString o)
{
    setVo(o);
    SetOption("vo", vo);
}

void MpvHandler::MsgLevel(QString level)
{
    QByteArray tmp = level.toUtf8();
    mpv_request_log_messages(mpv, tmp.constData());
    setMsgLevel(level);
}

void MpvHandler::ShowText(QString text, int duration)
{
    baka->overlay->showStatusText(text, duration);
    /*
    const QByteArray tmp1 = text.toUtf8(),
                     tmp2 = QString::number(duration).toUtf8(),
                     tmp3 = QString::number(level).toUtf8();
    const char *args[] = {"show_text", tmp1.constData(), tmp2.constData(), tmp3.constData(), NULL};
    Command(args);
    */
}

void MpvHandler::LoadFileInfo()
{
    QList<Mpv::Property> properties = {
        {"media-title", MPV_FORMAT_STRING},
        {"track-list", MPV_FORMAT_NODE},
        {"chapter-list", MPV_FORMAT_NODE},
        // video params
        {"video-codec", MPV_FORMAT_STRING},
        {"width", MPV_FORMAT_INT64},
        {"height", MPV_FORMAT_INT64},
        {"dwidth", MPV_FORMAT_INT64},
        {"dheight", MPV_FORMAT_INT64},
        {"video-params/aspect", MPV_FORMAT_INT64},
        // audio params
        {"audio-codec", MPV_FORMAT_STRING},
        {"audio-params", MPV_FORMAT_NODE},
        // metadata
        {"metadata", MPV_FORMAT_NODE}
    };
    GetProperties(properties, [=] (QSharedPointer<QVariantMap> property_values) {
        fileInfo.media_title = property_values->value("media-title").toString();
        fileInfo.tracks.clear();
        for (auto &rawTrack : property_values->value("track-list").toList()) {
            auto rawTrackMap = rawTrack.toMap();
            Mpv::Track track;
            track.id = rawTrackMap.value("id").toLongLong();
            track.type= rawTrackMap.value("type").toString();
            track.src_id = rawTrackMap.value("src-id").toLongLong();
            track.title = rawTrackMap.value("title").toString();
            track.lang = rawTrackMap.value("lang").toString();
            track.albumart = rawTrackMap.value("albumart").toBool();
            track._default = rawTrackMap.value("default").toBool();
            track.external = rawTrackMap.value("external").toBool();
            track.external_filename = rawTrackMap.value("external-filename").toString();
            track.codec = rawTrackMap.value("codec").toString();
            fileInfo.tracks.push_back(track);
        }
        emit trackListChanged(fileInfo.tracks);

        fileInfo.chapters.clear();
        for (auto &rawChapter : property_values->value("chapter-list").toList()) {
            auto rawChapterMap = rawChapter.toMap();
            Mpv::Chapter ch;
            ch.title = rawChapterMap.value("title").toString();
            ch.time = rawChapterMap.value("time").toDouble();
            fileInfo.chapters.push_back(ch);
        }
        emit chaptersChanged(fileInfo.chapters);

        fileInfo.video_params.codec = property_values->value("video-codec").toString();
        fileInfo.video_params.width = property_values->value("width").toLongLong();
        fileInfo.video_params.height = property_values->value("height").toLongLong();
        fileInfo.video_params.dwidth = property_values->value("dwidth").toLongLong();
        fileInfo.video_params.dheight = property_values->value("dheight").toLongLong();
        // though this has become useless, removing it causes a segfault--no clue:
        fileInfo.video_params.aspect = property_values->value("video-params/aspect").toLongLong();
        emit videoParamsChanged(fileInfo.video_params);

        fileInfo.audio_params.codec = property_values->value("audio-codec").toString();
        fileInfo.audio_params.samplerate = property_values->value("audio-params").toList().first().toMap().value("samplerate").toLongLong();
        fileInfo.audio_params.channels = property_values->value("audio-params").toList().first().toMap().value("channel-count").toLongLong();
        emit audioParamsChanged(fileInfo.audio_params);

        fileInfo.metadata.clear();
        for (auto &rawMetadataKey : property_values->value("metadata").toMap().keys()) {
            fileInfo.metadata[rawMetadataKey] = property_values->value("metadata").toMap().value(rawMetadataKey).toString();
        }

        emit fileInfoChanged(fileInfo);
        Volume(volume);
        Speed(speed);
        Mute(mute);
    });
}

void MpvHandler::LoadTracks() {
    GetProperty("track-list", MPV_FORMAT_NODE, [=] (const QVariant &prop) {
        QList<QVariant> rawTrackList = prop.toList();
        fileInfo.tracks.clear();
        for (auto &rawTrack : rawTrackList) {
            auto rawTrackMap = rawTrack.toMap();
            Mpv::Track track;
            track.id = rawTrackMap.value("id").toLongLong();
            track.type= rawTrackMap.value("type").toString();
            track.src_id = rawTrackMap.value("src-id").toLongLong();
            track.title = rawTrackMap.value("title").toString();
            track.lang = rawTrackMap.value("lang").toString();
            track.albumart = rawTrackMap.value("albumart").toBool();
            track._default = rawTrackMap.value("default").toBool();
            track.external = rawTrackMap.value("external").toBool();
            track.external_filename = rawTrackMap.value("external-filename").toString();
            track.codec = rawTrackMap.value("codec").toString();
            fileInfo.tracks.push_back(track);
        }

        emit trackListChanged(fileInfo.tracks);
    });
}

void MpvHandler::LoadVideoParams(std::function<void ()> cb) {
    QList<Mpv::Property> properties{
        {"video-codec", MPV_FORMAT_STRING},
        {"width", MPV_FORMAT_INT64},
        {"height", MPV_FORMAT_INT64},
        {"dwidth", MPV_FORMAT_INT64},
        {"dheight", MPV_FORMAT_INT64},
        {"video-params/aspect", MPV_FORMAT_INT64}
    };
    GetProperties(properties, [=](QSharedPointer<QVariantMap> property_values) {
        fileInfo.video_params.codec = property_values->value("video-codec").toString();
        fileInfo.video_params.width = property_values->value("width").toLongLong();
        fileInfo.video_params.height = property_values->value("height").toLongLong();
        fileInfo.video_params.dwidth = property_values->value("dwidth").toLongLong();
        fileInfo.video_params.dheight = property_values->value("dheight").toLongLong();
        // though this has become useless, removing it causes a segfault--no clue:
        fileInfo.video_params.aspect = property_values->value("video-params/aspect").toLongLong();
        emit videoParamsChanged(fileInfo.video_params);
        cb();
    });
}

void MpvHandler::SetOption(QString key, QString val)
{
    QByteArray tmp1 = key.toUtf8(),
               tmp2 = val.toUtf8();
    HandleErrorCode(mpv_set_option_string(mpv, tmp1.constData(), tmp2.constData()), "set option");
}

void MpvHandler::OpenFile(QString f)
{
    emit fileChanging(time, fileInfo.length);
    Command({"loadfile", f});
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

void MpvHandler::Command(const QStringList &args, std::function<void (mpv_event *)> on_event)
{
    const int *nargs = new int(args.length());
    char **cargs = new char*[*nargs+1];
    for (int i = 0; i < args.length(); i++) {
        QString arg = args.at(i);
        cargs[i] = new char[arg.length()+1];
        memcpy(cargs[i], arg.toUtf8().constData(), arg.length());
        cargs[i][arg.length()] = '\0';
    }
    cargs[*nargs] = NULL;
    const unsigned long id = reply_userdata++;
    reply_callbacks[id] = [=](mpv_event *event) {
        if (!HandleErrorCode(event->error, QString(args.join(" ")) + " command reply")) {
            if (on_event != nullptr) {
                on_event(event);
            }
        }
        for (int i = 0; i < *nargs; i++) {
            delete [] cargs[i];
        }
        delete [] cargs;
    };
    // qDebug() << "sendCommand " << args.join(" ") << " (" << id << ")";
    int error = mpv_command_async(mpv, id, (const char **)cargs);
    if (HandleErrorCode(error, QString(args.join(" "))))
    {
        mpv_event error_event{MPV_EVENT_NONE, error, id, nullptr};
        reply_callbacks[id](&error_event);
        reply_callbacks.remove(id);
    }
}

void MpvHandler::ObserveProperty(const char *name, mpv_format format, std::function<void (QVariant&)> on_property)
{
    const unsigned long id = reply_userdata++;
    reply_callbacks[id] = [=](mpv_event *event) {
        if (!HandleErrorCode(event->error, QString("get prop %0").arg(name))) {
            mpv_event_property *prop = (mpv_event_property*)event->data;
            QVariant propValue = mpv_data_as_variant(prop->data, format);
            on_property(propValue);
            // qDebug() << "observedProperty " << name << "(" << id << ")";
        }
    };
    // qDebug() << "observeProperty " << name << "(" << id << ")";
    HandleErrorCode(mpv_observe_property(mpv, id, name, format), QString("observe prop %0").arg(name));
}

void MpvHandler::GetProperty(const char *name, mpv_format format, std::function<void (QVariant&)> on_property)
{
    const unsigned long id = reply_userdata++;
    reply_callbacks[id] = [=](mpv_event *event) {
        if (!HandleErrorCode(event->error, QString("get prop %0").arg(name))) {
            mpv_event_property *prop = (mpv_event_property*)event->data;
            QVariant propValue = mpv_data_as_variant(prop->data, format);
            on_property(propValue);
            // qDebug() << "recvdProperty " << prop->name;
        }
        reply_callbacks.remove(id);
    };
    // qDebug() << "getProperty " << name << "(" << id << ")";
    int error = mpv_get_property_async(mpv, id, name, format);
    if (HandleErrorCode(error, QString("get prop %0").arg(name)))
    {
        QVariant prop = QVariant();
        on_property(prop);
        reply_callbacks.remove(id);
    }
}

void MpvHandler::GetProperties(const QList<Mpv::Property> &properties, std::function<void (QSharedPointer<QVariantMap>)> on_event)
{
    QSharedPointer<QVariantMap> property_values(new QVariantMap());
    // qDebug() << "getProperties " << property_values->keys().join(",");
    for (const Mpv::Property &prop : properties)
    {
        GetProperty(prop.name, prop.format, [=, n_properties = properties.length()](const QVariant &propValue) {
            property_values->insert(prop.name, QVariant(propValue));
            if (property_values->keys().length() == n_properties) {
                // qDebug() << "recvdProperties " << property_values->keys().join(",");
                on_event(property_values);
            }
        });
    }
}

bool MpvHandler::HandleErrorCode(int error_code, QString ctx)
{
    if(error_code >= 0)
        return false;
    QString error = mpv_error_string(error_code);
    if(error != QString())
        emit messageSignal("["+ctx+"]"+error+"\n");
    return true;
}
