#include "updatemanager.h"

#include <QNetworkRequest>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QFile>

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent),
    manager(new QNetworkAccessManager(this))
{
}

void UpdateManager::CheckForUpdates()
{
    emit progressSignal(0);
    QNetworkRequest request(QUrl("http://bakamplayer.u8sand.net/version"));
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::downloadProgress,
            [=](qint64 received, qint64 total)
            {
                emit progressSignal((int)(50.0*received/total));
            });

    connect(reply, &QNetworkReply::finished,
            [=]
            {
                if(reply->error())
                    emit errorSignal(reply->errorString());
                else
                {
                    emit progressSignal(50); // when we've gotten the data 50% is already done
                    QMap<QString, QString> info;
                    QList<QByteArray> lines = reply->readAll().split('\n');
                    QList<QByteArray> pair;
                    QString lastPair;
                    // go through the next 50% incrementally during parsing
                    double amnt = 50.0/lines.length(),
                           cur = 50+amnt;
                    for(auto line : lines)
                    {
                        if((pair = line.split('=')).size() != 2)
                            info[lastPair].append(line);
                        else
                            info[(lastPair = pair[0])] = QString(pair[1]);
                        progressSignal((int)(cur += amnt));
                    }
                    emit progressSignal(100);
                    emit versionInfoReceived(info);
                }
                reply->deleteLater();
            });
}

//#if defined(Q_OS_WIN)
void UpdateManager::DownloadUpdate()
{
    emit verboseSignal("Downloading update...");
    QNetworkRequest request(QUrl("http://bakamplayer.u8sand.net/Baka MPlayer.7z"));
    QNetworkReply *reply = manager->get(request);
    QFile *file = new QFile("Baka MPlayer.exe");
    file->open(QFile::Truncate);

    connect(reply, &QNetworkReply::downloadProgress,
            [=](qint64 received, qint64 total)
            {
                emit progressSignal((int)(100.0*received/total));
            });

    connect(reply, &QNetworkReply::readyRead,
            [=]
            {
                if(reply->error())
                    emit errorSignal(reply->errorString());
                else
                    file->write(reply->read(reply->bytesAvailable()));
            });

    connect(reply, &QNetworkReply::finished,
            [=]
            {
                if(reply->error())
                    emit errorSignal(reply->errorString());
                else
                {
                    file->flush();
                    file->close();
                    delete file;
                    emit verboseSignal("Downloaded");
                }
                reply->deleteLater();
            });
}

void UpdateManager::ApplyUpdate()
{
    emit verboseSignal("Applying update...");
    // todo
    emit verboseSignal("Done.");
}
//#endif
