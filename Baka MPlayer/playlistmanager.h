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

    QStringList GetList() const;        // return playlist
    QString GetPath() const;            // return path
    QString GetSuffix() const;          // return suffix
    QString GetCurrentFile() const;     // returns the current file full path
    bool GetShuffle() const;            // return shuffle
    bool GetShowAll() const;            // return showAll
    int GetIndex() const;               // return index
    int GetMax() const;                 // return maxium index

public slots:
    void LoadFile(QString file);        // frontend for opening a file, populating the playlist etc.
    void PlayIndex(int i);              // play a specific index (eg. double click index on playlist widget)
    bool Next();                        // play the next file in the playlist
    void Previous();                    // play the previous file in the playlist
    void SearchPlaylist(QString s);     // search through playlist
    void Refresh();                     // refresh the playlist (look again at the directory)
    void Shuffle(bool s);               // shuffle the playlist (resort)
    void ShowAll(bool s);               // show all--show files based on extension

private slots:
    void Populate();                    // populates the list based on path and showAll
    void Sort();                        // sorts based on (shuffle) either ascending or random

signals:
    void Play(QString f);               // signal to play a file with mpv
    void Stop();                        // signal to stop playback
    void Search(QString s);             // signal to modify search textbox
    void Show(bool v);                  // signal to show/hide playlist widget
    void ListChanged(QStringList l);    // signal to update the playlist
    void IndexChanged(int i);           // signal to change the playlist selection
    void ShuffleChanged(bool s);        // signal to update the shuffle state

private:
    QSettings *settings;                // application-wide settings
    QStringList list;                   // the playlist
    QString path,                       // the absolute file path
            suffix;                     // the file suffix
    bool shuffle,                       // shuffle?
         showAll;                       // showAll?
    int index;                          // the current index
};

#endif // PLAYLISTMANAGER_H
