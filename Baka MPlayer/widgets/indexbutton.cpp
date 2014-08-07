#include "indexbutton.h"

#include <QRect>
#include <QPainter>
#include <QColor>
#include <QFont>
#include <QToolTip>
#include <QString>

IndexButton::IndexButton(QWidget *parent) :
    QPushButton(parent),
    index(0)
{
}

int IndexButton::getIndex() const
{
    return index;
}

void IndexButton::setIndex(int _index)
{
    if(index >= 0)
    {
        index = _index;
        setMouseTracking(index >= 1000);
    }
}

void IndexButton::paintEvent(QPaintEvent *event)
{
    if(index > 0 && index < 1000)
    {
        QRect region = event->rect();
        QPainter painter(this);
        QFont font("Open Sans", 5, QFont::Bold);
        painter.setPen(QColor(255,255,255));
        painter.setFont(font);
        painter.drawText(region.center()+QPoint(0,3), QString::number(index));
    }
    QPushButton::paintEvent(event);
}

void IndexButton::mouseMoveEvent(QMouseEvent *event)
{
    if(index > 0)
    {
        QToolTip::showText(event->globalPos(),
                           QString::number(index),
                           this, rect());
    }
    QPushButton::mouseMoveEvent(event);
}
