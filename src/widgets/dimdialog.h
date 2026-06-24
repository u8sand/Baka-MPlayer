#ifndef DIMWIDGET_H
#define DIMWIDGET_H

#include <QDialog>
#include <QMouseEvent>

class MainWindow;
class DimDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DimDialog(QWidget *parent = 0);

    void show();
    bool close();

protected:
    void mousePressEvent(QMouseEvent *event);

signals:
    void visbilityChanged(bool dim);

private:
    MainWindow *window;
};

#endif // DIMWIDGET_H
