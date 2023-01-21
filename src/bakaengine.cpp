#include "bakaengine.h"

#include <QMessageBox>
#include <QDir>

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mpvhandler.h"
#include "gesturehandler.h"
#include "overlayhandler.h"
#include "updatemanager.h"
#include "widgets/dimdialog.h"
#include "util.h"

BakaEngine::BakaEngine(QObject *parent):
    QObject(parent),
    window(static_cast<MainWindow*>(parent)),
    mpv(new MpvHandler(window->ui->mpvFrame, this)),
    settings(new Settings(Util::SettingsLocation(), this)),
    gesture(new GestureHandler(this)),
    overlay(new OverlayHandler(this)),
    update(new UpdateManager(this)),
    // note: trayIcon does not work in my environment--known qt bug
    // see: https://bugreports.qt-project.org/browse/QTBUG-34364
    sysTrayIcon(new QSystemTrayIcon(window->windowIcon(), this)),
    // todo: tray menu/tooltip
    translator(nullptr),
    qtTranslator(nullptr)
{
    if(Util::DimLightsSupported())
        dimDialog = new DimDialog(window);
    else
    {
        dimDialog = nullptr;
        window->ui->action_Dim_Lights->setEnabled(false);
    }

    connect(mpv, &MpvHandler::messageSignal,
            mpv, [=](QString msg)
            {
                Print(msg, "mpv");
            });
    connect(update, &UpdateManager::messageSignal,
            update, [=](QString msg)
            {
                Print(msg, "update");
            });
}

void BakaEngine::LoadSettings()
{
    settings->Load();
    Load2_0_3();
}

void BakaEngine::Command(QString command)
{
    if(command == QString())
        return;
    QStringList args = command.split(" ");
    if(!args.empty())
    {
        if(args.front() == "baka") // implicitly understood
            args.pop_front();

        if(!args.empty())
        {
            auto iter = BakaCommandMap.find(args.front());
            if(iter != BakaCommandMap.end())
            {
                args.pop_front();
                (this->*(iter->first))(args); // execute command
            }
            else
                InvalidCommand(args.join(' '));
        }
        else
            RequiresParameters("baka");
    }
    else
        InvalidCommand(args.join(' '));
}

void BakaEngine::Print(QString what, QString who)
{
    QString out = QString("[%0]: %1").arg(who, what);
    (qStdout() << out).flush();
    window->ui->outputTextEdit->moveCursor(QTextCursor::End);
    window->ui->outputTextEdit->insertPlainText(out);
}

void BakaEngine::PrintLn(QString what, QString who)
{
    Print(what+"\n", who);
}

void BakaEngine::InvalidCommand(QString command)
{
    PrintLn(tr("invalid command '%0'").arg(command));
}

void BakaEngine::InvalidParameter(QString parameter)
{
    PrintLn(tr("invalid parameter '%0'").arg(parameter));
}

void BakaEngine::RequiresParameters(QString what)
{
    PrintLn(tr("'%0' requires parameters").arg(what));
}
