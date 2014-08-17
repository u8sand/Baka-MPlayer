#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InputDialog(int max, QDialog *parent = 0);
    ~InputDialog();

    static int getIndex(int max, QDialog *parent = 0);

private slots:
    void validate(QString input);

private:
    Ui::InputDialog *ui;
    int max;
};

#endif // INPUTDIALOG_H
