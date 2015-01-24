#ifndef OVERLAYHANDLER_H
#define OVERLAYHANDLER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QPoint>
#include <QHash>
#include <QFontMetrics>
#include <QFont>

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

    QFont overlay_font;
    QFontMetrics overlay_fm;
    int min_overlay,
        max_overlay,
        overlay_id;
};

#endif // OVERLAYHANDLER_H
