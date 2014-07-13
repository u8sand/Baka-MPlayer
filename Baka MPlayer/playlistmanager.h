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
    void PlayItem(QListWidgetItem *item);
    void PlayNext();
    void PlayPrevious();

    void SelectFile(QString path);
    void Select(QString url);

    void ToggleVisibility();

private slots:
    void Play(QString url);

signals:
    void PlayFile(QString url);
    void PlayUrl(QString url);

private:
    QListWidget *playlist;
    QString path;
    bool file;
};

#endif // PLAYLISTMANAGER_H
