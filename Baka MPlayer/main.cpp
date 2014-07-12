#include "windows/mainwindow.h"
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

    return a.exec();
}
