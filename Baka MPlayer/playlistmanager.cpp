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
    {
        playlist->setCurrentRow(playlist->currentRow()+1);
        Play(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayPrevious()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() > 1)
    {
        playlist->setCurrentRow(playlist->currentRow()-1);
        Play(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayIndex(QModelIndex index)
{
    Play(path + "/" + playlist->item(index.row())->text());
}

void PlaylistManager::PlayItem(QListWidgetItem *item)
{
    playlist->setCurrentItem(item);
    Play(path + "/" + item->text());
}

void PlaylistManager::Play(QString url)
{
    if(file)
        emit PlayFile(url);
    else
        emit PlayUrl(url);
}

void PlaylistManager::ToggleVisibility()
{
    playlist->setVisible(!playlist->isVisible());
}

void PlaylistManager::SelectFile(QString url)
{
    playlist->clear();

    file = true;
    QFileInfo fi(url);
    path = fi.absolutePath() + "/";
    QDir root(path);

    QFileInfoList flist = root.entryInfoList({ "*.mkv", "*.mp4", "*.avi", "*.mp3" }, QDir::Files); // todo: pass more file-types (get from settings)
    // todo: sort?
    for(auto &i : flist)
        playlist->addItem(i.fileName());

    if(playlist->count() > 1)
        playlist->setVisible(true);

    PlayItem((QListWidgetItem*)*playlist->findItems(fi.fileName(), Qt::MatchExactly).begin());
}

void PlaylistManager::Select(QString url)
{
    // if file: SelectFile(url);
    // else if url
    playlist->clear();

    file = false;
    path = ""; // todo: get path from url
    playlist->addItem(url); // todo: get basename

    PlayItem(playlist->item(0));
}
