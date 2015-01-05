#include "gesturehandler.h"

#include <QApplication>

#include "bakaengine.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"
#include "util.h"

GestureHandler::GestureHandler(QObject *parent):
    QObject(parent),
    elapsedTimer(nullptr)
{
    baka = static_cast<BakaEngine*>(parent);
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
    if(elapsedTimer && elapsedTimer->elapsed() > 10) // 10ms seems pretty good for all purposes
    {
        QPoint delta = mousePos - start.mousePos;

        if(gesture_type == MOVE)
            baka->window->move(start.windowPos + delta);
        else
        {
            switch(gesture_state)
            {
            case NONE:
                if(abs(delta.x()) >= abs(delta.y())+25) // 25 px seems to be a pretty good threshold
                    gesture_state = SEEKING;
                else if(abs(delta.y()) >= abs(delta.x())+25)
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
    return true;
}
