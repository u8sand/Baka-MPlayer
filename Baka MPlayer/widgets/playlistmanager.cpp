#include "playlistmanager.h"

PlaylistManager::PlaylistManager(QWidget *parent) :
    QListWidget(parent)
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
    this->setVisible(!this->isVisible());
}
