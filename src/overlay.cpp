#include "overlay.h"

Overlay::Overlay():label(nullptr), timer(new QTimer()) {}
Overlay::Overlay(const Overlay &overlay):label(overlay.label), timer(overlay.timer) {}
Overlay::~Overlay() {}
