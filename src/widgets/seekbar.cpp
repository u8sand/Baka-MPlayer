#include "seekbar.h"

#include <QTime>
#include <QToolTip>
#include <QPainter>
#include <QRect>

#include "util.h"

using namespace BakaUtil;

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
            tick = (int)(((double)tick/totalTime)*maximum());
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
        // Thanks to cmannett85: http://stackoverflow.com/questions/12417636/qt-show-mouse-position-like-tooltip
        //  and spyke (see below) for initial code
        // note: there is no real way to find the size of the generated tooltip...
        // these values work best for xx:xx size; it might be possible to hardcode sizes
        // for the other most common formats eg. xx:xx:xx, x:xx:xx, xx:xx, x:xx
        QToolTip::showText(QPoint(event->globalX()-25, mapToGlobal(rect().topLeft()).y()-40),
                           FormatTime((minimum() + ((maximum()-minimum()) * event->x()) / width())/maximum()*totalTime, totalTime),
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
            int x = (tick-minimum())*width()/(maximum()-minimum());
            painter.drawLine(x, region.top(), x, region.bottom());
        }
    }
}
