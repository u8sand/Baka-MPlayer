#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <QDialog>

namespace Ui {
class LocationDialog;
}

class LocationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocationDialog(QString path, QWidget *parent = 0);
    ~LocationDialog();

    static QString getUrl(QString path, QWidget *parent = 0);

private slots:
    void validate(QString input);

private:
    Ui::LocationDialog *ui;
};

#endif // LOCATIONDIALOG_H
