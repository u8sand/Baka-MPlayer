#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>
#include <QMouseEvent>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    explicit CustomSlider(QWidget *parent = 0);

public slots:
    void setValueNoSignal(int value);

protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif // CUSTOMSLIDER_H
