#include "infodialog.h"
#include "ui_infodialog.h"

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::info(/*const mpv_node_list metadata, */QWidget *parent)
{
    InfoDialog dialog(parent);
//    for(int i = 0; i < metadata.num; i++)
//        dialog.ui->infoList->addItem(QString(metadata.keys[i])+": "+QString(metadata.values[i].u.string));
    dialog.exec();
}
