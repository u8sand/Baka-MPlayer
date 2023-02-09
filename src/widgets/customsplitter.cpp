#include "customsplitter.h"

CustomSplitter::CustomSplitter(QWidget *parent) :
    QSplitter(parent),
    normalPos(0)
{
    this->setHandleWidth(0);
    connect(this, &CustomSplitter::splitterMoved,
            [=](int pos, int index)
            {
                if(index == 1)
                    emit positionChanged(max()-pos);
            });
}

int CustomSplitter::position() const
{
    return sizes().at(1);
}

int CustomSplitter::normalPosition() const
{
    return normalPos;
}

int CustomSplitter::max() const
{
    const QList<int> s = sizes();
    return s[0]+s[1];
}

void CustomSplitter::setPosition(int pos)
{
    setSizes({max()-pos, pos});
    emit positionChanged(pos);
}

void CustomSplitter::setNormalPosition(int pos)
{
    normalPos = pos;
}
