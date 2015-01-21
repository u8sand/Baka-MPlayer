#include "updatemanager.h"

#include <QCoreApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QStringList>
#include <QFile>
#include <QProcess>

#if defined(Q_OS_WIN)
#include <zip.h>
#endif

#include "bakaengine.h"
#include "util.h"

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    manager(new QNetworkAccessManager(this)),
    busy(false)
{

}

UpdateManager::~UpdateManager()
{
    delete manager;
}

bool UpdateManager::CheckForUpdates()
{
    if(busy)
        return false;
    busy = true;
    emit messageSignal(tr("Checking for updates...\n"));

    QNetworkRequest request(Util::VersionFileUrl());
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
                    emit messageSignal(reply->errorString());
                else
                {
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
                        emit progressSignal((int)(cur += amnt));
                    }
                    emit progressSignal(100);
                    busy = false;
                }
                reply->deleteLater();
            });
    return true;
}

#if defined(Q_OS_WIN)
bool UpdateManager::DownloadUpdate(const QString &url, const QString &version)
{
    if(busy)
        return false;
    busy = true;
    emit messageSignal(tr("Downloading update...\n"));

    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    QString filename = QString("%0\\Baka-MPlayer-%1.exe").arg(QCoreApplication::applicationDirPath(), version);
    QFile *file = new QFile(filename);
    if(!file->open(QFile::WriteOnly | QFile::Truncate))
    {
        emit messageSignal(tr("fopen error\n"));
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
                    emit messageSignal(reply->errorString());
                else if(file->write(reply->read(reply->bytesAvailable()), reply->bytesAvailable()) == -1)
                    emit messageSignal(tr("write error\n"));
            });

    connect(reply, &QNetworkReply::finished,
            [=]
            {
                if(reply->error())
                {
                    emit messageSignal(reply->errorString());
                    file->close();
                    delete file;
                }
                else
                {
                    file->flush();
                    file->close();
                    delete file;
                    emit messageSignal(tr("Download complete\n"));
                    busy = false;
                    ApplyUpdate(filename);
                }
                reply->deleteLater();
            });
    return true;
}

void UpdateManager::ApplyUpdate(const QString &file)
{
    emit messageSignal(tr("Extracting..."));
    // todo: extract  (to baka-mplayer-update)
//    int err;
//    zip *z = zip_open(file, 0, &err);
//    zip_set_file_compression(z, ZIP_CM_DEFLATE64, ZIP_CM_DEFAULT)
//        zip_close(z);
//    zip_close(z);
    QFile f("baka-mplayer-updater.bat");
    if(!f.open(QFile::Truncate))
    {
        emit messageSignal(tr("Could not open file for writing..."));
        return;
    }
    f.write(
        QString(
        "@echo off\r\n"
        "echo Updating baka-mplayer...\r\n"
        "ping 127.0.0.1 -n 1 -w 1000 > NUL\r\n"
        "cd baka-mplayer-update\r\n"
        "for %%i in (*) do move \"%%i\" ..\r\n"
        "for /d %%i in (*) do move \"%%i\" ..\r\n"
        "move /Y /r \"%0\" \".\" > NUL\r\n"
        "start /b \"\" cmd /c del \"%%~f0\"&exit /b\"\"\r\n").arg(
            "baka-mplayer").toUtf8());
    f.close();
    QProcess::startDetached("baka-mplayer-updater.bat");
    emit messageSignal(tr("Done."));
    baka->Quit();
}
#endif
