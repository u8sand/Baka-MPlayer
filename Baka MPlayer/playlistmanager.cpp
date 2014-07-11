#include "playlistmanager.h"

//#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist)
{
}

void PlaylistManager::PlayNext()
{
//    Play(playlist->selectedItems()->begin());
}

void PlaylistManager::SelectFile(QString url)
{
//    playlist->selectionModel()->select(playlist->findItems(url));
}

void PlaylistManager::ToggleVisibility()
{
    playlist->setVisible(!playlist->isVisible());
}

void PlaylistManager::AddFile(QString url)
{
    playlist->addItem(url);
}

void PlaylistManager::AddDirectory(QString path)
{
//    // loop through directorie and add files
//    QDir root(path);
//    QFileInfoList flist = root.entryInfoList({ ".mkv", ".avi", ".mp4" });

//    for(auto i : flist)
//        AddFile(i->fileName());
}
