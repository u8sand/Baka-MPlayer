#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
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
    void DownloadUpdate();

protected slots:
    void CheckForUpdatesReply(QNetworkReply *reply);
    void DoanloadUpdateReply(QNetworkReply *reply);

signals:
    void Update(QMap<QString, QString> info);
    void errorSignal(QString err);

private:
    QNetworkAccessManager *manager;
};

#endif // UPDATEMANAGER_H
