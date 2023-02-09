#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QtWidgets/QOpenGLWidget>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QSharedPointer>
#include <mpv/client.h>
#include <mpv/render_gl.h>

#include "mpvtypes.h"

#define MPV_REPLY_COMMAND 1
#define MPV_REPLY_PROPERTY 2

class MpvHandler Q_DECL_FINAL: public QOpenGLWidget
{
friend class BakaEngine;
    Q_OBJECT
public:
    explicit MpvHandler(QWidget *parent = 0);
    ~MpvHandler();

    void Initialize();
    const Mpv::FileInfo &getFileInfo()      { return fileInfo; }
    Mpv::PlayState getPlayState()           { return playState; }
    QString getFile()                       { return file; }
    QString getPath()                       { return path; }
    QString getScreenshotFormat()           { return screenshotFormat; }
    QString getScreenshotTemplate()         { return screenshotTemplate; }
    QString getScreenshotDir()              { return screenshotDir; }
    QString getVo()                         { return vo; }
    QString getMsgLevel()                   { return msgLevel; }
    double getSpeed()                       { return speed; }
    int getTime()                           { return time; }
    int getVolume()                         { return volume; }
    int getVid()                            { return vid; }
    int getAid()                            { return aid; }
    int getSid()                            { return sid; }
    bool getSubtitleVisibility()            { return subtitleVisibility; }
    bool getMute()                          { return mute; }

    int getOsdWidth()                       { return osdWidth; }
    int getOsdHeight()                      { return osdHeight; }

    void getMediaInfo(std::function<void (QString)> cb);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    bool FileExists(QString);

public slots:
    void LoadFile(QString);
    QString LoadPlaylist(QString);
    bool PlayFile(QString);

    void AddOverlay(int id, int x, int y, QString file, int offset, int w, int h);
    void RemoveOverlay(int id);

    void Play();
    void Pause();
    void Stop();
    void PlayPause(QString fileIfStopped);
    void Restart();
    void Rewind();
    void Mute(bool);

    void Seek(int pos, bool relative = false, bool osd = false);
    int Relative(int pos);
    void FrameStep();
    void FrameBackStep();

    void Chapter(int);
    void NextChapter();
    void PreviousChapter();

    void Volume(int, bool osd = false);
    void Speed(double);
    void Aspect(QString);
    void Vid(int);
    void Aid(int);
    void Sid(int);

    void Screenshot(bool withSubs = false);

    void ScreenshotFormat(QString);
    void ScreenshotTemplate(QString);
    void ScreenshotDirectory(QString);

    void AddSubtitleTrack(QString);
    void AddAudioTrack(QString);
    void ShowSubtitles(bool);
    void SubtitleScale(double scale, bool relative = false);

    void Deinterlace(bool);
    void Interpolate(bool);
    void Vo(QString);

    void MsgLevel(QString level);

    void LoadTracks();
    void LoadVideoParams(std::function<void ()> cb);

    void Command(const QStringList &strlist, std::function<void (mpv_event *)> on_event = nullptr);
    void SetOption(QString key, QString val);

protected slots:
    void OpenFile(QString);
    QString PopulatePlaylist();
    void LoadFileInfo();

    void ObserveProperty(const char *name, mpv_format format, std::function<void (QVariant &)> on_property);
    void GetProperty(const char *name, mpv_format format, std::function<void (QVariant &)> on_property);
    void GetProperties(const QList<Mpv::Property> &properties, std::function<void (QSharedPointer<QVariantMap>)> on_event);
    bool HandleErrorCode(int,  QString ctx = "unknown");

private Q_SLOTS:
    void onMpvEvents();
    void handle_mpv_event(mpv_event *event);
    void maybeUpdate();

    void setPlaylist(const QStringList& l)  { emit playlistChanged(l); }
    void setFileInfo()                      { emit fileInfoChanged(fileInfo); }
    void setPlayState(Mpv::PlayState s)     { emit playStateChanged(playState = s); }
    void setFile(QString s)                 { emit fileChanged(file = s); }
    void setPath(QString s)                 { emit pathChanged(path = s); }
    void setScreenshotFormat(QString s)     { emit screenshotFormatChanged(screenshotFormat = s); }
    void setScreenshotTemplate(QString s)   { emit screenshotTemplateChanged(screenshotTemplate = s); }
    void setScreenshotDir(QString s)        { emit screenshotDirChanged(screenshotDir = s); }
    void setVo(QString s)                   { emit voChanged(vo = s); }
    void setMsgLevel(QString s)             { emit msgLevelChanged(msgLevel = s); }
    void setSpeed(double d)                 { emit speedChanged(speed = d); }
    void setTime(int i)                     { emit timeChanged(time = i); }
    void setVolume(int i)                   { emit volumeChanged(volume = i); }
    void setIndex(int i)                    { emit indexChanged(index = i); }
    void setVid(int i)                      { emit vidChanged(vid = i); }
    void setAid(int i)                      { emit aidChanged(aid = i); }
    void setSid(int i)                      { emit sidChanged(sid = i); }
    void setSubtitleVisibility(bool b)      { emit subtitleVisibilityChanged(subtitleVisibility = b); }
    void setMute(bool b)                    { if(mute != b) emit muteChanged(mute = b); }

signals:
    void playlistChanged(const QStringList&);
    void fileInfoChanged(const Mpv::FileInfo&);
    void trackListChanged(const QList<Mpv::Track>&);
    void chaptersChanged(const QList<Mpv::Chapter>&);
    void videoParamsChanged(const Mpv::VideoParams&);
    void audioParamsChanged(const Mpv::AudioParams&);
    void playStateChanged(Mpv::PlayState);
    void fileChanging(int, int);
    void fileChanged(QString);
    void pathChanged(QString);
    void screenshotFormatChanged(QString);
    void screenshotTemplateChanged(QString);
    void screenshotDirChanged(QString);
    void voChanged(QString);
    void msgLevelChanged(QString);
    void speedChanged(double);
    void timeChanged(int);
    void volumeChanged(int);
    void indexChanged(int);
    void vidChanged(int);
    void aidChanged(int);
    void sidChanged(int);
    void debugChanged(bool);
    void subtitleVisibilityChanged(bool);
    void muteChanged(bool);

    void showText(QString m, int duration = 4000);
    void messageSignal(QString m);

private:
    static void on_update(void *ctx);

    mpv_handle *mpv;
    mpv_render_context *mpv_gl;

    // variables
    Mpv::PlayState playState = Mpv::Idle;
    Mpv::FileInfo fileInfo;
    QString     file,
                path,
                screenshotFormat,
                screenshotTemplate,
                screenshotDir,
                suffix,
                vo,
                msgLevel;
    double      speed = 1;
    int         time = 0,
                lastTime = 0,
                volume = 100,
                index = 0,
                vid,
                aid,
                sid;
    bool        init = false,
                playlistVisible = false,
                subtitleVisibility = true,
                mute = false;
    int         osdWidth,
                osdHeight;
    unsigned long long                                                reply_userdata = 0;
    QHash<unsigned long long, std::function<void (mpv_event *event)>> reply_callbacks;
};

#endif // MPVHANDLER_H
