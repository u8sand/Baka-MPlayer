#include "seekbar.h"

#include <QTime>
#include <QToolTip>
#include <QPainter>
#include <QRect>
#include <QStyle>

#include "util.h"

SeekBar::SeekBar(QWidget *parent):
    CustomSlider(parent),
    tickReady(false),
    totalTime(0)
{
}

void SeekBar::setTracking(int _totalTime)
{
    if(_totalTime != 0)
    {
        totalTime = _totalTime;
        // now that we've got totalTime, calculate the tick locations
        // we need to do this because totalTime is obtained after the LOADED event is fired--we need totalTime for calculations
        for(auto &tick : ticks)
            tick = ((double)tick/totalTime)*maximum();
        if(ticks.length() > 0)
        {
            tickReady = true; // ticks are ready to be displayed
            repaint(rect());
        }
        setMouseTracking(true);
    }
    else
        setMouseTracking(false);
}

void SeekBar::setTicks(QList<int> values)
{
    ticks = values; // just set the values
    tickReady = false; // ticks need to be converted when totalTime is obtained
}

void SeekBar::mouseMoveEvent(QMouseEvent* event)
{
    if(totalTime != 0)
    {
        QToolTip::showText(QPoint(event->globalX()-25, mapToGlobal(rect().topLeft()).y()-40),
                           Util::FormatTime(QStyle::sliderValueFromPosition(minimum(), maximum(), event->x(), width())*(double)totalTime/maximum(), totalTime),
                           this, rect());
    }
    QSlider::mouseMoveEvent(event);
}

void SeekBar::paintEvent(QPaintEvent *event)
{
    CustomSlider::paintEvent(event);
    if(isEnabled() && tickReady)
    {
        QRect region = event->rect();
        QPainter painter(this);
        painter.setPen(QColor(190,190,190));
        for(auto &tick : ticks)
        {
            int x = QStyle::sliderPositionFromValue(minimum(), maximum(), tick, width());
            painter.drawLine(x, region.top(), x, region.bottom());
        }
    }
}
