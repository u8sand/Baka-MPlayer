#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class BakaEngine;

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QString location, QObject *parent = 0);
    ~Settings();

public slots:
    void Load();
    void Save();

    QJsonObject getConfigRoot();
    void setConfigRoot(QJsonObject);
    QJsonObject getRecentRoot();
    void setRecentRoot(QJsonObject);

protected:
    void LoadIni();
    int ParseLine(QString line);
    QString FixKeyOnLoad(QString key);
    QStringList SplitQStringList(QString list);

private:
    BakaEngine *baka;
    QJsonDocument config;
    QJsonDocument recent;
    QString location;
};

#endif // SETTINGS_H
