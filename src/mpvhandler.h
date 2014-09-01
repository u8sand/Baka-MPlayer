#ifndef MPVHANDLER_H
#define MPVHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <mpv/client.h>

#include "mpvtypes.h"

#define VARIABLE(t, n) \
    public: t get

class MpvHandler : public QObject
{
    Q_OBJECT
public:
    explicit MpvHandler(int64_t wid, QObject *parent = 0);
    ~MpvHandler();

protected:
    virtual bool event(QEvent *event);          // QObject event function

public slots:
    void LoadFile(QString f);
    void PlayIndex(int i);
    void NextFile();
    void PreviousFile();
    void Populate();
    void Refresh();
    void Sort();

    void OpenFile(QString f);                   // open the file for mpv playing
    void Play();
    void Pause();
    void PlayPause();                           // toggle pause/unpause state
    void Seek(int pos, bool relative = false);  // seek to specific location
    void Restart();                             // seek to the beginning
    void Stop();                                // stop playback (and go to beginning)
    void SetVid(int vid);                       // set the video track
    void SetAid(int aid);                       // set the audio track
    void SetSid(int sid);                       // set the subtitle track
    void SetAspect(QString aspect);             // set the aspect ratio
    void AddSub(QString f);                     // add an external subtitle track
    void SetChapter(int chapter);               // seek to the specified chapter
    void NextChapter();                         // seek to next chapter
    void PreviousChapter();                     // seek to previous chapter
    void FrameStep();                           // frame step
    void FrameBackStep();                       // frame back step
    void AddVolume(int level);
    void AdjustVolume(int level);               // adjust the media volume
    void Screenshot(bool withSubs = false);     // take a screenshot
    void ToggleFullscreen();                    // toggle fullscreen
    void SetSubs(bool b);                       // set subtitle visibility
    void AddSubScale(double scale);
    void SetSubScale(double scale);

    void Debug(bool b);
    void CursorAutoHide(bool b);

    void LoadFileInfo();                        // load all the required file information
    void LoadTracks();
    void LoadChapters();
    void LoadVideoParams();

private slots:
    void AsyncCommand(const char *args[]);      // execute async mpv command

signals:
    void errorSignal(QString e);                // triggered when an error occurs
    void debugSignal(QString d);                // triggered when an mpv log message is send

private:
    mpv_handle *mpv;                            // mpv client handle

    // variables
    Mpv::PlayState playState = Mpv::Idle;
    QStringList playlist;
    Mpv::FileInfo fileInfo = {QString(), 0, {0, 0, 0, 0, 0}, QList<Mpv::Track>(), QList<Mpv::Chapter>()};
    QString     lastFile,
                screenshotFormat,
                screenshotTemplate,
                search,
                path,
                suffix;
    double      speed = 1;
    int         time = 0,
                volume = 100,
                index = 0;
    bool        debug = false,
                showAll = false,
                shuffle = false,
                playlistVisible = false;

public:
    const Mpv::FileInfo &getFileInfo()      { return fileInfo; }
    const QStringList &getPlaylist()        { return playlist; }
    Mpv::PlayState getPlayState()           { return playState; }
    QString getFile()                       { return path+playlist[index]; }
    QString getLastFile()                   { return lastFile; }
    QString getScreenshotFormat()           { return screenshotFormat; }
    QString getScreenshotTemplate()         { return screenshotTemplate; }
    QString getSearch()                     { return search; }
    QString getPath()                       { return path; }
    double getSpeed()                       { return speed; }
    int getTime()                           { return time; }
    int getVolume()                         { return volume; }
    int getIndex()                          { return index; }
    int getMaxIndex()                       { return playlist.size()-1; }
    bool getDebug()                         { return debug; }
    bool getShowAll()                       { return showAll; }
    bool getShuffle()                       { return shuffle; }
    bool getPlaylistVisible()               { return playlistVisible; }

public slots:
    void setPlayState(Mpv::PlayState s)     { emit playStateChanged(playState = s); }
    void setLastFile(QString s)             { emit lastFileChanged(lastFile = s); }
    void setScreenshotFormat(QString s)     { emit screenshotFormatChanged(screenshotFormat = s); }
    void setScreenshotTemplate(QString s)   { emit screenshotTemplateChanged(screenshotTemplate = s); }
    void setSearch(QString s)               { emit searchChanged(search = s); }
    void setSpeed(double d)                 { emit speedChanged(speed = d); }
    void setTime(int i)                     { emit timeChanged(time = i); }
    void setVolume(int i)                   { emit volumeChanged(volume = i); }
    void setIndex(int i)                    { emit indexChanged(index = i); }
    void setDebug(bool b)                   { emit debugChanged(debug = b); }
    void setShowAll(bool b)                 { emit showAllChanged(showAll = b); }
    void setShuffle(bool b)                 { emit shuffleChanged(shuffle = b); }
    void setPlaylistVisible(bool b)         { emit playlistVisibleChanged(playlistVisible = b); }

signals:
    void playlistChanged(const QStringList&);
    void fileInfoChanged(const Mpv::FileInfo&);
    void playStateChanged(Mpv::PlayState);
    void fileChanged(QString);
    void lastFileChanged(QString);
    void screenshotFormatChanged(QString);
    void screenshotTemplateChanged(QString);
    void searchChanged(QString);
    void speedChanged(double);
    void timeChanged(int);
    void volumeChanged(int);
    void indexChanged(int);
    void debugChanged(bool);
    void showAllChanged(bool);
    void shuffleChanged(bool);
    void playlistVisibleChanged(bool);
};

#endif // MPVHANDLER_H
