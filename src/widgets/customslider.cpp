#include "customslider.h"

CustomSlider::CustomSlider(QWidget *parent):
    QSlider(parent)
{
    connect(this, SIGNAL(sliderMoved(int)),
            this, SLOT(setValue(int)));
}

void CustomSlider::setValueNoSignal(int value)
{
    this->blockSignals(true);
    this->setValue(value);
    this->blockSignals(false);
}

void CustomSlider::mousePressEvent(QMouseEvent *event)
{
  // Thanks to spyke: http://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
  if (event->button() == Qt::LeftButton)
  {
//      if (orientation() == Qt::Vertical)
//          setValue(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height());
//      else // we're always horizontal, extra code not needed
      setValue(minimum() + double((maximum()-minimum()) * event->x()) / width());
      event->accept();
  }
  QSlider::mousePressEvent(event);
}
