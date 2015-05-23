#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include <QLabel>
#include <QImage>
#include <QTimer>

class Overlay : public QObject
{
    Q_OBJECT
public:
    explicit Overlay(QLabel *label, QImage *canvas, QTimer *timer, QObject *parent = 0);
    ~Overlay();

private:
    QLabel *label;
    QImage *canvas;
    QTimer *timer;
};

#endif // OVERLAY_H
