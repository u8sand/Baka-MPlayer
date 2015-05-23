#include "overlay.h"

Overlay::Overlay(QLabel *label, QImage *canvas, QTimer *timer, QObject *parent):
    QObject(parent)
{
    this->label = label;
    this->canvas = canvas;
    this->timer = timer;
}

Overlay::~Overlay()
{
    delete label;
    delete canvas;
    if(timer != nullptr)
        delete timer;
}
