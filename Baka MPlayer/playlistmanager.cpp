#include "playlistmanager.h"

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist)
{
}

void PlaylistManager::PlayNext()
{
}

void PlaylistManager::SelectFile(QString url)
{
    // select the file on our list
}

void PlaylistManager::ToggleVisibility()
{
    playlist->setVisible(!playlist->isVisible());
}
