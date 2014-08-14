#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QList>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    explicit CustomSlider(QWidget *parent = 0);

public slots:
    void setTracking(int _totalTime);
    void setValueNoSignal(int value);
    void setTicks(QList<int> values);

protected:
    QString formatTrackingTime(int _time);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QList<int> ticks;
    bool tickReady;
    int totalTime;
};

#endif // CUSTOMSLIDER_H
