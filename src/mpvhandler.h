#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <mpv/client.h>

#include "mpvtypes.h"

class MpvHandler : public QObject
{
friend class BakaEngine;
    Q_OBJECT
public:
    explicit MpvHandler(int64_t wid, QObject *parent = 0);
    ~MpvHandler();

    const Mpv::FileInfo &getFileInfo()      { return fileInfo; }
    Mpv::PlayState getPlayState()           { return playState; }
    QString getFile()                       { return file; }
    QString getPath()                       { return path; }
    QString getScreenshotFormat()           { return screenshotFormat; }
    QString getScreenshotTemplate()
    {
        if(screenshotTemplate==QString())
            return (screenshotTemplate = "screenshot%#04");
        else
            return screenshotTemplate;
    }
    QString getScreenshotDir()
    {
        if(screenshotTemplate == QString())
            return (screenshotDir = ".");
        else
            return screenshotDir;
    }
    double getSpeed()                       { return speed; }
    int getTime()                           { return time; }
    int getVolume()                         { return volume; }
    int getVid()                            { return vid; }
    int getAid()                            { return aid; }
    int getSid()                            { return sid; }
    bool getSubtitleVisibility()            { return subtitleVisibility; }

    int getOsdWidth()                       { return osdWidth; }
    int getOsdHeight()                      { return osdHeight; }

    QString getMediaInfo();

protected:
    virtual bool event(QEvent*);

    bool FileExists(QString);

public slots:
    void LoadFile(QString);
    QString LoadPlaylist(QString);
    void PlayFile(QString);

    void AddOverlay(int id, int x, int y, QString file, int offset, int w, int h);
    void RemoveOverlay(int id);

    void Play();
    void Pause();
    void Stop();
    void PlayPause(QString fileIfStopped);
    void Restart();
    void Rewind();

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
    void ShowSubtitles(bool);
    void SubtitleScale(double scale, bool relative = false);

    void Debug(QString level);

    void ShowText(QString text, int duration = 4000, int level = 1);

    void LoadTracks();
    void LoadChapters();
    void LoadVideoParams();
    void LoadMetadata();
    void LoadOsdSize();

    void Command(const QStringList &strlist);
    void SetOption(QString key, QString val);

protected slots:
    void OpenFile(QString);
    QString PopulatePlaylist();
    void LoadFileInfo();
    void SetProperties();

    void AsyncCommand(const char *args[]);
    void Command(const char *args[]);
    void NotInitialized();

private slots:
    void setPlaylist(const QStringList& l)  { emit playlistChanged(l); }
    void setFileInfo()                      { emit fileInfoChanged(fileInfo); }
    void setPlayState(Mpv::PlayState s)     { emit playStateChanged(playState = s); }
    void setFile(QString s)                 { emit fileChanged(file = s); }
    void setPath(QString s)                 { emit pathChanged(path = s); }
    void setScreenshotFormat(QString s)     { emit screenshotFormatChanged(screenshotFormat = s); }
    void setScreenshotTemplate(QString s)   { emit screenshotTemplateChanged(screenshotTemplate = s); }
    void setScreenshotDir(QString s)        { emit screenshotDirChanged(screenshotDir = s); }
    void setSpeed(double d)                 { emit speedChanged(speed = d); }
    void setTime(int i)                     { emit timeChanged(time = i); }
    void setVolume(int i)                   { emit volumeChanged(volume = i); }
    void setIndex(int i)                    { emit indexChanged(index = i); }
    void setVid(int i)                      { emit vidChanged(vid = i); }
    void setAid(int i)                      { emit aidChanged(aid = i); }
    void setSid(int i)                      { emit sidChanged(sid = i); }
    void setSubtitleVisibility(bool b)      { emit subtitleVisibilityChanged(subtitleVisibility = b); }

signals:
    void playlistChanged(const QStringList&);
    void fileInfoChanged(const Mpv::FileInfo&);
    void trackListChanged(const QList<Mpv::Track>&);
    void chaptersChanged(const QList<Mpv::Chapter>&);
    void videoParamsChanged(const Mpv::VideoParams&);
    void playStateChanged(Mpv::PlayState);
    void fileChanged(QString);
    void pathChanged(QString);
    void screenshotFormatChanged(QString);
    void screenshotTemplateChanged(QString);
    void screenshotDirChanged(QString);
    void speedChanged(double);
    void timeChanged(int);
    void volumeChanged(int);
    void indexChanged(int);
    void vidChanged(int);
    void aidChanged(int);
    void sidChanged(int);
    void debugChanged(bool);
    void subtitleVisibilityChanged(bool);

    void messageSignal(QString m);

private:
    mpv_handle *mpv = nullptr;

    // variables
    Mpv::PlayState playState = Mpv::Idle;
    Mpv::FileInfo fileInfo;
    QString     file,
                path,
                screenshotFormat,
                screenshotTemplate,
                screenshotDir,
                suffix;
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
                subtitleVisibility = true;

    int         osdWidth,
                osdHeight;
};

#endif // MPVHANDLER_H
