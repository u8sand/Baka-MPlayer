#include "playlistmanager.h"

#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, MpvHandler *_mpv, QObject *parent) :
    QObject(parent),
    playlist(_playlist),
    mpv(_mpv),
    index(0),
    path("")
{
}

// TODO: Check to make sure files exist and such

void PlaylistManager::PlayNext()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() < playlist->count()-1)
    {
        playlist->setCurrentRow(++index);
        mpv->OpenFile(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayPrevious()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() > 1)
    {
        playlist->setCurrentRow(--index);
        mpv->OpenFile(path + playlist->currentItem()->text());
    }
}

void PlaylistManager::PlayIndex(QModelIndex i)
{
    mpv->OpenFile(path + playlist->item((index = i.row()))->text());
}

void PlaylistManager::LoadFile(QString f)
{
    playlist->clear();
    // todo: check for web urls--we won't use QDir on those

    QFileInfo fi(f);
    path = fi.absolutePath() + "/";
    QDir root(path);

    QFileInfoList flist = root.entryInfoList({ "*.mkv", "*.mp4", "*.avi", "*.mp3", "*.ogm" }, QDir::Files); // todo: pass more file-types (get from settings)
    // todo: sort?
    int n = 0;
    for(auto &i : flist)
    {
        if(i == fi)
            index = n;
        else
            n++;

        playlist->addItem(i.fileName());
    }
    playlist->setCurrentRow(index);

    if(playlist->count() > 1)
        playlist->setVisible(true);

    mpv->OpenFile(f);
}
