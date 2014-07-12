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
    void PlayIndex(QModelIndex index);
    void PlayNext();

    void SelectFile(QString url);
    void ToggleVisibility();
    void AddFile(QString url);
    void AddDirectory(QString path);

signals:
    void Play(QString file);

private:
    QListWidget *playlist;
};

#endif // PLAYLISTMANAGER_H
