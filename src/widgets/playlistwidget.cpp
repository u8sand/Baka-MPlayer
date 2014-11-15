#include "playlistwidget.h"

#include <QListWidgetItem>
#include <QMenu>

#include <algorithm> // for std::random_shuffle and std::sort

PlaylistWidget::PlaylistWidget(QWidget *parent) :
    QListWidget(parent),
    cItem()
{
}

QAction *PlaylistWidget::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    QListWidget::addAction(action);
    return action;
}

QString PlaylistWidget::RemoveItem(int index)
{
    QListWidgetItem *item = takeItem(index);
    if(item)
    {
        playlist.removeOne(item->text());
        return item->text();
    }
    return QString();
}

void PlaylistWidget::SelectItem(const QString &item, bool internal)
{
    if(item != "")
    {
        if(!internal)
        {
            QList<QListWidgetItem*> items = findItems(item, Qt::MatchExactly);
            if(items.length() > 0)
            {
                setCurrentItem(items.first());
                cItem = items.first()->text();
            }
        }
        else
            cItem = item;
    }
}

void PlaylistWidget::Populate(QStringList list)
{
    playlist = list;
    playlist.sort();
}

QString PlaylistWidget::FirstItem()
{
    QListWidgetItem *item = QListWidget::item(0);
    if(item)
        return item->text();
    return QString();
}

QString PlaylistWidget::CurrentItem()
{
    QListWidgetItem *item = currentItem();
    if(item)
        return item->text();
    return QString();
}

QString PlaylistWidget::PreviousItem()
{
    return FileAt(currentRow()-1);
}

QString PlaylistWidget::NextItem()
{
    return FileAt(currentRow()+1);
}

QString PlaylistWidget::FileAt(int index)
{
    QListWidgetItem *item = QListWidget::item(index);
    if(item)
        return item->text();
    return QString();
}

void PlaylistWidget::Search(QString s)
{
    QListWidgetItem *_item = currentItem();
    QString item;
    if(_item)
        item = _item->text();
    else
        item = cItem;
    QStringList newPlaylist;
    for(QStringList::iterator item = playlist.begin(); item != playlist.end(); item++)
        if(item->contains(s))
            newPlaylist.append(*item);
    clear();
    addItems(newPlaylist);
    SelectItem(item);
}

void PlaylistWidget::ShowAll(bool b)
{
    if(playlist.length() > 0)
    {
        QListWidgetItem *_item = currentItem();
        QString item;
        if(_item)
            item = _item->text();
        else
            item = cItem;
        if(b)
        {
            clear();
            addItems(playlist);
        }
        else
        {
            if(!_item)
                item = cItem;
            QString suffix = item.split('.').last();
            QStringList newPlaylist;
            for(QStringList::iterator i = playlist.begin(); i != playlist.end(); i++)
                if(i->endsWith(suffix))
                    newPlaylist.append(*i);
            clear();
            addItems(newPlaylist);
        }
        SelectItem(item);
    }
}

void PlaylistWidget::Shuffle(bool b)
{
    if(count() > 0)
    {
        QListWidgetItem *_item = currentItem();
        QString item;
        if(_item)
            item = _item->text();
        else
            item = cItem;
        if(b)
        {
            QStringList newPlaylist = playlist;
            std::random_shuffle(newPlaylist.begin(), newPlaylist.end());
            clear();
            addItems(newPlaylist);
        }
        else
        {
            clear();
            addItems(playlist);
        }
        SelectItem(item);
    }
}

void PlaylistWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QListWidgetItem *item = itemAt(event->pos());
    if(item)
    {
        QMenu *menu = new QMenu();
        menu->addActions(actions());
        menu->exec(viewport()->mapToGlobal(event->pos()));
        delete menu;
    }
}
