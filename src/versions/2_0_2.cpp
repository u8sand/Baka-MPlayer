#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "util.h"

#if defined(Q_OS_WIN)
#include <QDate>
#include "ui/updatedialog.h"
#endif

void BakaEngine::Load2_0_2()
{
    LoadBaka2_0_2();
    LoadMpv2_0_0();
    LoadInput2_0_2();
}

void BakaEngine::LoadBaka2_0_2()
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
    window->recent = Util::ToNativeSeparators(settings->valueQStringList("recent"));
    window->maxRecent = settings->valueInt("maxRecent", 5);
    window->gestures = settings->valueBool("gestures", true);
    window->setLang(settings->value("lang", "auto"));
#if defined(Q_OS_WIN)
    QDate last = settings->valueQDate("lastcheck", QDate(2014, 1, 1));
    if(last.daysTo(QDate::currentDate()) > 7) // been a week since we last checked?
    {
        updateDialog->CheckForUpdates();
        settings->setValueQDate("lastcheck", QDate::currentDate());
    }
#endif
    settings->endGroup();
    window->UpdateRecentFiles();
}

void BakaEngine::LoadInput2_0_2()
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
