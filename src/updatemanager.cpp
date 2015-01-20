#include "updatemanager.h"

#include <QCoreApplication>
#include <QNetworkRequest>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QFile>

#include "bakaengine.h"
#include "util.h"

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    manager(new QNetworkAccessManager(this))
{
}

UpdateManager::~UpdateManager()
{
    delete manager;
}

void UpdateManager::CheckForUpdates()
{
    QNetworkRequest request(QUrl(Util::VersionFileUrl()));
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

#if defined(Q_OS_WIN)
void UpdateManager::DownloadUpdate(QString url, QString version)
{
    // todo: handle redirects
/*
    emit verboseSignal("Downloading update...");
    QNetworkRequest request(QUrl(url));
    QNetworkReply *reply = manager->get(request);
    QFile *file = new QFile(QCoreApplication::applicationDirPath()+"\\Baka-MPlayer-"+version+".exe");
    if(!file->open(QFile::WriteOnly | QFile::Truncate))
    {
        emit errorSignal("write error");
        reply->abort();
        delete file;
    }

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
                else if(file->write(reply->read(reply->bytesAvailable()), reply->bytesAvailable()) == -1)
                    emit errorSignal("write error");
            });

    connect(reply, &QNetworkReply::finished,
            [=]
            {
                if(reply->error())
                {
                    emit errorSignal(reply->errorString());
                    file->close();
                    delete file;
                }
                else
                {
                    file->flush();
                    file->close();
                    delete file;
                    emit verboseSignal("Downloaded");
                    ApplyUpdate("");
                }
                reply->deleteLater();
            });
*/
}

void UpdateManager::ApplyUpdate(QString version)
{
    emit verboseSignal("Applying update...");

    // extract?
    // execute new version ("Baka-MPlayer-"+version+".exe") passing --update BAKA_MPLAYER_VERSION

    emit verboseSignal("Done.");
}
#endif
