#include "playlistmanager.h"

#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist),
    path("")
{
    connect(playlist, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(PlayIndex(QModelIndex)));
}

void PlaylistManager::PlayNext()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() < playlist->count())
        emit Play(path+playlist->item(playlist->currentRow()+1)->text());
}

void PlaylistManager::PlayPrevious()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() > 1)
        emit Play(path+playlist->item(playlist->currentRow()-1)->text());
}

void PlaylistManager::PlayIndex(QModelIndex index)
{
    emit Play(path+playlist->item(index.row())->text());
}

void PlaylistManager::SelectFile(QString url)
{
    // todo: select file
}

void PlaylistManager::ToggleVisibility()
{
    playlist->setVisible(!playlist->isVisible());
}

void PlaylistManager::AddUrl(QString url)
{
    playlist->clear();

    path = ""; // todo: get path from url
    playlist->addItem(url); // todo: get file name

    playlist->setCurrentRow(1);
    playlist->setVisible(false);

    emit Play(url);
}

void PlaylistManager::AddFile(QString file)
{
    playlist->clear();

    path = QFileInfo(file).absolutePath();
    QDir root(path);

    QFileInfoList flist = root.entryInfoList({ "*.mkv", "*.mp4", "*.avi", "*.mp3" }, QDir::Files); // todo: pass more file-types (get from settings)
    // todo: sort?
    for(auto &i : flist)
        playlist->addItem(i.fileName());

    playlist->setCurrentRow(1);
    if(playlist->count() > 1)
        playlist->setVisible(true);
    playlist->setVisible(false);
    emit Play(file);
}
