#include "customlistwidget.h"

#include <QListWidgetItem>
#include <QMenu>

CustomListWidget::CustomListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

QAction *CustomListWidget::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    QListWidget::addAction(action);
    return action;
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
