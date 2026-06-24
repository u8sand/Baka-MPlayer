#ifndef KEYDIALOG_H
#define KEYDIALOG_H

#include <QDialog>
#include <QScopedPointer>

namespace Ui {
class KeyDialog;
}

class KeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyDialog(QWidget *parent = 0);
    ~KeyDialog();

    QPair<QString, QPair<QString, QString>> SelectKey(bool add, QPair<QString, QPair<QString, QString>> init = (QPair<QString, QPair<QString, QString>>()));

protected:
    void SetButtons();

private:
    QScopedPointer<Ui::KeyDialog> ui;
    bool add;
};

#endif // KEYDIALOG_H
