#ifndef CUSTOMSPLITTER_H
#define CUSTOMSPLITTER_H

#include <QSplitter>

class CustomSplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit CustomSplitter(QWidget *parent = 0);

    // todo: use position from the right side of the form
    int position() const;
    int normalPosition() const;
    int max() const;

public slots:
    void setPosition(int pos);
    void setNormalPosition(int pos);

protected slots:
    void convertSignal(int pos, int index);

signals:
    void positionChanged(int pos);

private:
    int normalPos;
};

#endif // CUSTOMSPLITTER_H
