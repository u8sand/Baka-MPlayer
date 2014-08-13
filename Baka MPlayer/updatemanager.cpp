#include "updatemanager.h"

#include <QNetworkRequest>
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
        emit ErrorSignal("UpdateManager: "+reply->errorString());
    else
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

void UpdateManager::DoanloadUpdateReply(QNetworkReply *reply)
{
    if(reply->error())
        emit ErrorSignal("UpdateManager: "+reply->errorString());
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
