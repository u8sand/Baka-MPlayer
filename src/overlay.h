#ifndef OVERLAY_H
#define OVERLAY_H

#include <QLabel>
#include <QTimer>
#include <QSharedPointer>

struct Overlay {
public:
    Overlay();
    Overlay(const Overlay &overlay);
    ~Overlay();

    // we use a ptr because this label needs to have a parent of another object
    //  and that object will try to delete it
    QLabel *label;

    // sharing this pointer seems simpler for dealing with the copy constructor
    QSharedPointer<QTimer> timer;
};

#endif // OVERLAY_H
