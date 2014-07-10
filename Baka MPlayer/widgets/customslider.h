#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QMouseEvent>
#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    explicit CustomSlider(QWidget *parent = 0):
        QSlider(parent)
    {

    }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
      // Thanks to spyke: http://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
      if (event->button() == Qt::LeftButton)
      {
          if (orientation() == Qt::Vertical)
              setValue(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height() ) ;
          else
              setValue(minimum() + ((maximum()-minimum()) * event->x()) / width() ) ;

          event->accept();
      }
      QSlider::mousePressEvent(event);
    }
};

#endif // CUSTOMSLIDER_H
