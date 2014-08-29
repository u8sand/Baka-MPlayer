#include "lightdialog.h"

LightDialog::LightDialog(QWidget *parent) :
    QDialog(parent)
{
    // yes, the order here is strange. it doesn't work any other way.
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(.6);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool |
                   Qt::FramelessWindowHint |
                   Qt::WindowDoesNotAcceptFocus);
    setStyleSheet("background-color: rgb(255,255,255);");
    show();
    hide();
    setWindowFlags(windowFlags() | Qt::WindowTransparentForInput);
}
