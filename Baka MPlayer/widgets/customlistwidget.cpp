#include "customlistwidget.h"

#include <QListWidgetItem>
#include <QAction>

// todo

CustomListWidget::CustomListWidget(QWidget *parent) :
    QListWidget(parent)
{
    menu = new QMenu(this);

    QAction *rfpa = menu->addAction("R&emove from playlist"),
            *dfda = menu->addAction("&Delete from Disk");
    menu->addSeparator();
    QAction *ra   = menu->addAction("&Refresh");

    connect(rfpa, SIGNAL(triggered()),
            this, SLOT(removeFromPlaylist()));
    connect(dfda, SIGNAL(triggered()),
            this, SLOT(deleteFromDisk()));
    connect(ra, SIGNAL(triggered()),
            this, SIGNAL(Refresh()));
}

void CustomListWidget::contextMenuEvent(QContextMenuEvent *event) // todo
{
    QListWidgetItem *item = itemAt(event->pos());
    if(item)
        menu->exec(viewport()->mapToGlobal(event->pos()));
}

void CustomListWidget::removeFromPlaylist() // todo
{
    removeItemWidget(currentItem());
}

void CustomListWidget::deleteFromDisk() // todo
{

}
