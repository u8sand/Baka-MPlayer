#ifndef GESTUREHANDLER_H
#define GESTUREHANDLER_H

#include <QObject>
#include <QElapsedTimer>
#include <QPoint>

class BakaEngine;

class GestureHandler : public QObject
{
    Q_OBJECT
public:
    enum GESTURE_TYPE {
        MOVE,
        HSEEK_VVOLUME
    };

    enum GESTURE_STATE {
        NONE,
        SEEKING,
        ADJUSTING_VOLUME
    };

    explicit GestureHandler(QObject *parent = 0);

public slots:
    bool Begin(int gesture_type, QPoint mousePos, QPoint windowPos);
    bool Process(QPoint mousePos);
    bool End();

private:
    BakaEngine *baka;

    QElapsedTimer elapsedTimer;

    double hRatio,
           vRatio;
    int timer_threshold,
        gesture_threshold;

    int gesture_type,
        gesture_state;
    struct
    {
        QPoint mousePos,
               windowPos;
        int    time,
               volume;
    } start;
};

#endif // GESTUREHANDLER_H
