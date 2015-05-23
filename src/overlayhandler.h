#ifndef OVERLAYHANDLER_H
#define OVERLAYHANDLER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QPoint>
#include <QHash>
#include <QFont>
#include <QColor>
#include <QLabel>

class BakaEngine;
class Overlay;

class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    explicit OverlayHandler(QObject *parent = 0);
    ~OverlayHandler();

public slots:
    void showStatusText(const QString &text, int duration = 4000);
    void showInfoText(bool show = true);
    void showText(const QStringList &text, QFont font, QColor color, QPoint pos, int duration, int id = -1);

protected slots:
    void remove(int id);

private:
    BakaEngine *baka;

    QHash<int, Overlay*> overlays;

    int min_overlay,
        max_overlay,
        overlay_id;
};

#endif // OVERLAYHANDLER_H
