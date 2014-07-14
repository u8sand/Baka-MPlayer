#include "playlistmanager.h"

#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist),
    index(0),
    path("")
{
    connect(playlist, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(PlayIndex(QModelIndex)));
}

// TODO: Check to make sure files exist and such

void PlaylistManager::PlayNext()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() < playlist->count())
    {
        playlist->setCurrentRow(index++);
        PlayFile(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayPrevious()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() > 1)
    {
        playlist->setCurrentRow(--index);
        PlayFile(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayIndex(QModelIndex i)
{
    index = i.row();
    PlayFile(path + "/" + playlist->item(index)->text());
}

void PlaylistManager::PlayItem(QListWidgetItem *i)
{
    playlist->setCurrentItem(i);
    index = playlist->row(i);
    PlayFile(path + "/" + i->text());
}

void PlaylistManager::ToggleVisibility()
{
    playlist->setVisible(!playlist->isVisible());
}

void PlaylistManager::LoadFile(QString f)
{
    playlist->clear();
    // todo: check for web urls--we won't use QDir on those

    QFileInfo fi(f);
    path = fi.absolutePath() + "/";
    QDir root(path);

    QFileInfoList flist = root.entryInfoList({ "*.mkv", "*.mp4", "*.avi", "*.mp3" }, QDir::Files); // todo: pass more file-types (get from settings)
    // todo: sort?
    for(auto &i : flist)
        playlist->addItem(i.fileName());

    if(playlist->count() > 1)
        playlist->setVisible(true);

    PlayItem((*playlist->findItems(fi.fileName(), Qt::MatchExactly).begin()));
}
