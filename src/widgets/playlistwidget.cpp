#include "playlistwidget.h"

#include <QListWidgetItem>
#include <QMenu>

#include <algorithm> // for std::random_shuffle and std::sort

PlaylistWidget::PlaylistWidget(QWidget *parent) :
    QListWidget(parent)
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
    playlist.removeOne(item->text());
    return item->text();
}

void PlaylistWidget::SelectItem(const QString &item)
{
    if(item != "")
        setCurrentItem(findItems(item, Qt::MatchExactly).first());
}

void PlaylistWidget::Populate(QStringList list)
{
    playlist = list;
    playlist.sort();
    clear();
    addItems(playlist);
}

QString PlaylistWidget::PreviousItem()
{
    return item(currentRow()-1)->text();
}

QString PlaylistWidget::NextItem()
{
    return item(currentRow()+1)->text();
}

void PlaylistWidget::Search(QString s)
{
    QString item = currentItem()->text();
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
    if(count() == 0) return;
    QListWidgetItem *item = currentItem();
    if(b)
    {
        clear();
        addItems(playlist);
    }
    else
    {
        // todo: this is gross; make it more efficient
        if(item)
        {
            QString suffix = item->text().split('.').last();
            QStringList newPlaylist;
            for(QStringList::iterator i = playlist.begin(); i != playlist.end(); i++)
                if(i->endsWith(suffix))
                    newPlaylist.append(*i);
            clear();
            addItems(newPlaylist);
        }
    }
    SelectItem(item->text());
}

void PlaylistWidget::Shuffle(bool b)
{
    QString item = currentItem()->text();
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
