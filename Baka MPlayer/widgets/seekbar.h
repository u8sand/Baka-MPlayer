#ifndef SEEKBAR_H
#define SEEKBAR_H

#include <QMouseEvent>
#include <QPaintEvent>
#include <QList>

#include "customslider.h"

class SeekBar : public CustomSlider
{
    Q_OBJECT
public:
    explicit SeekBar(QWidget *parent = 0);

public slots:
    void setTracking(int _totalTime);
    void setTicks(QList<int> values);

protected:
    QString formatTrackingTime(int _time);

    void mouseMoveEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);

private:
    QList<int> ticks;
    bool tickReady;
    int totalTime;
};

#endif // SEEKBAR_H
