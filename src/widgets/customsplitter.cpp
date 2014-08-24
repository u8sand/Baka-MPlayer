#include "customsplitter.h"

CustomSplitter::CustomSplitter(QWidget *parent) :
    QSplitter(parent),
    normalPos(0)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(convertSignal(int,int)));
}

int CustomSplitter::position() const
{
    return sizes()[0];
}

int CustomSplitter::normalPosition() const
{
    return normalPos;
}

int CustomSplitter::max() const
{
    QList<int> s = sizes();
    return s[0]+s[1];
}

void CustomSplitter::setPosition(int pos)
{
    QList<int> s = sizes();
    setSizes({pos, s[0]+s[1]-pos});
    emit positionChanged(pos);
}

void CustomSplitter::setNormalPosition(int pos)
{
    normalPos = pos;
}

void CustomSplitter::convertSignal(int pos, int)
{
    emit positionChanged(pos);
}
