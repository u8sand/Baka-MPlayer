#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QListWidget>
#include <QString>

class PlaylistManager : public QObject // todo: re-think this entirely
{
    Q_OBJECT
public:
    explicit PlaylistManager(QListWidget *playlist, QObject *parent = 0);

public slots:
    void PlayIndex(int i);                              // plays the file at index i
    void PlayNext();                                    // plays the next file (++index)
    void PlayPrevious();                                // plays the previous file (--index)
    void LoadFile(QString file, bool showAll = false);  // loads the playlist given a file
    void Shuffle(bool shuffle);                         // shuffle the playlist entries
    void SelectCurrent();                               // sets the playlist selection to the current playing file
    void Refresh(QString suffix = "");                  // refresh the playlist (look at the directory again and repopulate the playlist)
    void ShowAll(bool showAll);                         // change ShowAll state

signals:
    void PlayFile(QString file);                        // playfile signal, gets forwarded to mpv->OpenFile
    void SetVisible(bool visibile);                     // visibility signal, gets forwarded to mainwindow::SetPlaylist(bool)

private:
    QStringList list;
    QListWidget *playlist;
    QString path;
    int index;
};

#endif // PLAYLISTMANAGER_H
