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

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QString file, QObject *parent = 0);

public slots:
    void Load();
    void Save();

    QJsonObject getRoot();
    void setRoot(QJsonObject);

protected:
    void LoadIni();
    int ParseLine(QString line);
    QString FixKeyOnLoad(QString key);
    QStringList SplitQStringList(QString list);

private:
    QJsonDocument document;
    QString file;
};

#endif // SETTINGS_H
