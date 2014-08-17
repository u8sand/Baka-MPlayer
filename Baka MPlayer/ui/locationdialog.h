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
    explicit LocationDialog(QWidget *parent = 0);
    ~LocationDialog();

    static QString getUrl(QWidget *parent = 0);

private slots:
    void validate(QString input);

    void on_pasteButton_clicked();
    void on_copyButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::LocationDialog *ui;
};

#endif // LOCATIONDIALOG_H
