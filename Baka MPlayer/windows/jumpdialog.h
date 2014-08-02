#ifndef JUMPDIALOG_H
#define JUMPDIALOG_H

#include <QDialog>

namespace Ui {
class JumpDialog;
}

class JumpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JumpDialog(QWidget *parent = 0);
    ~JumpDialog();

    static int getTime(QWidget *parent = 0);

private:
    Ui::JumpDialog *ui;
};

#endif // JUMPDIALOG_H
