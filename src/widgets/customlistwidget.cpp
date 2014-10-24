#include "customlistwidget.h"

#include <QListWidgetItem>
#include <QMenu>

CustomListWidget::CustomListWidget(QWidget *parent) :
    QListWidget(parent),
    showAll(false)
{
}

QAction *CustomListWidget::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    QListWidget::addAction(action);
    return action;
}

void CustomListWidget::SelectItem(const QString &item)
{
    setCurrentItem(*findItems(item, Qt::MatchExactly).begin());
}

QString CustomListWidget::PreviousItem()
{
    return item(currentRow()-1)->text();
}

QString CustomListWidget::NextItem()
{
    return item(currentRow()+1)->text();
}

void CustomListWidget::Search(QString s)
{
    // todo: narrow list based on search
}

void CustomListWidget::ShowAll(bool b)
{
    // todo: narrow list based on suffix
}

void CustomListWidget::Shuffle(bool b)
{
    // todo: shuffle list
}

bool CustomListWidget::isShowAll()
{
    return showAll;
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
