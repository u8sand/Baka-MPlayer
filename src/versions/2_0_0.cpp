#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mpvhandler.h"

#include <QDir>

void BakaEngine::Load2_0_0()
{
    LoadBaka2_0_0();
    LoadMpv2_0_0();
}

void BakaEngine::LoadBaka2_0_0()
{
    settings->beginGroup("baka-mplayer");
    window->setOnTop(QString(settings->value("onTop", "never")));
    window->setAutoFit(settings->valueInt("autoFit", 100));
    sysTrayIcon->setVisible(settings->valueBool("trayIcon", false));
    window->setHidePopup(settings->valueBool("hidePopup", false));
    window->setRemaining(settings->valueBool("remaining", true));
    window->ui->splitter->setNormalPosition(settings->valueInt("splitter", window->ui->splitter->max()*1.0/8));
    window->setDebug(settings->valueBool("debug", false));
    window->ui->hideFilesButton->setChecked(!settings->valueBool("showAll", true));
    window->setScreenshotDialog(settings->valueBool("screenshotDialog", true));
    window->maxRecent = 5;
    QString lf = settings->value("lastFile");
    if(lf != QString())
        window->recent.push_front(QDir::toNativeSeparators(lf));
    window->gestures = true;
    window->setLang("auto");
    settings->endGroup();
    window->UpdateRecentFiles();
}

void BakaEngine::LoadMpv2_0_0()
{
    settings->beginGroup("mpv");
    for(Settings::SettingsGroupData::iterator entry = settings->map().begin(); entry != settings->map().end(); ++entry)
    {
        if(entry.key() == "volume") // exception--we want to update our ui accordingly
            mpv->Volume(entry.value().toInt());
        else if(entry.key() == "speed")
            mpv->Speed(entry.value().toDouble());
        else if(entry.key() == "screenshot-template")
        {
            QString temp = entry.value();
            int i = temp.lastIndexOf('/');
            if(i != -1)
            {
                mpv->ScreenshotDirectory(QDir::toNativeSeparators(temp.mid(0, i)));
                mpv->ScreenshotTemplate(temp.mid(i+1));
            }
            else
            {
                mpv->ScreenshotDirectory(".");
                mpv->ScreenshotTemplate(temp);
            }
        }
        else if(entry.key() != QString() && entry.value() != QString())
            mpv->SetOption(entry.key(), entry.value());
    }
    settings->endGroup();
}
