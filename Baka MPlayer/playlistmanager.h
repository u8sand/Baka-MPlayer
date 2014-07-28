#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QListWidget>
#include <QString>
#include "mpvhandler.h"

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QListWidget *playlist, MpvHandler *mpv, QObject *parent = 0);

public slots:
    void PlayNext();
    void PlayPrevious();
    void PlayIndex(QModelIndex i);
    void LoadFile(QString file);

signals:
    void PlayFile(QString file);

private:
    QListWidget *playlist;
    MpvHandler *mpv;
    int index;
    QString path;
};

#endif // PLAYLISTMANAGER_H
