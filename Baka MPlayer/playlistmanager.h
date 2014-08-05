#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QSettings *settings, QObject *parent = 0);

//    QStringList GetList() const;      // return playlist
    int GetIndex() const;               // return index
//    bool GetShuffle() const;          // return shuffle

public slots:
    void LoadFile(QString file);        // frontend for opening a file, populating the playlist etc.
    void PlayIndex(int i);              // play a specific index (eg. double click index on playlist widget)
    void Next();                        // play the next file in the playlist
    void Previous();                    // play the previous file in the playlist

    void Refresh();                     // refresh the playlist (look again at the directory)
    void Shuffle(bool s);               // shuffle the playlist (resort)
    void ShowAll(bool s);               // show all--show files based on extension

private slots:
    void Populate();                    // populates the list based on path and showAll
    void Sort();                        // sorts based on (shuffle) either ascending or random

signals:
    void Play(QString f);               // signal to play a file with mpv
    void Stop();                        // signal to stop playback
    void Show(bool v);                  // signal to show/hide playlist widget

    void ListChanged(QStringList l);    // signal to update the playlist
    void IndexChanged(int i);           // signal to change the playlist selection
    void ShuffleChanged(bool s);        // signal to update the shuffle state

private:
    QSettings *settings;
    QStringList list;
    QString path, suffix;
    bool shuffle, showAll;
    int index;
};

#endif // PLAYLISTMANAGER_H
