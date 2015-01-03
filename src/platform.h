#ifndef PLATFORM_H
#define PLATFORM_H

#include <QWidget>
#include <QSettings>
#include <QString>

namespace Platform {

bool DimLightsSupported();
void SetAlwaysOnTop(WId wid, bool);
QSettings *InitializeSettings(QObject *parent);

bool IsValidFile(QString path);
bool IsValidUrl(QString url);
bool IsValidLocation(QString loc); // combined file and url

}

#endif // PLATFORM_H

