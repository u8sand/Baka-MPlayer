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
    void SelectItem(const QString &item, bool internal = false);
    void BoldText(const QString &item, bool state);
    void Populate(QStringList list);

    QString FirstItem();
    QString CurrentItem();
    QString PreviousItem();
    QString NextItem();
    QString FileAt(int index);

    void Search(QString);
    void ShowAll(bool);
    void Shuffle(bool);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QStringList playlist;
    QString cItem;
};

#endif // PLAYLISTWIDGET_H
