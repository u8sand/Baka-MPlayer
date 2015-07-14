#include "seekbar.h"

#include <QTime>
#include <QToolTip>
#include <QStyle>

#include "util.h"

SeekBar::SeekBar(QWidget *parent):
    CustomSlider(parent),
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
        readyTicks();
        setMouseTracking(true);
    }
    else
        setMouseTracking(false);
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
