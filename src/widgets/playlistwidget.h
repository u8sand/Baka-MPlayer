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
    void SelectItem(const QString &item);

    QString PreviousItem();
    QString NextItem();

    void Search(QString);
    void ShowAll(bool);
    void Shuffle(bool);

    bool isShowAll();
protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QStringList playlist;
    QString search;
    bool showAll;
};

#endif // PLAYLISTWIDGET_H
