#include "ui/mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(QString(BAKA_MPLAYER_LANG) != "baka-mplayer-en")
    {
      QTranslator translator;
      translator.load(BAKA_MPLAYER_LANG);
      a.installTranslator(&translator);
    }

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
