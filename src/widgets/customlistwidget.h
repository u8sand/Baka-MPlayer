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

protected slots:
    void removeFromPlaylist();
    void deleteFromDisk();

signals:
    void Refresh();

private:
    QMenu *menu;
};

#endif // CUSTOMLISTWIDGET_H
