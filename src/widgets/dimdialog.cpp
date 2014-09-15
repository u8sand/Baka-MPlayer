#include "dimdialog.h"

DimDialog::DimDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(.6);
    setWindowFlags(Qt::Tool |
                   Qt::FramelessWindowHint);
    setStyleSheet("background-color: rgb(255,255,255);");
}

void DimDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit clicked();
        event->accept();
    }
    QDialog::mousePressEvent(event);
}
