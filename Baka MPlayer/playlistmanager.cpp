#include "playlistmanager.h"

#include <QDir>

#include <algorithm>

PlaylistManager::PlaylistManager(QListWidget *_playlist, QObject *parent) :
    QObject(parent),
    playlist(_playlist),
    index(0)
{
}

void PlaylistManager::PlayIndex(int i)
{
    QString file = path+list[(index = i)];
    QFile f(file);
    if(f.exists())
        emit PlayFile(file);
}

void PlaylistManager::PlayNext()
{
    if(list.size() > 0 && index < list.size()-1)
        PlayIndex(++index);
}

void PlaylistManager::PlayPrevious()
{
    if(list.size() > 0 && index > 1)
        PlayIndex(++index);
}

void PlaylistManager::LoadFile(QString f, bool showAll)
{
    QRegExp rx("^(http://.+\\.[a-z]+)", Qt::CaseInsensitive);

    if(rx.indexIn(f) != -1) // web url
    {
        path = "";
        index = 0;
        list.clear();
        list.push_back(f);
        Refresh();
    }
    else // local file
    {
        QFileInfo fi(f);
        if(path == fi.absolutePath() && // path is the same
           (index = list.indexOf(fi.fileName())) != -1) // file exists in the list
            PlayIndex(index);                           // play it
        else // file doesn't exist in list
        {
            path = fi.absolutePath()+"/"; // get path
            // populate list
            if(showAll)
                Refresh();
            else
                Refresh(fi.suffix());
            index = list.indexOf(fi.fileName()); // get index
        }
    }
    SelectCurrent();
    // open up the playlist only if there is more than one item
    if(list.size() > 1)
        playlist->setVisible(true);
    // play file
    emit PlayFile(f);
}

void PlaylistManager::Shuffle()
{
    playlist->clear();
    std::random_shuffle(list.begin(), list.end());
    playlist->addItems(list);
}

void PlaylistManager::SelectCurrent()
{
    playlist->setCurrentRow(index);
}

void PlaylistManager::Refresh(QString suffix)
{
    if(path != "")
    {
        list.clear(); // clear existing list
        QDir root(path);
        QFileInfoList flist;
        if(suffix == "")
            flist = root.entryInfoList({"*.mkv", "*.mp4", "*.avi", "*.mp3", "*.ogm"}, QDir::Files); // todo: pass more file-types (get from settings)
        else
            flist = root.entryInfoList({"*."+suffix}, QDir::Files);
        for(auto &i : flist)
            list.push_back(i.fileName()); // add files to the list
    }
    playlist->clear();
    playlist->addItems(list);
}

void PlaylistManager::ShowAll(bool showAll)
{
    QString file = list[index];
    if(showAll)
        Refresh();
    else
    {
        QFileInfo fi(file);
        Refresh(fi.suffix());
    }
    if((index = list.indexOf(file)) == -1) // find new index
    {
        // not found: set to top of list
        index = 0;
    }
}
