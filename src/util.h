#ifndef UTIL_H
#define UTIL_H

#include <QString>

namespace BakaUtil
{
    QString FormatTime(int time, int totalTime);
    QString FormatNumber(int val, int length);
    QString HumanSize(qint64);
}

#endif // UTIL_H
