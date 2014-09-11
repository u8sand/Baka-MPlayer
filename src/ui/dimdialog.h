#ifndef DIMWIDGET_H
#define DIMWIDGET_H

#include <QDialog>
#include <QMouseEvent>

class DimDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DimDialog(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked();
};

#endif // DIMWIDGET_H
