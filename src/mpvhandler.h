#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include <mpv/client.h>

#include "mpvtypes.h"

class MpvHandler : public QObject
{
    Q_OBJECT
public:
    explicit MpvHandler(int64_t wid, QObject *parent = 0);
    ~MpvHandler();

    const Mpv::FileInfo &getFileInfo()      { return fileInfo; }
    const QStringList &getPlaylist()        { return playlist; }
    Mpv::PlayState getPlayState()           { return playState; }
    QString getFile()                       { return file; }
    QString getPath()                       { return path; }
    QString getLastFile()                   { return lastFile; }
    QString getScreenshotFormat()           { return screenshotFormat; }
    QString getScreenshotTemplate()         { return screenshotTemplate; }
    QString getScreenshotDir()              { return screenshotDir; }
    QString getSearch()                     { return search; }
    double getSpeed()                       { return speed; }
    int getTime()                           { return time; }
    int getVolume()                         { return volume; }
    int getIndex()                          { return index; }
    int getMaxIndex()                       { return playlist.size()-1; }
    bool getDebug()                         { return debug; }
    bool getShowAll()                       { return showAll; }
    bool getShuffle()                       { return shuffle; }
    bool getPlaylistVisible()               { return playlistVisible; }
    bool getSubtitleVisibility()            { return subtitleVisibility; }

protected:
    virtual bool event(QEvent *event);

public slots:
    void LoadSettings(QSettings*);
    void SaveSettings(QSettings*);

    void LoadFile(QString);

    void PlayIndex(int);
    void NextFile();
    void PreviousFile();

    void RefreshPlaylist();
    void SearchPlaylist(QString);
    void ShufflePlaylist(bool);
    void ShowAllPlaylist(bool);

    void Play();
    void Pause();
    void Stop();
    void PlayPause(int indexIfStopped);
    void Restart();
    void Rewind();

    void Seek(int pos, bool relative = false);
    void FrameStep();
    void FrameBackStep();

    void Chapter(int chapter);
    void NextChapter();
    void PreviousChapter();

    void Volume(int level);
    void Speed(double);
    void Aspect(QString aspect);
    void Vid(int vid);
    void Aid(int aid);
    void Sid(int sid);

    void Screenshot(bool withSubs = false);

    void ScreenshotFormat(QString);
    void ScreenshotTemplate(QString);
    void ScreenshotDirectory(QString);

    void AddSubtitleTrack(QString file);
    void ShowSubtitles(bool);
    void SubtitleScale(double scale, bool relative = false);

    void Debug(bool b);

    void LoadTracks();
    void LoadChapters();
    void LoadVideoParams();
protected slots:
    void OpenFile(QString f);
    void PopulatePlaylist();
    void SortPlaylist();
    void LoadFileInfo();
    void SetProperties();

    void AsyncCommand(const char *args[]);

private slots:
    void setPlaylist()                      { emit playlistChanged(playlist); }
    void setFileInfo()                      { emit fileInfoChanged(fileInfo); }
    void setPlayState(Mpv::PlayState s)     { emit playStateChanged(playState = s); }
    void setFile(QString s)                 { emit fileChanged(file = s); }
    void setPath(QString s)                 { emit pathChanged(path = s); }
    void setLastFile(QString s)             { emit lastFileChanged(lastFile = s); }
    void setScreenshotFormat(QString s)     { emit screenshotFormatChanged(screenshotFormat = s); }
    void setScreenshotTemplate(QString s)   { emit screenshotTemplateChanged(screenshotTemplate = s); }
    void setScreenshotDir(QString s)        { emit screenshotDirChanged(screenshotDir = s); }
    void setSearch(QString s)               { emit searchChanged(search = s); }
    void setSpeed(double d)                 { emit speedChanged(speed = d); }
    void setTime(int i)                     { emit timeChanged(time = i); }
    void setVolume(int i)                   { emit volumeChanged(volume = i); }
    void setIndex(int i)                    { emit indexChanged(index = i); }
    void setDebug(bool b)                   { emit debugChanged(debug = b); }
    void setShowAll(bool b)                 { emit showAllChanged(showAll = b); }
    void setShuffle(bool b)                 { emit shuffleChanged(shuffle = b); }
    void setPlaylistVisible(bool b)         { emit playlistVisibleChanged(playlistVisible = b); }
    void setSubtitleVisibility(bool b)      { emit subtitleVisibilityChanged(subtitleVisibility = b); }

signals:
    void playlistChanged(const QStringList&);
    void fileInfoChanged(const Mpv::FileInfo&);
    void playStateChanged(Mpv::PlayState);
    void fileChanged(QString);
    void pathChanged(QString);
    void lastFileChanged(QString);
    void screenshotFormatChanged(QString);
    void screenshotTemplateChanged(QString);
    void screenshotDirChanged(QString);
    void searchChanged(QString);
    void speedChanged(double);
    void timeChanged(int);
    void volumeChanged(int);
    void indexChanged(int);
    void debugChanged(bool);
    void showAllChanged(bool);
    void shuffleChanged(bool);
    void playlistVisibleChanged(bool);
    void subtitleVisibilityChanged(bool);

    void errorSignal(QString e);
    void debugSignal(QString d);

private:
    mpv_handle *mpv;

    // variables
    Mpv::PlayState playState = Mpv::Idle;
    QStringList playlist;
    Mpv::FileInfo fileInfo = {QString(), 0, {0, 0, 0, 0, 0}, QList<Mpv::Track>(), QList<Mpv::Chapter>()};
    QString     file,
                path,
                lastFile,
                screenshotFormat,
                screenshotTemplate,
                screenshotDir,
                search,
                suffix;
    double      speed = 1;
    int         time = 0,
                volume = 100,
                index = 0;
    bool        debug = false,
                showAll = false,
                shuffle = false,
                playlistVisible = false,
                subtitleVisibility = true;
};

#endif // MPVHANDLER_H
