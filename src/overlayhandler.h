#ifndef OVERLAYHANDLER_H
#define OVERLAYHANDLER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QPoint>
#include <QHash>
#include <QFontMetrics>
#include <QFont>
#include <QLabel>

class BakaEngine;

class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    explicit OverlayHandler(QObject *parent = 0);
    ~OverlayHandler();

public slots:
    void showStatusText(const QString &text, int duration = 4000);
    void showInfoText(bool show = true);

    void showText(const QString &text, int duration, QPoint pos, int id = -1);

protected slots:
    void setFont(int n);

private:
    BakaEngine *baka;

    struct overlay
    {
        QLabel *label;
        QImage *canvas;
        QTimer *timer;
    };
    QHash<int, overlay*> overlays;

    QFont overlay_font;
    QFontMetrics overlay_fm;
    int min_overlay,
        max_overlay,
        overlay_id;

    QString info;
};

#endif // OVERLAYHANDLER_H
