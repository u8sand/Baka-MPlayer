#include "windows/mainwindow.h"
#include "settingsmanager.h"
#include <QApplication>

#ifdef Q_OS_WIN
#include <QtPlugin>

Q_IMPORT_PLUGIN(qsvg)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SettingsManager settings(SETTINGS_FILE);

    MainWindow w(&settings);
    w.show();

    return a.exec();
}
