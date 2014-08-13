#include "playlistmanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QRegExp>

#include <algorithm> // for std::random_shuffle and std::sort

PlaylistManager::PlaylistManager(QSettings *_settings, QObject *parent) :
    QObject(parent),
    settings(_settings)
{
    shuffle = settings->value("playlist/shuffle", false).toBool();
    showAll = settings->value("playlist/show-all", false).toBool();
    index = 0;
}

QStringList PlaylistManager::GetList() const
{
    return list;
}

QString PlaylistManager::GetPath() const
{
    return path;
}

QString PlaylistManager::GetSuffix() const
{
    return suffix;
}

QString PlaylistManager::GetCurrentFile() const
{
    return path+list[index];
}

bool PlaylistManager::GetShuffle() const
{
    return shuffle;
}

bool PlaylistManager::GetShowAll() const
{
    return showAll;
}

int PlaylistManager::GetIndex() const
{
    return index;
}

void PlaylistManager::LoadFile(QString f)
{
    if(f == "") return; // ignore empty file

    QRegExp rx("^(https?://.+\\.[a-z]+)", Qt::CaseInsensitive);

    if(rx.indexIn(f) != -1) // web url
    {
        path = "";
        index = 0;
        list.clear();
        list.push_back(f);
        emit ListChanged(list);
    }
    else // local file
    {
     QFileInfo fi(f);
     if(path == fi.absolutePath() && // path is the same
        (index = list.indexOf(fi.fileName())) != -1) // file exists in the list
         PlayIndex(index);                           // play it
     else // file doesn't exist in list
     {
         path = QString(fi.absolutePath()+"/"); // get path
         suffix = fi.suffix();
         Populate();
         Sort();
         emit ListChanged(list);
         index = list.indexOf(fi.fileName()); // get index
     }
    }
    if(list.size() > 1) // open up the playlist only if there is more than one item
        emit Show(true);
    PlayIndex(index);
}

void PlaylistManager::PlayIndex(int i)
{
    if(i >= 0 && i < list.size())
    {
        index = i;
        emit IndexChanged(index);

        if(path == "") // web url
        {
            settings->setValue("last-file", list[index]);
            emit Play(QString(list[index]));
        }
        else
        {
            QFile f(QString(path+list[index]));
            if(f.exists())
            {
                settings->setValue("last-file", path+list[index]);
                emit Play(QString(path+list[index]));
            }
            else
                emit Stop();
        }
    }
    else
        Stop();
}

void PlaylistManager::Next()
{
    PlayIndex(index+1);
}

void PlaylistManager::Previous()
{
    PlayIndex(index-1);
}

void PlaylistManager::SearchPlaylist(QString s)
{
    QStringList tmplist;
    for(QStringList::iterator item = list.begin(); item != list.end(); ++item)
        if(item->contains(s, Qt::CaseInsensitive))
            tmplist.push_back(*item);
    emit ListChanged(tmplist);
}

void PlaylistManager::Refresh()
{
    shuffle = false;
    emit ShuffleChanged(false);

    Populate();
    Sort();
    emit ListChanged(list);
    emit Search("");
}

void PlaylistManager::Shuffle(bool s)
{
    shuffle = s;
    Sort();
    emit ListChanged(list);
}

void PlaylistManager::ShowAll(bool s) // todo: use current selection's suffix
{
    showAll = s;
    Populate();
    Sort();
    emit ListChanged(list);
    emit Search("");
}

void PlaylistManager::Populate()
{
    if(path != "")
    {
        list.clear(); // clear existing list
        QDir root(path);
        QFileInfoList flist;
        if(suffix == "")
            flist = root.entryInfoList({"*.mkv", "*.mp4", "*.avi", "*.mp3", "*.ogm"}, QDir::Files); // todo: pass more file-types (get from settings)
        else
            flist = root.entryInfoList({QString("*.").append(suffix)}, QDir::Files);
        for(auto &i : flist)
            list.push_back(i.fileName()); // add files to the list
    }
}

void PlaylistManager::Sort()
{
    if(shuffle) // shuffle list
        std::random_shuffle(list.begin(), list.end());
    else        // sort list
        std::sort(list.begin(), list.end());
}
