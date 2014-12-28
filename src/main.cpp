#include "ui/mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    // parse command line
    QStringList args = QApplication::arguments();
    QStringList::iterator arg = args.begin();
    if(++arg != args.end())
        w.Load(*arg);
    else
        w.Load();

    return a.exec();
}
