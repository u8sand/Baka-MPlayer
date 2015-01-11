#include "customslider.h"

#include <QStyle>

CustomSlider::CustomSlider(QWidget *parent):
    QSlider(parent)
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
