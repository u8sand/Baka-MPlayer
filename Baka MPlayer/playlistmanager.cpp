#include "playlistmanager.h"

#include <QDir>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist),
    index(0)
{
}

void PlaylistManager::PlayIndex(int i)
{
    if(index != i)
    {
        QString file = path+playlist->item((index = i))->text();
        QFile f(file);
        if(f.exists())
            emit PlayFile(file);
    }
}

void PlaylistManager::PlayNext()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() < playlist->count()-1)
        PlayIndex(++index);
}

void PlaylistManager::PlayPrevious()
{
    if(playlist->count() > 0 &&
            playlist->currentRow() > 1)
        PlayIndex(--index);
}

void PlaylistManager::LoadFile(QString f)
{
    playlist->clear();
    // todo: check for web urls--we won't use QDir on those

    QFileInfo fi(f);
    path = fi.absolutePath()+"/";
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

    emit PlayFile(f);
}

void PlaylistManager::StopAfterCurrent() // todo
{

}

void PlaylistManager::ToggleShuffle() // todo
{

}

void PlaylistManager::SelectCurrent()
{
    playlist->setCurrentRow(index);
}

void PlaylistManager::ShowAll() // todo
{

}

void PlaylistManager::Refresh() // todo
{

}
