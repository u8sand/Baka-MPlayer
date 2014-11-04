#include "infodialog.h"
#include "ui_infodialog.h"
#include <QFileInfo>
#include <QDateTime>

#include "util.h"

using namespace BakaUtil;

InfoDialog::InfoDialog(const QString &fileName, const Mpv::FileInfo &_fileInfo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    fileInfo(_fileInfo)
{
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    QFileInfo fi(fileName);
    QList<QPair<QString, QString>> items = {
        {"File name", fi.fileName()},
        {"Media Title", fileInfo.media_title},
        {"File size", HumanSize(fi.size())},
        {"Date created", fi.created().toString()},
        {"Media length", FormatTime(fileInfo.length, fileInfo.length)},
        {"[Video]",""},
        {"Video codec", fileInfo.video_params.codec},
        {"Video format", fileInfo.video_params.format},
        {"Video bitrate", fileInfo.video_params.bitrate},
        {"Video dimensions", QString::number(fileInfo.video_params.width)+" x "+QString::number(fileInfo.video_params.height)},
        {"[Audio]",""},
        {"Audio codec", fileInfo.audio_params.codec},
        {"Audio format", fileInfo.audio_params.format},
        {"Audio bitrate", fileInfo.audio_params.bitrate},
        {"Audio samplerate", fileInfo.audio_params.samplerate},
        {"Audio channels", fileInfo.audio_params.channels}
    };
    ui->infoWidget->setRowCount(items.length());
    int r = 0;
    for(auto &iter : items)
    {
        ui->infoWidget->setItem(r, 0, new QTableWidgetItem(iter.first));
        ui->infoWidget->setItem(r, 1, new QTableWidgetItem(iter.second));
        r++;
    }
    if(fileInfo.tracks.length() > 0)
    {
        ui->infoWidget->setRowCount(ui->infoWidget->rowCount()+1+fileInfo.tracks.length());
        ui->infoWidget->setItem(r++, 0, new QTableWidgetItem("[Track List]"));
        for(auto &track : fileInfo.tracks)
        {
            ui->infoWidget->setItem(r, 0, new QTableWidgetItem(QString::number(track.id)));
            ui->infoWidget->setItem(r, 1, new QTableWidgetItem(track.title+"["+track.type+":"+track.lang+"] "+track.external_filename));
            r++;
        }
    }
    if(fileInfo.chapters.length() > 0)
    {
        ui->infoWidget->setRowCount(ui->infoWidget->rowCount()+1+fileInfo.tracks.length());
        ui->infoWidget->setItem(r++, 0, new QTableWidgetItem("[Chapter List]"));
        for(auto &chapter : fileInfo.chapters)
        {
            ui->infoWidget->setItem(r, 0, new QTableWidgetItem(chapter.title));
            ui->infoWidget->setItem(r, 1, new QTableWidgetItem(FormatTime(chapter.time, fileInfo.length)));
            r++;
        }
    }
    ui->infoWidget->resizeColumnsToContents();
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::info(const QString &fileName, const Mpv::FileInfo &fileInfo, QWidget *parent)
{
    InfoDialog dialog(fileName, fileInfo, parent);
    dialog.exec();
}
