#ifndef IRCDIALOG_H
#define IRCDIALOG_H

#include <QDialog>

namespace Ui {
class IrcDialog;
}

class IrcDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IrcDialog(QWidget *parent = 0);
    ~IrcDialog();

private:
    Ui::IrcDialog *ui;
};

#endif // IRCDIALOG_H
