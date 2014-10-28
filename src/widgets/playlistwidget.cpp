#include "playlistwidget.h"

#include <QListWidgetItem>
#include <QMenu>

#include <algorithm> // for std::random_shuffle and std::sort

PlaylistWidget::PlaylistWidget(QWidget *parent) :
    QListWidget(parent),
    showAll(false)
{
}

QAction *PlaylistWidget::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    QListWidget::addAction(action);
    return action;
}

void PlaylistWidget::SelectItem(const QString &item)
{
    setCurrentItem(*findItems(item, Qt::MatchExactly).begin());
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
    // todo: narrow list based on search
}

void PlaylistWidget::ShowAll(bool b)
{
    // todo: narrow list based on suffix
}

void PlaylistWidget::Shuffle(bool b)
{
    if(b)
    {
        QList<QListWidgetItem*> L = items(nullptr);
        std::random_shuffle(L.begin(), L.end());
    }
    else
        sortItems(Qt::AscendingOrder);
}

bool PlaylistWidget::isShowAll()
{
    return showAll;
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
