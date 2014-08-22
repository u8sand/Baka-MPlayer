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

    // setup dialog: todo, make this neater and more informative
    ui->infoList->addItem("media-title = "+fileInfo.media_title);
    ui->infoList->addItem("length = "+QString::number(fileInfo.length));
    ui->infoList->addItem("width = "+QString::number(fileInfo.video_params.width));
    ui->infoList->addItem("height = "+QString::number(fileInfo.video_params.height));
    ui->infoList->addItem("dwidth = "+QString::number(fileInfo.video_params.dwidth));
    ui->infoList->addItem("dheight = "+QString::number(fileInfo.video_params.dheight));
    ui->infoList->addItem("Tracks:");
    for(auto track : fileInfo.tracks)
        ui->infoList->addItem(QString::number(track.id)+": "+track.title+"["+track.type+":"+track.lang+"] "+track.external_filename);
    ui->infoList->addItem("Chapters:");
    for(auto chapter : fileInfo.chapters)
        ui->infoList->addItem(chapter.title+": "+QString::number(chapter.time));
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
