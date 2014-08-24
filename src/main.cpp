#include "ui/mainwindow.h"

#include <QApplication>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QtPlugin>

Q_IMPORT_PLUGIN(qsvg)
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // todo: figure out how we want to deal with configuration on windows
#if Q_OS_WIN
    // saves to $(application directory)\${SETTINGS_FILE}.ini
    QSettings settings(QApplication::applicationDirPath()+"\\"+SETTINGS_FILE, QSettings::IniFormat);
#else
    // saves to  ~/.config/${SETTINGS_FILE}.ini on linux
    // saves to %APP_DATA%/${SETTINGS_FILE}.ini on windows
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTINGS_FILE);
#endif

    MainWindow w(&settings);
    w.show();

    return a.exec();
}
