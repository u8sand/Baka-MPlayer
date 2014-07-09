#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QListWidget>

class PlaylistManager : public QListWidget
{
    Q_OBJECT
public:
    explicit PlaylistManager(QWidget *parent = 0);

    void PlayNext();

signals:


public slots:
    void SelectFile(QString url);
    void ToggleVisibility();
private:

};

#endif // PLAYLISTMANAGER_H
