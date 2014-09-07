#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QContextMenuEvent>
#include <QAction>

class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit CustomListWidget(QWidget *parent = 0);

    QAction *addAction(const QString &text);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void reordered(int old_index, int new_index); // todo
};

#endif // CUSTOMLISTWIDGET_H
