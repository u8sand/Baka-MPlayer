#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "util.h"
#include "mpvhandler.h"

#include <QDir>

#if defined(Q_OS_WIN)
#include <QDate>
#include "updatemanager.h"
#endif

void BakaEngine::Load2_0_2(Settings *settings)
{
    LoadBaka2_0_2(settings);
    LoadMpv2_0_0(settings);
    LoadInput2_0_2(settings);
}

void BakaEngine::LoadBaka2_0_2(Settings *settings)
{
    settings->beginGroup("baka-mplayer");
    window->setOnTop(settings->value("onTop", "never"));
    window->setAutoFit(settings->valueInt("autoFit", 100));
    sysTrayIcon->setVisible(settings->valueBool("trayIcon", false));
    window->setHidePopup(settings->valueBool("hidePopup", false));
    window->setRemaining(settings->valueBool("remaining", true));
    window->ui->splitter->setNormalPosition(settings->valueInt("splitter", window->ui->splitter->max()*1.0/8));
    window->setDebug(settings->valueBool("debug", false));
    window->ui->hideFilesButton->setChecked(!settings->valueBool("showAll", true));
    window->setScreenshotDialog(settings->valueBool("screenshotDialog", true));
    for(auto &entry : Util::ToNativeSeparators(settings->valueQStringList("recent")))
        window->recent.push_back(entry);
    window->maxRecent = settings->valueInt("maxRecent", 5);
    window->gestures = settings->valueBool("gestures", true);
    window->setLang(settings->value("lang", "auto"));
#if defined(Q_OS_WIN)
    QDate last = settings->valueQDate("lastcheck", QDate(2014, 1, 1));
    if(last.daysTo(QDate::currentDate()) > 7) // been a week since we last checked?
    {
        update->CheckForUpdates();
        settings->setValueQDate("lastcheck", QDate::currentDate());
    }
#endif
    settings->endGroup();
    window->UpdateRecentFiles();
}

void BakaEngine::LoadMpv2_0_0(Settings *settings)
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
            if(!entry.value().isEmpty()) // default screenshot template
            {
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
        }
        else if(entry.key() != QString() && entry.value() != QString())
            mpv->SetOption(entry.key(), entry.value());
    }
    settings->endGroup();
}

void BakaEngine::LoadInput2_0_2(Settings *settings)
{
    settings->beginGroup("input");

    // apply default shortcut mappings
    input = default_input;

    // load settings defined input bindings
    for(Settings::SettingsGroupData::iterator entry = settings->map().begin(); entry != settings->map().end(); ++entry)
    {
        QStringList parts = entry.value().split('#');
        QPair<QString, QString> pair;
        pair.first = parts.front().trimmed();
        parts.pop_front();
        if(!parts.empty())
            pair.second = parts.join('#').trimmed();
        input[entry.key()] = pair;
    }

    window->MapShortcuts();

    settings->endGroup();
}
