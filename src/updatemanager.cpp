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
#include <QSharedPointer>

#if defined(Q_OS_WIN)
#include <QDir>
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

bool UpdateManager::CheckForUpdates()
{
    if(busy)
        return false;
    busy = true;
    emit messageSignal(tr("Checking for updates..."));
    emit progressSignal(0);
    QNetworkRequest request(Util::VersionFileUrl());
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::downloadProgress,
            reply, [=](qint64 received, qint64 total)
            {
                emit progressSignal((int)(50.0*received/total));
            });

    connect(reply, &QNetworkReply::finished,
            reply, [=]
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
                    for(auto &line : lines)
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
bool UpdateManager::DownloadUpdate(const QString &url)
{
    if(busy)
        return false;
    busy = true;
    emit messageSignal(tr("Downloading update..."));
    emit progressSignal(0);
    QNetworkRequest request(url);
    QString filename = QDir::toNativeSeparators(QString("%0/Baka-MPlayer.zip").arg(QCoreApplication::applicationDirPath()));
    QSharedPointer<QFile> file(new QFile(filename));
    if(!file->open(QFile::WriteOnly | QFile::Truncate))
    {
        emit messageSignal(tr("fopen error\n"));
        busy = false;
        return false;
    }

    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::downloadProgress,
            reply, [=](qint64 received, qint64 total)
            {
                emit progressSignal((int)(99.0*received/total));
            });

    connect(reply, &QNetworkReply::readyRead,
            reply, [=]
            {
                if(reply->error())
                    emit messageSignal(reply->errorString());
                else if(file->write(reply->read(reply->bytesAvailable()), reply->bytesAvailable()) == -1)
                    emit messageSignal(tr("write error"));
            });

    connect(reply, &QNetworkReply::finished,
            reply, [=]
            {
                if(reply->error())
                {
                    emit messageSignal(reply->errorString());
                    file->close();
                    file->deleteLater();
                }
                else
                {
                    file->flush();
                    file->close();
                    file->deleteLater();
                    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                    if(redirect.isEmpty())
                    {
                        busy = false;
                        emit messageSignal(tr("Download complete"));
                        ApplyUpdate(filename);
                        emit progressSignal(100);
                    }
                    else
                    {
                        emit messageSignal(tr("Redirected..."));
                        busy = false;
                        DownloadUpdate(redirect.toString());
                    }
                }
                reply->deleteLater();
            });
    return true;
}

void UpdateManager::ApplyUpdate(const QString &file)
{
    emit messageSignal(tr("Extracting..."));
    // create a temporary directory for baka
    QString path = QDir::toNativeSeparators(QString("%0/.tmp/").arg(QCoreApplication::applicationDirPath()));
    QString exe = QDir::toNativeSeparators(QString("%0/Baka MPlayer.exe").arg(QCoreApplication::applicationDirPath()));
    QString bat = QDir::toNativeSeparators(QString("%0/updater.bat").arg(QCoreApplication::applicationDirPath()));
    QDir dir;
    dir.mkpath(path);
    int err;
    struct zip *z = zip_open(file.toUtf8(), 0, &err);
    int n = zip_get_num_entries(z, 0);
    for(int64_t i = 0; i < n; ++i)
    {
        // get file stats
        struct zip_stat s;
        zip_stat_index(z, i, 0, &s);
        // extract file
        char *buf = new char[s.size]; // allocate buffer
        // extract file to buffer
        zip_file *zf = zip_fopen_index(z, i, 0);
        zip_fread(zf, buf, s.size);
        zip_fclose(zf);
        // write new file
        QFile f(path + s.name);
        f.open(QFile::WriteOnly | QFile::Truncate);
        f.write(buf, s.size);
        f.close();
    }
    zip_close(z);
    // write updater batch script
    emit messageSignal(tr("Creating updater script..."));
    QFile f(bat);
    if(!f.open(QFile::WriteOnly | QFile::Truncate))
    {
        emit messageSignal(tr("Could not open file for writing..."));
        return;
    }
    f.write(
        QString(
            "@echo off\r\n"
            "echo %0\r\n"                                               // status message
            "ping 127.0.0.1 -n 1 -w 1000 > NUL\r\n"                     // wait while baka closes
            "cd \"%1\"\r\n"                                             // go to extracted directory
            "for %%i in (*) do move /Y \"%%i\" ..\r\n"                  // move all files up
            "for /d %%i in (*) do move /Y \"%%i\" ..\r\n"               // move all directories up
            "cd ..\r\n"                                                 // go up
            "rmdir /Q /S \"%2\"\r\n"                                    // remove the tmp directory
            "del /Q \"%3\"\r\n"                                         // delete the zip file
            "start /b \"\" \"%4\"\r\n"                                  // start the new baka-mplayer
            "start /b \"\" cmd /c del \"%~f0\"&exit /b\"\"\r\n").arg(   // remove the script itself and exit
            tr("Updating..."),
            path,
            path,
            file,
            exe).toUtf8());
    f.close();

    QProcess::startDetached(bat, QStringList());
    emit messageSignal(tr("Done. Restarting..."));
    baka->Quit();
}
#endif
