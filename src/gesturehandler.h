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
    ~GestureHandler();

public slots:
    bool Begin(int gesture_type, QPoint mousePos, QPoint windowPos);
    bool Process(QPoint mousePos);
    bool End();

private:
    BakaEngine *baka;

    QElapsedTimer *elapsedTimer;

    double hRatio = 0.05,
           vRatio = 0.10;

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
