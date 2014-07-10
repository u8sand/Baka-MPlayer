#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QListWidget>

class PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaylistManager(QListWidget *playlist, QObject *parent = 0);

    void PlayNext();
signals:


public slots:
    void SelectFile(QString url);
    void ToggleVisibility();

private:
    QListWidget *playlist;
};

#endif // PLAYLISTMANAGER_H
