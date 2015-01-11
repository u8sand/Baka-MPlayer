#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void submitted(QString);
};

#endif // CUSTOMLINEEDIT_H
