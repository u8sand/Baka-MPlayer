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
    void PlayNext();
    void PlayPrevious();
    void PlayIndex(QModelIndex i);
    void LoadFile(QString file);

signals:
    void PlayFile(QString file);

private:
    QListWidget *playlist;
    int index;
    QString path;
};

#endif // PLAYLISTMANAGER_H
