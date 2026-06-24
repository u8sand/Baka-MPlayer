#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QString>
#include <QScopedPointer>

#include <functional>

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InputDialog(QString prompt, QString title, const std::function<bool (QString)> &validation, QWidget *parent = 0);
    ~InputDialog();

    static QString getInput(QString prompt, QString title, const std::function<bool (QString)> &validation, QWidget *parent = 0);

private slots:
    void validate(QString input);

private:
    QScopedPointer<Ui::InputDialog> ui;
    const std::function<bool (QString)> &validation;

    // Q_DISABLE_COPY(Ui::InputDialog)
};

#endif // INPUTDIALOG_H
