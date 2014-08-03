#include "windows/mainwindow.h"

#include <QApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QtPlugin>

Q_IMPORT_PLUGIN(qsvg)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    MainWindow w(&settings);
    w.show();

    return a.exec();
}
