#ifndef PLATFORM_H
#define PLATFORM_H

#include <QWidget>
#include <QSettings>

namespace Platform {

bool DimLightsSupported();
void SetAlwaysOnTop(WId wid, bool);
QSettings *InitializeSettings(QObject *parent);

}

#endif // PLATFORM_H

