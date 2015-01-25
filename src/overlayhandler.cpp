#include "overlayhandler.h"

#include "bakaengine.h"
#include "mpvhandler.h"

#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QTimer>

OverlayHandler::OverlayHandler(QObject *parent):
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    overlay_font("Monospace", 14),
    overlay_fm(overlay_font),
    min_overlay(3),
    max_overlay(63),
    overlay_id(min_overlay) // start at 1
{
    overlay_font.setBold(true);
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

void OverlayHandler::showStatusText(const QString &text, int duration)
{
    showText(text, duration, QPoint(20, 20), 1);
}

void OverlayHandler::showInfoText(bool show)
{
    if(show) // show media info
        showText(baka->mpv->getMediaInfo(), 0, QPoint(20, 20), 2);
    else // hide media info
    {
        auto overlay_iter = overlays.find(2);
        if(overlay_iter == overlays.end())
            overlays[2] = nullptr;

        if(overlays[2] != nullptr)
        {
            baka->mpv->RemoveOverlay(2); // remove the overlay
            delete overlays[2]->canvas; // delete the canvas
            delete overlays[2]; // delete the overlay itself
            overlays[2] = nullptr; // set it to nullptr
        }
    }
}

void OverlayHandler::showText(const QString &text, int duration, QPoint pos, int id)
{
    QStringList elements = text.split("\n");
    QPainterPath path(QPoint(0, 0));
    int h = overlay_fm.height();
    QPoint p = QPoint(0, h);
    for(auto element : elements)
    {
        path.addText(p, overlay_font, element);
        p += QPoint(0, h);
    }

    QImage *canvas = new QImage(path.boundingRect().width()+5, p.y(), QImage::Format_ARGB32); // make the canvas the right size
    canvas->fill(0); // fill it with nothing

    QPainter painter(canvas); // prepare to paint
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(overlay_font);
    painter.setPen(QColor(0, 0, 0));
    painter.setBrush(QColor(0xF3F300));
    painter.drawPath(path);

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
        if(duration > 0)
            overlays[id]->timer->start(duration); // restart the timer
    }
    else // overlay doesn't exist yet
    {
        // allocate a new overlay structure
        overlays[id] = new overlay{
            canvas,
            duration > 0 ? new QTimer(this) : nullptr
        };
        if(duration > 0)
        {
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
}
