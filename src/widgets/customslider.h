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
    void setTicks(QList<int> values);
    void readyTicks();

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    QList<int> ticks;
    bool tickReady;
};

#endif // CUSTOMSLIDER_H
