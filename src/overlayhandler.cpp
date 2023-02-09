#include "overlayhandler.h"

#include "bakaengine.h"
#include "mpvhandler.h"
#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "util.h"
#include "overlay.h"

#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QThread>

#define OVERLAY_INFO 62
#define OVERLAY_STATUS 63
#define OVERLAY_REFRESH_RATE 1000

OverlayHandler::OverlayHandler(QObject *parent):
    QObject(parent),
    baka(static_cast<BakaEngine*>(parent)),
    refresh_timer(this),
    min_overlay(1),
    max_overlay(60),
    overlay_id(min_overlay)
{
    refresh_timer.setSingleShot(true);
    connect(&refresh_timer, &QTimer::timeout, // on timeout
            this, [=] { showInfoText(); });
}

OverlayHandler::~OverlayHandler() {}

void OverlayHandler::showStatusText(const QString &text, int duration)
{
    if(text != QString())
        showText(text,
                 QFont(Util::MonospaceFont(),
                       14, QFont::Bold), QColor(0xFFFFFF),
                 QPoint(20, 20), duration, OVERLAY_STATUS);
    else if(duration == 0)
        remove(OVERLAY_STATUS);
}

void OverlayHandler::showInfoText(bool show)
{
    if(show) // show media info
    {
        refresh_timer.start(OVERLAY_REFRESH_RATE);
        baka->mpv->getMediaInfo([=](QString mediaInfo) {
            showText(mediaInfo,
                    QFont(Util::MonospaceFont(),
                          14, QFont::Bold), QColor(0xFFFF00),
                    QPoint(20, 20), 0, OVERLAY_INFO);
        });
    }
    else // hide media info
    {
        refresh_timer.stop();
        remove(OVERLAY_INFO);
    }
}

void OverlayHandler::showText(const QString &text, QFont font, QColor color, QPoint pos, int duration, int id)
{
    overlay_mutex.lock();
    // increase next overlay_id
    if(id == -1) // auto id
    {
        id = overlay_id;
        overlay_id = std::max(min_overlay, (overlay_id + 1) % max_overlay);
    }

    Overlay &overlay = overlays[id];
    if (overlay.label == nullptr)
    {
        // this label is owned by baka->window->ui->mpvFrame which will try to
        //  delete it when it is destroyed
        overlay.label = new QLabel(baka->window->ui->mpvFrame);
        overlay.label->setStyleSheet("background-color:rgba(0,0,0,0);background-image:url();");
    }

    QFontMetrics fm(font);
    QStringList lines = text.split('\n');
    // the 1.3 was pretty much determined through trial and error; this formula isn't perfect
    // apparently, QFontMetrics doesn't work that well
    const float fm_correction = 1.3;
    int w = 0,
        h = fm.height()*lines.length();
    for(auto &line : lines)
        w = std::max(fm.horizontalAdvance(line), w);
    float xF = float(baka->window->ui->mpvFrame->width()-2*pos.x()) / (fm_correction*w);
    float yF = float(baka->window->ui->mpvFrame->height()-2*pos.y()) / h;
    font.setPointSizeF(std::min(font.pointSizeF()*std::min(xF, yF), font.pointSizeF()));

    fm = QFontMetrics(font);
    h = fm.height();
    w = 0;
    QPainterPath path(QPoint(0, 0));
    QPoint p = QPoint(0, h);
    for(auto &line : lines)
    {
        path.addText(p, font, line);
        w = std::max(int(fm_correction*path.currentPosition().x()), w);
        p += QPoint(0, h);
    }

    QImage canvas(w, p.y(), QImage::Format_ARGB32); // make the canvas the right size
    canvas.fill(QColor(0,0,0,0)); // fill it with nothing

    QPainter painter(&canvas); // prepare to paint
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Overlay);
    painter.setFont(font);
    painter.setPen(QColor(0, 0, 0));
    painter.setBrush(color);
    painter.drawPath(path);

    // add over mpv as label
    overlay.label->setGeometry(pos.x(),
                       pos.y(),
                       canvas.width(),
                       canvas.height());
    overlay.label->setPixmap(QPixmap::fromImage(canvas));
    overlay.label->show();

    if(duration > 0)
    {
        overlay.timer->start(duration);
        connect(overlay.timer.get(), &QTimer::timeout, // on timeout
                overlay.timer.get(), [this, id] { remove(id); });
    }

    // add as mpv overlay
    baka->mpv->AddOverlay(
        id,
        pos.x(), pos.y(),
        "&"+QString::number(quintptr(canvas.bits())),
        0, canvas.width(), canvas.height());

    overlay_mutex.unlock();
}

void OverlayHandler::remove(int id)
{
    overlay_mutex.lock();
    if (overlays.contains(id)) {
      baka->mpv->RemoveOverlay(id);
      {
        Overlay &overlay = overlays[id];
        overlay.timer->stop();
        overlay.label->hide();
        overlay.label->deleteLater();
      }
      overlays.remove(id);
    }
    overlay_mutex.unlock();
}
