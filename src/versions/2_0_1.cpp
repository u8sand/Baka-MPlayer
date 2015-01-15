#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "util.h"

void BakaEngine::Load2_0_1()
{
    LoadBaka2_0_1();
    LoadMpv2_0_0();
}

void BakaEngine::LoadBaka2_0_1()
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
    window->gestures = true;
    window->setLang("auto");
    settings->endGroup();
    window->UpdateRecentFiles();
}
