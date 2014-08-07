#ifndef OPENBUTTON_H
#define OPENBUTTON_H

#include <QPushButton>
#include <QMouseEvent>

class OpenButton : public QPushButton
{
    Q_OBJECT
public:
    explicit OpenButton(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);

signals:
    void LeftClick();
    void MiddleClick();
    void RightClick();

};

#endif // OPENBUTTON_H
