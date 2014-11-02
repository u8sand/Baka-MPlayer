#include "infodialog.h"
#include "ui_infodialog.h"

InfoDialog::InfoDialog(const Mpv::FileInfo &_fileInfo, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    fileInfo(_fileInfo)
{
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));

    QList<QPair<QString, QString>> items = {
        {"media-title", fileInfo.media_title},
        {"length", QString::number(fileInfo.length)},
        {"width", QString::number(fileInfo.video_params.width)},
        {"height", QString::number(fileInfo.video_params.height)},
        {"dwidth", QString::number(fileInfo.video_params.dwidth)},
        {"dheight", QString::number(fileInfo.video_params.dheight)}
    };
    ui->infoWidget->setRowCount(items.length()+1+fileInfo.tracks.length()+1+fileInfo.chapters.length());
    int r = 0;
    for(auto &iter : items)
    {
        ui->infoWidget->setItem(r, 0, new QTableWidgetItem(iter.first));
        ui->infoWidget->setItem(r, 1, new QTableWidgetItem(iter.second));
        r++;
    }
    ui->infoWidget->setItem(r++, 0, new QTableWidgetItem("Track List"));
    for(auto &track : fileInfo.tracks)
    {
        ui->infoWidget->setItem(r, 0, new QTableWidgetItem(QString::number(track.id)));
        ui->infoWidget->setItem(r, 1, new QTableWidgetItem(track.title+"["+track.type+":"+track.lang+"] "+track.external_filename));
        r++;
    }
    ui->infoWidget->setItem(r++, 0, new QTableWidgetItem("Chapter List"));
    for(auto &chapter : fileInfo.chapters)
    {
        ui->infoWidget->setItem(r, 0, new QTableWidgetItem(chapter.title));
        ui->infoWidget->setItem(r, 1, new QTableWidgetItem(QString::number(chapter.time)));
        r++;
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::info(const Mpv::FileInfo &fileInfo, QWidget *parent)
{
    InfoDialog dialog(fileInfo, parent);
    dialog.exec();
}
