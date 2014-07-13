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
    void PlayPrevious();

    void SelectFile(QString url);
    void ToggleVisibility();
    void AddUrl(QString url);
    void AddFile(QString path);

signals:
    void Play(QString url);

private:
    QListWidget *playlist;
    QString path;
};

#endif // PLAYLISTMANAGER_H
