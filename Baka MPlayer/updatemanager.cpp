#include "updatemanager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QStringList>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
}

void UpdateManager::CheckForUpdates()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(reply(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://bakamplayer.u8sand.net/version")));
}

void UpdateManager::Ready(QNetworkReply *reply)
{
    if(reply->error())
        emit ErrorSignal("UpdateManager: "+reply->errorString());
    else // todo
    {
        QMap<QString> info;
        QStringList lines = reply->readAll().split('\n');
        QStringList pair;
        for(auto line : lines)
        {
            pair = line.split('=');
            info[pair[0]] = pair[1].replace('\r','\n'); // for multi-line info use \r's instead of \n's
        }
        emit Update(info);
    }
    reply->deleteLater();
}
