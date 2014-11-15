#include "openbutton.h"

OpenButton::OpenButton(QWidget *parent):
    QPushButton(parent)
{
}

void OpenButton::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit LeftClick();
    else if(event->button() == Qt::MiddleButton)
        emit MiddleClick();
    else if(event->button() == Qt::RightButton)
        emit RightClick();
    event->accept();
    QPushButton::mousePressEvent(event);
}
