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
    if(_index != 0)
    {
        index = _index;
        setMouseTracking(abs(index) >= 1000);
        repaint(this->rect());
    }
}

void IndexButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    if(isEnabled() && index != 0 && abs(index) < 1000)
    {
        QRect region = event->rect();
        QPainter painter(this);
        painter.setPen(QColor(0,0,0));
        painter.setFont(QFont("Open Sans", 6));
        if(index > 0)   // next button
            painter.drawText(region.adjusted(-3, -1, -3, -1), Qt::AlignCenter, QString::number(index));
        else            // previous button
            painter.drawText(region.adjusted(3, -1, 3, -1), Qt::AlignCenter, QString::number(-index));
    }
}

void IndexButton::mouseMoveEvent(QMouseEvent *event)
{
    if(isEnabled() && index > 0)
    {
        QToolTip::showText(event->globalPos(),
                           QString::number(abs(index)),
                           this, rect());
    }
    QPushButton::mouseMoveEvent(event);
}
