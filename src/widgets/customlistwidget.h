#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QMenu>
#include <QContextMenuEvent>

class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit CustomListWidget(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void reordered(int old_index, int new_index); // todo
};

#endif // CUSTOMLISTWIDGET_H
