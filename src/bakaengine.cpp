#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mpvhandler.h"
#include "gesturehandler.h"
#include "ui/updatedialog.h"
#include "widgets/dimdialog.h"
#include "util.h"

#include <QMessageBox>
#include <QDir>


BakaEngine::BakaEngine(QObject *parent):
    QObject(parent),
    window(static_cast<MainWindow*>(parent)),
    mpv(new MpvHandler(window->ui->mpvFrame->winId(), this)),
    settings(Util::InitializeSettings(this)),
    gesture(new GestureHandler(this)),
    updateDialog(new UpdateDialog(window)),
    // note: trayIcon does not work in my environment--known qt bug
    // see: https://bugreports.qt-project.org/browse/QTBUG-34364
    sysTrayIcon(new QSystemTrayIcon(window->windowIcon(), this)),
    // todo: tray menu/tooltip
    translator(nullptr),
    qtTranslator(nullptr)
{
    if(Util::DimLightsSupported())
        dimDialog = new DimDialog(window, nullptr);
    else
    {
        dimDialog = nullptr;
        window->ui->action_Dim_Lights->setEnabled(false);
    }

    connect(mpv, SIGNAL(messageSignal(QString)),
            this, SLOT(MpvPrint(QString)));
}

BakaEngine::~BakaEngine()
{
    if(translator != nullptr)
        delete translator;
    if(qtTranslator != nullptr)
        delete qtTranslator;
    if(dimDialog != nullptr)
        delete dimDialog;
    delete updateDialog;
    delete gesture;
    delete mpv;
    delete settings;
}

void BakaEngine::LoadSettings()
{
    if(settings == nullptr)
        return;

    settings->Load();

    QString version;
    if(settings->isEmpty()) // empty settings
    {
        version = "2.0.2"; // current version

        // populate initially
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        settings->beginGroup("mpv");
        settings->setValue("af", "scaletempo");
        settings->setValue("vo", "vdpau,opengl-hq");
        settings->setValue("hwdec", "auto");
        settings->endGroup();
#endif
    }
    else
    {
        settings->beginGroup("baka-mplayer");
        version = settings->value("version", "1.9.9"); // defaults to the first version without version info in settings
        settings->endGroup();
    }

    if(version == "2.0.2") Load2_0_2();
    else if(version == "2.0.1") { Load2_0_1(); settings->clear(); SaveSettings(); }
    else if(version == "2.0.0") { Load2_0_0(); settings->clear(); SaveSettings(); }
    else if(version == "1.9.9") { Load1_9_9(); settings->clear(); SaveSettings(); }
    else
    {
        Load2_0_2();
        window->ui->action_Preferences->setEnabled(false);
        QMessageBox::information(window, tr("Settings version not recognized"), tr("The settings file was made by a newer version of baka-mplayer; please upgrade this version or seek assistance from the developers.\nSome features may not work and changed settings will not be saved."));
        delete settings;
        settings = nullptr;
    }
}

void BakaEngine::SaveSettings()
{
    if(settings == nullptr)
        return;

    settings->beginGroup("baka-mplayer");
    settings->setValue("onTop", window->onTop);
    settings->setValueInt("autoFit", window->autoFit);
    settings->setValueBool("trayIcon", sysTrayIcon->isVisible());
    settings->setValueBool("hidePopup", window->hidePopup);
    settings->setValueBool("remaining", window->remaining);
    settings->setValueInt("splitter", (window->ui->splitter->position() == 0 ||
                                    window->ui->splitter->position() == window->ui->splitter->max()) ?
                                    window->ui->splitter->normalPosition() :
                                    window->ui->splitter->position());
    settings->setValueBool("showAll", !window->ui->hideFilesButton->isChecked());
    settings->setValueBool("screenshotDialog", window->screenshotDialog);
    settings->setValueBool("debug", window->debug);
    settings->setValueQStringList("recent", Util::FromNativeSeparators(window->recent));
    settings->setValueInt("maxRecent", window->maxRecent);
    settings->setValue("lang", window->lang);
    settings->setValueBool("gestures", window->gestures);
    settings->setValue("version", "2.0.2");
    settings->endGroup();

    settings->beginGroup("mpv");
    settings->setValueInt("volume", mpv->volume);
    settings->setValueDouble("speed", mpv->speed);
    if(mpv->screenshotFormat != "")
        settings->setValue("screenshot-format", mpv->screenshotFormat);
    if(mpv->screenshotTemplate != "")
        settings->setValue("screenshot-template", QDir::fromNativeSeparators(mpv->screenshotDir)+"/"+mpv->screenshotTemplate);
    settings->endGroup();

    settings->beginGroup("input");
    settings->clear();
    for(auto input_iter = input.begin(); input_iter != input.end(); ++input_iter)
    {
        auto default_iter = default_input.find(input_iter.key());
        if(default_iter != default_input.end())
            if(input_iter->first == default_iter->first &&
               input_iter->second == default_iter->second) // skip entries that are the same as a default_input entry
                continue;
        settings->setValue(input_iter.key(), input_iter->first + " # " + input_iter->second);
    }
    settings->endGroup();

    settings->Save();
}

void BakaEngine::Command(QString command)
{
    if(command == QString())
        return;
    QStringList args = command.split(" ");
    if(!args.empty())
    {
        if(args.front() == "mpv")
        {
            args.pop_front();
            if(mpv->playState > 0)
                MpvCommand(args);
        }
        else// if(args.front() == "baka")
        {
            if(args.front() == "baka")
                args.pop_front();
            BakaCommand(args);
        }
    }
    else
        InvalidCommand(args.join(' '));
}

void BakaEngine::MpvCommand(QStringList &args)
{
    if(!args.empty())
        mpv->CommandString(args.join(" "));
    else
        RequiresParameters("mpv");
}

void BakaEngine::BakaCommand(QStringList &args)
{
    if(!args.empty())
    {
        auto iter = BakaCommandMap.find(args.front());
        if(iter != BakaCommandMap.end())
        {
            args.pop_front();
            (this->*(*iter))(args); // execute command
        }
        else
            InvalidCommand(args.join(' '));
    }
    else
        RequiresParameters("baka");
}

void BakaEngine::BakaPrint(QString output)
{
    window->ui->outputTextEdit->moveCursor(QTextCursor::End);
    window->ui->outputTextEdit->insertPlainText(QString("[baka]: %0").arg(output));
}

void BakaEngine::MpvPrint(QString output)
{
    window->ui->outputTextEdit->moveCursor(QTextCursor::End);
    window->ui->outputTextEdit->insertPlainText(QString("[mpv]: %0").arg(output));
}

void BakaEngine::InvalidCommand(QString command)
{
    BakaPrint(tr("invalid command '%0'\n").arg(command));
}

void BakaEngine::InvalidParameter(QString parameter)
{
    BakaPrint(tr("invalid parameter '%0'\n").arg(parameter));
}

void BakaEngine::RequiresParameters(QString what)
{
    BakaPrint(tr("'%0'' requires parameters\n").arg(what));
}
