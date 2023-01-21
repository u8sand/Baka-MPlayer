#ifndef OVERLAY_H
#define OVERLAY_H

#include <QLabel>
#include <QTimer>
#include <QScopedPointer>

struct Overlay {
    Overlay(QScopedPointer<QLabel> &label, QScopedPointer<QTimer> &timer);

    QScopedPointer<QLabel> label;
    QScopedPointer<QTimer> timer;
};

#endif // OVERLAY_H
