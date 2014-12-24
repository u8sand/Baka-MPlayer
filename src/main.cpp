#include "ui/mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef BAKA_MPLAYER_LANG_PATH
    QTranslator translator;
#ifdef BAKA_MPLAYER_LANG
    translator.load(QString("%0baka-mplayer_%1").arg(BAKA_MPLAYER_LANG_PATH, BAKA_MPLAYER_LANG));
#else
    translator.load(QString("%0baka-mplayer_%1").arg(BAKA_MPLAYER_LANG_PATH, QLocale::system().name().left(2)));
#endif
    a.installTranslator(&translator);
#endif

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
