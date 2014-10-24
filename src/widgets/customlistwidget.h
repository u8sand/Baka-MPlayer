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
    bool showAll;
};

#endif // CUSTOMLISTWIDGET_H
