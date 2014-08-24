#ifndef CUSTOMSPLITTER_H
#define CUSTOMSPLITTER_H

#include <QSplitter>

class CustomSplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit CustomSplitter(QWidget *parent = 0);

    QList<int> normalSize();
    bool isCollapsed(int which);

public slots:
    void SetCollapse(bool collapse, int which = -1);
    void setSizes(const QList<int> &list);

private slots:
    void UpdateSize();

signals:
    void sizesChanged();

private:
    int size[2];
    bool collapsed[2];
};

#endif // CUSTOMSPLITTER_H
