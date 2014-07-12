#include "playlistmanager.h"

#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist)
{
    connect(playlist, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(PlayIndex(QModelIndex)));
}

void PlaylistManager::PlayNext()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() < playlist->count())
        emit Play(playlist->item(playlist->currentRow()+1)->text());
}

void PlaylistManager::PlayIndex(QModelIndex index)
{
    emit Play(playlist->item(index.row())->text());
}

void PlaylistManager::SelectFile(QString url)
{
//    playlist->selectionModel()->
    // if on playlist:
    // else AddDirectory(url);
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
    QDir root(QFileInfo(path).dir());
    QFileInfoList flist = root.entryInfoList({ "*.mkv", "*.mp4", "*.avi" }, QDir::Files); // todo: pass file-types (mkv avi mp4 etc.. get from settings)

    for(auto &i : flist)
        AddFile(i.fileName());
}
