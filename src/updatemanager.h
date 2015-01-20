#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QString>

class BakaEngine;

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = 0);
    ~UpdateManager();

public slots:
    void CheckForUpdates();
#if defined(Q_OS_WIN)
    void DownloadUpdate(QString url, QString version);
    void ApplyUpdate(QString version);
signals:
    void Downloaded(int);
#else
signals:
#endif
    void versionInfoReceived(QMap<QString, QString> info);

    void progressSignal(int percent);

    void errorSignal(QString err);
    void verboseSignal(QString msg);

private:
    BakaEngine *baka;
    QNetworkAccessManager *manager;
};

#endif // UPDATEMANAGER_H
