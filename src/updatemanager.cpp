#include "updatemanager.h"

#include <QNetworkRequest>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QFile>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
}

void UpdateManager::CheckForUpdates()
{
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(CheckForUpdatesReply(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://bakamplayer.u8sand.net/version")));
}

void UpdateManager::DownloadUpdate()
{
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(DownloadUpdateReply(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://bakamplayer.u8sand.net/Baka MPlayer.exe")));
}

void UpdateManager::CheckForUpdatesReply(QNetworkReply *reply)
{
    if(reply->error())
        emit errorSignal("UpdateManager: "+reply->errorString());
    else
    {
        QMap<QString, QString> info;
        QList<QByteArray> lines = reply->readAll().split('\n');
        QList<QByteArray> pair;
        for(auto line : lines)
        {
            pair = line.split('=');
            info[QString(pair[0])] = QString(pair[1].replace('\r','\n')); // for multi-line info use \r's instead of \n's
        }
        emit Update(info);
    }
    reply->deleteLater();
}

void UpdateManager::DoanloadUpdateReply(QNetworkReply *reply)
{
    if(reply->error())
        emit errorSignal("UpdateManager: "+reply->errorString());
    else
    {
        // todo: progress bar
        // todo: compression
        QFile *file = new QFile("Baka MPlayer.exe");
        if(file->open(QFile::Truncate))
        {
            file->write(reply->readAll());
            file->flush();
            file->close();
        }
        delete file;
    }
    reply->deleteLater();
}
