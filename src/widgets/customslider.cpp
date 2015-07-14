#include "customslider.h"

#include <QStyle>
#include <QPainter>
#include <QRect>
#include <QStyle>

CustomSlider::CustomSlider(QWidget *parent):
    QSlider(parent),
    tickReady(false)
{
}

void CustomSlider::setValueNoSignal(int value)
{
    this->blockSignals(true);
    this->setValue(value);
    this->blockSignals(false);
}

void CustomSlider::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
      setValue(QStyle::sliderValueFromPosition(minimum(), maximum(), event->x(), width()));
      event->accept();
  }
  QSlider::mousePressEvent(event);
}

void CustomSlider::setTicks(QList<int> values)
{
    ticks = values; // just set the values
    tickReady = false; // ticks need to be converted when totalTime is obtained
}

void CustomSlider::readyTicks()
{
    if(ticks.length() > 0)
    {
        tickReady = true; // ticks are ready to be displayed
        repaint(rect());
    }
}

void CustomSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
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
