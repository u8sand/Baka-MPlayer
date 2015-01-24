#include "overlayhandler.h"

#include "bakaengine.h"
#include "mpvhandler.h"

#include <QPainter>
#include <QColor>
#include <QTimer>

OverlayHandler::OverlayHandler(QObject *parent):
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    overlay_font("Monospace", 11),
    overlay_fm(overlay_font),
    min_overlay(2),
    max_overlay(63),
    overlay_id(min_overlay) // start at 1
{
}

OverlayHandler::~OverlayHandler()
{
    for(auto overlay_iter = overlays.begin(); overlay_iter != overlays.end(); ++overlay_iter)
    {
        if(*overlay_iter != nullptr)
        {
            delete (*overlay_iter)->timer;
            delete (*overlay_iter)->canvas;
            delete (*overlay_iter);
        }
    }
}

void OverlayHandler::showText(QString text, int duration)
{
    showText(text, duration, QPoint(20, 20), 1);
}

void OverlayHandler::showText(QString text, int duration, QPoint pos, int id)
{
    // draw to new canvas
    QImage *canvas = new QImage(overlay_fm.size(0, text), QImage::Format_ARGB32); // make the canvas the right size
    canvas->fill(0); // fill it with nothing
    QPainter painter(canvas); // prepare to paint
    painter.setFont(overlay_font); // set the font
    painter.setPen(QColor(0xe4cf0b)); // set the color
    painter.drawText(canvas->rect(), text); // draw
    // add as mpv overlay
    baka->mpv->AddOverlay(
        id == -1 ? overlay_id : id,
        pos.x(), pos.y(),
        "&"+QString::number(quintptr(canvas->bits())),
        0, canvas->width(), canvas->height());
    // increase next overlay_id
    if(id == -1) // auto id
    {
        id = overlay_id;
        if(overlay_id+1 > max_overlay)
            overlay_id = min_overlay;
        else
            ++overlay_id;
    }
    // handle overlay
    // create it if it's not found
    auto overlay_iter = overlays.find(id);
    if(overlay_iter == overlays.end())
        overlays[id] = nullptr;
    if(overlays[id] != nullptr) // this overlay already exists
    {
        delete overlays[id]->canvas; // delete the old canvas
        overlays[id]->canvas = canvas; // update the canvas
        overlays[id]->timer->start(duration); // restart the timer
    }
    else // overlay doesn't exist yet
    {
        overlays[id] = new overlay{canvas, new QTimer(this)}; // allocate a new overlay and timer
        overlays[id]->timer->start(duration); // start the timer
        connect(overlays[id]->timer, &QTimer::timeout, // on timeout
                [=]
                {
                    baka->mpv->RemoveOverlay(id); // remove the overlay
                    delete overlays[id]->canvas; // delete the canvas
                    delete overlays[id]->timer; // delete the timer
                    delete overlays[id]; // delete the overlay itself
                    overlays[id] = nullptr; // set it to nullptr
                });
    }
}
