#include "customlistwidget.h"

#include <QListWidgetItem>

CustomListWidget::CustomListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

void CustomListWidget::contextMenuEvent(QContextMenuEvent *event)
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
