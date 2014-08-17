#include "lightdialog.h"

LightDialog::LightDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(.4);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool |
                   Qt::FramelessWindowHint |
                   Qt::WindowDoesNotAcceptFocus);
    setStyleSheet("background-color: rgb(100,100,100);");
}
