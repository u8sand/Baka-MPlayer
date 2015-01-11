#include "customlineedit.h"

CustomLineEdit::CustomLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
}

void CustomLineEdit::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return) // do we need to check Qt::Key_Enter too?
    {
        emit submitted(text());
        event->accept();
    }
    QLineEdit::keyPressEvent(event);
}
