#include "infodialog.h"
#include "ui_infodialog.h"
#include <QFileInfo>
#include <QDateTime>

#include "util.h"

InfoDialog::InfoDialog(const QString &fileName, const Mpv::FileInfo &_fileInfo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    fileInfo(_fileInfo)
{
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    QTableWidgetItem *item;

    QFileInfo fi(fileName);
    QList<QPair<QString, QString>> items = {
        {tr("File name"), fi.fileName()},
        {tr("Media Title"), fileInfo.media_title},
        {tr("File size"), Util::HumanSize(fi.size())},
        {tr("Date created"), fi.created().toString()},
        {tr("Media length"), Util::FormatTime(fileInfo.length, fileInfo.length)},
    };

    if(fileInfo.video_params.codec != QString())
    {
        QList<QPair<QString, QString>> video_items = {
            {tr("[Video]"), QString()},
            {tr("Video codec"), fileInfo.video_params.codec},
            {tr("Video format"), fileInfo.video_params.format},
            {tr("Video bitrate"), fileInfo.video_params.bitrate},
            {tr("Video dimensions"), QString::number(fileInfo.video_params.width)+" x "+QString::number(fileInfo.video_params.height)}
        };
        items.append(video_items);
    }

    if(fileInfo.audio_params.codec != QString())
    {
        QList<QPair<QString, QString>> audio_items = {
            {tr("[Audio]"), QString()},
            {tr("Audio codec"), fileInfo.audio_params.codec},
            {tr("Audio format"), fileInfo.audio_params.format},
            {tr("Audio bitrate"), fileInfo.audio_params.bitrate},
            {tr("Audio samplerate"), fileInfo.audio_params.samplerate},
            {tr("Audio channels"), fileInfo.audio_params.channels}
        };
        items.append(audio_items);
    }

    ui->infoWidget->setRowCount(items.length());
    int r = 0;
    for(auto &iter : items)
    {
        if(iter.second == QString())
        {
            item = new QTableWidgetItem(iter.first);
            item->setTextAlignment(Qt::AlignCenter);
            ui->infoWidget->setItem(r, 0, item);
        }
        else
        {
            ui->infoWidget->setItem(r, 0, new QTableWidgetItem(iter.first));
            ui->infoWidget->setItem(r, 1, new QTableWidgetItem(iter.second));
        }
        r++;
    }
    if(fileInfo.tracks.length() > 0)
    {
        ui->infoWidget->setRowCount(ui->infoWidget->rowCount()+1+fileInfo.tracks.length());
        item = new QTableWidgetItem(tr("[Track List]"));
        item->setTextAlignment(Qt::AlignCenter);
        ui->infoWidget->setItem(r++, 0, item);
        for(auto &track : fileInfo.tracks)
        {
            ui->infoWidget->setItem(r, 0, new QTableWidgetItem(QString::number(track.id)));
            ui->infoWidget->setItem(r, 1, new QTableWidgetItem(track.title+"["+track.type+":"+track.lang+"] "+track.external_filename));
            r++;
        }
    }
    if(fileInfo.chapters.length() > 0)
    {
        ui->infoWidget->setRowCount(ui->infoWidget->rowCount()+1+fileInfo.chapters.length());

        item = new QTableWidgetItem(tr("[Chapter List]"));
        item->setTextAlignment(Qt::AlignCenter);
        ui->infoWidget->setItem(r++, 0, item);
        for(auto &chapter : fileInfo.chapters)
        {
            ui->infoWidget->setItem(r, 0, new QTableWidgetItem(chapter.title));
            ui->infoWidget->setItem(r, 1, new QTableWidgetItem(Util::FormatTime(chapter.time, fileInfo.length)));
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
