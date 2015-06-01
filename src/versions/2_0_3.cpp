#include "bakaengine.h"

#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "util.h"
#include "mpvhandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QDir>

#if defined(Q_OS_WIN)
#include <QDate>
#include "updatemanager.h"
#endif

void BakaEngine::Load2_0_3()
{
    QJsonObject root = settings->getRoot();
    window->setOnTop(root["onTop"].toString("never"));
    window->setAutoFit(root["autoFit"].toInt(100));
    sysTrayIcon->setVisible(root["trayIcon"].toBool(false));
    window->setHidePopup(root["hidePopup"].toBool(false));
    window->setRemaining(root["remaining"].toBool(true));
    window->ui->splitter->setNormalPosition(root["splitter"].toInt(window->ui->splitter->max()*1.0/8));
    window->setDebug(root["debug"].toBool(false));
    window->ui->hideFilesButton->setChecked(!root["showAll"].toBool(true));
    window->setScreenshotDialog(root["screenshotDialog"].toBool(true));
    window->recent.clear();
    for(auto entry : root["recent"].toArray())
    {
        QJsonObject entry_json = entry.toObject();
        window->recent.append(Recent(entry_json["path"].toString(), entry_json["title"].toString(), entry_json["time"].toInt(0)));
    }
    window->maxRecent = root["maxRecent"].toInt(5);
    window->gestures = root["gestures"].toBool(true);
    window->setLang(root["lang"].toString("auto"));
#if defined(Q_OS_WIN)
    QDate last = QDate::fromString(root["lastcheck"].toString()); // convert to date
    if(last.daysTo(QDate::currentDate()) > 7) // been a week since we last checked?
    {
        update->CheckForUpdates();
        root["lastcheck"] = QDate::currentDate().toString();
    }
#endif
    window->UpdateRecentFiles();

    // apply default shortcut mappings
    input = default_input;

    // load settings defined input bindings
    QJsonObject input_json = root["input"].toObject();
    for(auto &key : input_json.keys())
    {
        QJsonArray parts = input_json[key].toArray();
        input[key] = QPair<QString, QString>{
            parts[0].toString(),
            parts[1].toString()
        };
    }

    window->MapShortcuts();

    QJsonObject mpv_json = root["mpv"].toObject();
    mpv->Volume(mpv_json["volume"].toInt(100));
    mpv_json.remove("volume");
    mpv->Speed(mpv_json["speed"].toDouble(1.0));
    mpv_json.remove("speed");
    mpv->Vo(mpv_json["vo"].toString());
    mpv_json.remove("vo");
    QString temp = mpv_json["screenshot-template"].toString();
    if(!temp.isEmpty()) // default screenshot template
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
    mpv_json.remove("screenshot-template");
    for(auto &key : mpv_json.keys())
        if(key != QString() && mpv_json[key].toString() != QString())
            mpv->SetOption(key, mpv_json[key].toString());
}

void BakaEngine::SaveSettings()
{
    QJsonObject root = settings->getRoot();
    root["onTop"] = window->onTop;
    root["autoFit"] = window->autoFit;
    root["trayIcon"] = sysTrayIcon->isVisible();
    root["hidePopup"] = window->hidePopup;
    root["remaining"] = window->remaining;
    root["splitter"] = (window->ui->splitter->position() == 0 ||
                                    window->ui->splitter->position() == window->ui->splitter->max()) ?
                                    window->ui->splitter->normalPosition() :
                                    window->ui->splitter->position();
    root["showAll"] = !window->ui->hideFilesButton->isChecked();
    root["screenshotDialog"] = window->screenshotDialog;
    root["debug"] = window->debug;
    root["maxRecent"] = window->maxRecent;
    root["lang"] = window->lang;
    root["gestures"] = window->gestures;
    root["version"] = "2.0.3";

    QJsonArray recent_json;
    for(auto &entry : window->recent)
    {
        QJsonObject recent_sub_object_json;
        recent_sub_object_json["path"] = QDir::fromNativeSeparators(entry.path);
        if(entry.title != QString())
            recent_sub_object_json["title"] = entry.title;
        if(entry.time > 0)
            recent_sub_object_json["time"] = entry.time;
        recent_json.append(recent_sub_object_json);
    }
    root["recent"] = recent_json;

    QJsonObject input_json;
    for(auto input_iter = input.begin(); input_iter != input.end(); ++input_iter)
    {
        auto default_iter = default_input.find(input_iter.key());
        if(default_iter != default_input.end())
        {
            if(input_iter->first == default_iter->first &&
               input_iter->second == default_iter->second) // skip entries that are the same as a default_input entry
                continue;
        }
        else // not found in defaults
        {
            if(*input_iter == QPair<QString, QString>({QString(), QString()})) // skip empty entries
                continue;
        }
        input_json[input_iter.key()] = QJsonArray{input_iter->first, input_iter->second};
    }
    root["input"] = input_json;

    QJsonObject mpv_json = root["mpv"].toObject();
    mpv_json["volume"] = mpv->volume;
    mpv_json["speed"] = mpv->speed;
    mpv_json["vo"] = mpv->vo;
    if(mpv->screenshotFormat != "")
        mpv_json["screenshot-format"] = mpv->screenshotFormat;
    if(mpv->screenshotTemplate != "")
        mpv_json["screenshot-template"] = QDir::fromNativeSeparators(mpv->screenshotDir)+"/"+mpv->screenshotTemplate;
    root["mpv"] = mpv_json;

    settings->setRoot(root);
    settings->Save();
}
