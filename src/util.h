#ifndef UTIL_H
#define UTIL_H

#include <QWidget>
#include <QString>

#include "settings.h"

namespace Util {

// platform specific
QString VersionFileUrl();
QString DownloadFileUrl();

bool DimLightsSupported();
void SetAlwaysOnTop(WId wid, bool);
Settings *InitializeSettings(QObject *parent);

bool IsValidFile(QString path);
bool IsValidLocation(QString loc); // combined file and url

// common
bool IsValidUrl(QString url);

QString FormatTime(int time, int totalTime);
QString FormatRelativeTime(int time);
QString FormatNumber(int val, int length);
QString FormatNumberWithAmpersand(int val, int length);
QString HumanSize(qint64);
QString ShortenPathToParent(const QString &path);
QStringList ToNativeSeparators(QStringList list);
QStringList FromNativeSeparators(QStringList list);

}

#endif // UTIL_H
