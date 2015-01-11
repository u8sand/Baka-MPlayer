#include "dimdialog.h"

DimDialog::DimDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowState(Qt::WindowFullScreen);
    setWindowOpacity(.6);
    setStyleSheet("background-color: black;");
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
