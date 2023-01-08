#include "gesturehandler.h"

#include <QApplication>
#include <QDesktopWidget>

#include "bakaengine.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"
#include "util.h"

GestureHandler::GestureHandler(QObject *parent):
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    elapsedTimer(nullptr)
{
    timer_threshold = 10; // 10ms works quite well
    gesture_threshold = 15; // 15 pixels is probably fine for anything
}

GestureHandler::~GestureHandler()
{
    if(elapsedTimer)
    {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
}

bool GestureHandler::Begin(int gesture_type, QPoint mousePos, QPoint windowPos)
{
    if(!elapsedTimer)
    {
        elapsedTimer = new QElapsedTimer();
        elapsedTimer->start();
        this->gesture_type = gesture_type;
        this->gesture_state = NONE;
        // calculate pixel ratios based on physical dpi
        hRatio = qApp->desktop()->physicalDpiX() / 800.0;
        vRatio = qApp->desktop()->physicalDpiY() / 450.0;
    }
    else
        return false;

    if(gesture_type == MOVE)
        start.windowPos = windowPos;
    else // if(gesture_type == HSEEK_VVOLUME)
    {
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
        start.time = baka->mpv->getTime();
        start.volume = baka->mpv->getVolume();
    }
    start.mousePos = mousePos;
    return true;
}

bool GestureHandler::Process(QPoint mousePos)
{
    if(elapsedTimer && elapsedTimer->elapsed() > timer_threshold) // 10ms seems pretty good for all purposes
    {
        QPoint delta = mousePos - start.mousePos;

        if(gesture_type == MOVE)
            baka->window->move(start.windowPos + delta);
        else
        {
            switch(gesture_state)
            {
            case NONE:
                if(abs(delta.x()) >= abs(delta.y()) + gesture_threshold)
                    gesture_state = SEEKING;
                else if(abs(delta.y()) >= abs(delta.x()) + gesture_threshold)
                    gesture_state = ADJUSTING_VOLUME;
                break;
            case SEEKING:
            {
                int relative = delta.x() * hRatio;
                baka->mpv->Seek(start.time + relative, false, true);
                break;
            }
            case ADJUSTING_VOLUME:
                baka->mpv->Volume(start.volume - delta.y() * vRatio, true);
                break;
            }
        }

        elapsedTimer->restart();
        return true;
    }
    else
        return false;
}

bool GestureHandler::End()
{
    if(elapsedTimer)
    {
        delete elapsedTimer;
        elapsedTimer = nullptr;
        QApplication::restoreOverrideCursor();
    }
    else
        return false;
    return gesture_state != NONE;
}
