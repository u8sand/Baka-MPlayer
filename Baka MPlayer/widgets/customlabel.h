#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>
#include <QMouseEvent>

class CustomLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CustomLabel(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked();
};

#endif // CUSTOMLABEL_H
