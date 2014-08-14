#include "customslider.h"

#include <QTime>
#include <QToolTip>
#include <QPainter>
#include <QRect>

CustomSlider::CustomSlider(QWidget *parent):
    QSlider(parent),
    tickReady(false),
    totalTime(0)
{
}

void CustomSlider::setTracking(int _totalTime)
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

void CustomSlider::setValueNoSignal(int value)
{
    this->blockSignals(true);
    this->setValue(value);
    this->blockSignals(false);
}

void CustomSlider::setTicks(QList<int> values)
{
    ticks = values; // just set the values
    tickReady = false; // ticks need to be converted when totalTime is obtained
}

QString CustomSlider::formatTrackingTime(int _time)
{
    QTime time = QTime::fromMSecsSinceStartOfDay((int)(((double)_time/maximum())*totalTime) * 1000);
    if(totalTime >= 3600)           // hours
        return time.toString("h:mm:ss");
    if(totalTime >= 60)             // minutes
        return time.toString("mm:ss");
    return time.toString("0:ss");   // seconds
}

void CustomSlider::mouseMoveEvent(QMouseEvent* event)
{
    if(totalTime != 0)
    {
        // Thanks to cmannett85: http://stackoverflow.com/questions/12417636/qt-show-mouse-position-like-tooltip
        //  and spyke (see below) for initial code
        if (orientation() == Qt::Vertical)
        {
            QToolTip::showText(event->globalPos(),
                               formatTrackingTime(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height()),
                               this, rect());
        }
        else
        {
            QToolTip::showText(event->globalPos(),
                               formatTrackingTime(minimum() + ((maximum()-minimum()) * event->x()) / width()),
                               this, rect());
        }
    }
    QSlider::mouseMoveEvent(event);
}

void CustomSlider::mousePressEvent(QMouseEvent *event)
{
  // Thanks to spyke: http://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
  if (event->button() == Qt::LeftButton)
  {
      if (orientation() == Qt::Vertical)
          setValue(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height());
      else
          setValue(minimum() + ((maximum()-minimum()) * event->x()) / width());

      event->accept();
  }
  QSlider::mousePressEvent(event);
}

void CustomSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
    if(isEnabled() && tickReady)
    {
        QRect region = event->rect();
        QPainter painter(this);
        painter.setPen(QColor(255,255,255));
        for(auto &tick : ticks)
        {
            if(orientation() == Qt::Vertical)
            {
                int y = height() - (tick-minimum())*height()/(maximum()-minimum()); // the inverse of above
                painter.drawLine(region.left(), y, region.right(), y);
            }
            else
            {
                int x = (tick-minimum())*width()/(maximum()-minimum()); // the inverse of above
                painter.drawLine(x, region.top(), x, region.bottom());
            }
        }
    }
}
