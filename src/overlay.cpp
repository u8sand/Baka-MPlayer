#include "overlay.h"

Overlay::Overlay(QScopedPointer<QLabel> &label, QScopedPointer<QTimer> &timer):
    label(label.take()),
    timer(timer.take())
{}
