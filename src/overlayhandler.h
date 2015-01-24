#ifndef OVERLAYHANDLER_H
#define OVERLAYHANDLER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QPoint>
#include <QHash>

class BakaEngine;

class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    explicit OverlayHandler(QObject *parent = 0);
    ~OverlayHandler();

public slots:
    void showText(QString text, int duration);
    void showText(QString text, int duration, QPoint pos, int id = -1);
private:
    BakaEngine *baka;

    struct overlay
    {
        QImage *canvas;
        QTimer *timer;
    };
    QHash<int, overlay*> overlays;

    const int min_overlay = 5,
              max_overlay = 60;
    int overlay_id;
};

#endif // OVERLAYHANDLER_H
