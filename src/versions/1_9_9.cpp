#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "mpvhandler.h"
#include "util.h"

#include <QDir>

void BakaEngine::Load1_9_9()
{
    LoadBaka1_9_9();
    LoadMpv1_9_9();
}

void BakaEngine::LoadBaka1_9_9()
{
    settings->beginGroup("window");
    window->setOnTop(settings->value("onTop", "never"));
    window->setAutoFit(settings->valueInt("autoFit", 100));
    window->sysTrayIcon->setVisible(settings->valueBool("trayIcon", false));
    window->setHidePopup(settings->valueBool("hidePopup", false));
    window->setRemaining(settings->valueBool("remaining", true));
    window->ui->splitter->setNormalPosition(settings->valueInt("splitter", window->ui->splitter->max()*1.0/8));
    window->ui->hideFilesButton->setChecked(!settings->valueBool("showAll", true));
    settings->endGroup();
    settings->beginGroup("common");
    window->setDebug(settings->valueBool("debug", false));
    settings->endGroup();
    window->maxRecent = 5;
    window->setScreenshotDialog(true);
    settings->beginGroup("mpv");
    QString lf = settings->value("mpv/lastFile");
    if(lf != QString())
        window->recent.push_front(QDir::toNativeSeparators(lf));
    settings->endGroup();
    window->gestures = true;
    window->setLang("auto");
    window->UpdateRecentFiles();
}

void BakaEngine::LoadMpv1_9_9()
{
    settings->beginGroup("mpv");
    mpv->ScreenshotFormat(settings->value("screenshotFormat", "jpg"));
    mpv->ScreenshotDirectory(QDir::toNativeSeparators(settings->value("screenshotDir", ".")));
    mpv->ScreenshotTemplate(settings->value("screenshotTemplate"));
    mpv->Speed(settings->valueDouble("speed", 1.0));
    mpv->Volume(settings->valueInt("volume", 100));
    settings->endGroup();
    settings->beginGroup("common");
    mpv->Debug(settings->valueBool("debug", false));
    settings->endGroup();
}
