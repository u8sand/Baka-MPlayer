#include "customsplitter.h"

CustomSplitter::CustomSplitter(QWidget *parent) :
    QSplitter(parent),
    size{1, 1},
    collapsed{false, false}
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(UpdateSize(int,int)));
    connect(this, SIGNAL(sizesChanged()),
            this, SLOT(UpdateSize()));
}

bool CustomSplitter::isCollapsed(int which)
{
    if((which == 0 && collapsed[0]) ||
       (which == 1 && collapsed[1]))
        return true;
    return false;
}

void CustomSplitter::SetCollapse(bool collapse, int which)
{
    if(collapse)
    {
        if(which == 0)
            setSizes({0, size[0]+size[1]});
        else if(which == 1)
            setSizes({size[0]+size[1], 0});
    }
    else
    {
        if(which == 0)
        {
            if(collapsed[1])
                setSizes({size[0]+size[1], 0});
            else
                setSizes({size[0], size[1]});
        }
        else if(which == 1)
        {
            if(collapsed[0])
                setSizes({0, size[0]+size[1]});
            else
                setSizes({size[0], size[1]});
        }
        else
            setSizes({size[0], size[1]});
    }
}

void CustomSplitter::setSizes(const QList<int> &list)
{
    QSplitter::setSizes(list);
    emit sizesChanged();
}

void CustomSplitter::UpdateSize()
{
    QList<int> s = sizes();
    if(s[0] != 0 && s[1] != 0)
    {
        size[0] = s[0];
        size[1] = s[1];
        collapsed[0] = false;
        collapsed[1] = false;
    }
    else if(s[0] == 0)
    {
        collapsed[0] = true;
        collapsed[1] = false;
    }
    else if(s[1] == 0)
    {
        collapsed[0] = false;
        collapsed[1] = true;
    }

}
