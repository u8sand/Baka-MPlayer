#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QListWidget>
#include <QContextMenuEvent>
#include <QAction>

class PlaylistWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlaylistWidget(QWidget *parent = 0);

    QAction *addAction(const QString &text);
    QString RemoveItem(int index);
    void SelectItem(const QString &item);
    void Populate(QStringList list);

    QString PreviousItem();
    QString NextItem();

    void Search(QString);
    void ShowAll(bool);
    void Shuffle(bool);
protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QStringList playlist;
};

#endif // PLAYLISTWIDGET_H
