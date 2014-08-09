#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include <QMap>
#include <QString>

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = 0);

public slots:
    void CheckForUpdates();

protected slots:
    void reply(QNetworkReply *reply);

signals:
    void Update(QMap<QString> info);
    void ErrorSignal(QString err);

private:

};

#endif // UPDATEMANAGER_H
