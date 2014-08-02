#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QListWidget>
#include <QString>

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QListWidget *playlist, QObject *parent = 0);

public slots:
    void PlayIndex(int i);          // plays the file at index i
    void PlayNext();                // plays the next file (++index)
    void PlayPrevious();            // plays the previous file (--index)
    void LoadFile(QString file);    // loads the playlist given a file
    void StopAfterCurrent();        // stop after the current file ends
    void ToggleShuffle();           // shuffling/not (will shuffle/unshuffle playlist)
    void SelectCurrent();           // sets the playlist selection to the current playing file
    void ShowAll();                 // show all file types in folder (rather than just those that are the same as the video)
    void Refresh();                 // refresh the playlist (look at the directory again and repopulate the playlist)

signals:
    void PlayFile(QString file);    // playfile signal, gets forwarded to mpv->OpenFile

private:
    QListWidget *playlist;
    QString path;
    int index;
};

#endif // PLAYLISTMANAGER_H
