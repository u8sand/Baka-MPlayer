#ifndef RECENT_H
#define RECENT_H

#include <QString>

struct Recent
{
    Recent(QString s = QString(), QString t = QString(), int p = 0):
        path(s), title(t), time(p) {}

    operator QString() const
    {
        return path;
    }

    bool operator==(const Recent &recent) const
    {
	return (this->path == recent.path);
    }

    bool operator==(const QString &s) const
    {
	return (this->path == s);
    }	

    QString path,
            title;
    int     time;
};

#endif // RECENT_H
