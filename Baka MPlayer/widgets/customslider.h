#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QMouseEvent>
#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    explicit CustomSlider(QWidget *parent = 0);

public slots:
    void setTracking(int _totalTime);
    void setValueNoSignal(int value);

protected:
    QString formatTrackingTime(int _time);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);

private:
    int totalTime;
};

#endif // CUSTOMSLIDER_H
