#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

#include <mpv/client.h>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = 0);
    ~InfoDialog();

    static void info(const mpv_node_list metadata, QWidget *parent = 0);
private:
    Ui::InfoDialog *ui;
};

#endif // INFODIALOG_H
