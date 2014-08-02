#ifndef OPENBUTTON_H
#define OPENBUTTON_H

#include <QPushButton>
#include <QMouseEvent>

class OpenButton : public QPushButton
{
    Q_OBJECT
public:
    explicit OpenButton(QWidget *parent = 0):
        QPushButton(parent)
    {
    }

signals:
    void LeftClick();
    void MiddleClick();
    void RightClick();

protected:
    void mousePressEvent(QMouseEvent *event)
    {
      if(event->button() == Qt::LeftButton)
        emit LeftClick();
      else if(event->button() == Qt::MiddleButton)
        emit MiddleClick();
      else if(event->button() == Qt::RightButton)
        emit RightClick();
      QPushButton::mousePressEvent(event);
    }
};

#endif // OPENBUTTON_H
