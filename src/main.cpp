#include "ui/mainwindow.h"

#include <QApplication>

#ifdef Q_OS_WIN
#include <QtPlugin>

Q_IMPORT_PLUGIN(qsvg)
#endif

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

    return a.exec();
}
