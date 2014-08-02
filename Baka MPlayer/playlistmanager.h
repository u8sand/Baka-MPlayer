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
    void PlayIndex(int i);
    void PlayNext();
    void PlayPrevious();
    void LoadFile(QString file);
    void StopAfterCurrent();
    void ToggleShuffle();

signals:
    void PlayFile(QString file);

private:
    QListWidget *playlist;
    QString path;
    int index;
};

#endif // PLAYLISTMANAGER_H
