#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QMouseEvent>
#include <QList>

#include "customslider.h"

class SeekBar : public CustomSlider
{
    Q_OBJECT
public:
    explicit SeekBar(QWidget *parent = 0);

public slots:
    void setTracking(int _totalTime);

protected:
    QString formatTrackingTime(int _time);

    void mouseMoveEvent(QMouseEvent* event);

private:
    int totalTime;
};

#endif // SEEKBAR_H
