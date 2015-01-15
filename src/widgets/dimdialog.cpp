#include "dimdialog.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>

DimDialog::DimDialog(MainWindow *window, QWidget *parent) :
    QDialog(parent),
    window(window)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setWindowOpacity(.6);
    setStyleSheet("background-color: black;");

    connect(qApp, &QApplication::focusWindowChanged,
            [=](QWindow *focusWindow)
            {
                // note: focusWindow will be 0 if anything is clicked outside of our program which is useful
                // the only other problem is that when dragging by the top handle
                // it will be 0 thus reverting dim lights, this is a side effect
                // which will have to stay for now.
                if(this->isVisible())
                {
                    if(focusWindow == nullptr)
                        this->close();
                    else if(focusWindow == this->windowHandle())
                    {
                        window->raise();
                        window->activateWindow();
                        window->setFocus();
                    }
                }
            });
}

void DimDialog::show()
{
    // set the geometry in the show so that we can fill the desktop (even on another monitor)
    setGeometry(qApp->desktop()->screenGeometry(window->frameGeometry().center()));
    emit visbilityChanged(true);
    QDialog::show();
}

bool DimDialog::close()
{
    emit visbilityChanged(false);
    return QDialog::close();
}

void DimDialog::mousePressEvent(QMouseEvent *event)
{
    close();
    QDialog::mousePressEvent(event);
}
