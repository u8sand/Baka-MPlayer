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

    // command action mappings (action (right) performs command (left))
    const QHash<QString, QAction*> commandActionMap = {
        {"mpv add chapter +1", window->ui->action_Next_Chapter},
        {"mpv add chapter -1", window->ui->action_Previous_Chapter},
        {"mpv add speed +0.25", window->ui->action_Increase},
        {"mpv add speed -0.25", window->ui->action_Decrease},
        {"mpv add sub-scale +0.02", window->ui->action_Size},
        {"mpv add sub-scale -0.02", window->ui->actionS_ize},
        {"mpv frame_back_step", window->ui->actionFrame_Back_Step},
        {"mpv frame_step", window->ui->action_Frame_Step},
        {"mpv screenshot subtitles", window->ui->actionWith_Subtitles},
        {"mpv screenshot video", window->ui->actionWithout_Subtitles},
        {"mpv set speed 1", window->ui->action_Reset},
        {"mpv set sub-scale 1", window->ui->action_Reset_Size},
        {"mpv set time-pos 0", window->ui->action_Restart},
        {"mpv set video-aspect -1", window->ui->action_Auto_Detect},
        {"mpv set video-aspect 16:9", window->ui->actionForce_16_9},
        {"mpv set video-aspect 2_35:1", window->ui->actionForce_2_35_1},
        {"mpv set video-aspect 4:3", window->ui->actionForce_4_3},
        {"mpv toggle sub-visibility", window->ui->actionShow_Subtitles},
        {"baka add_subtitles", window->ui->action_Add_Subtitle_File},
        {"baka fitwindow 0", window->ui->action_To_Current_Size},
        {"baka fitwindow 50", window->ui->action50},
        {"baka fitwindow 75", window->ui->action75},
        {"baka fitwindow 100", window->ui->action100},
        {"baka fitwindow 150", window->ui->action150},
        {"baka fitwindow 200", window->ui->action200},
        {"baka fullscreen", window->ui->action_Full_Screen},
        {"baka jump", window->ui->action_Jump_to_Time},
        {"baka media_info", window->ui->actionMedia_Info},
        {"baka new", window->ui->action_New_Player},
        {"baka open", window->ui->action_Open_File},
        {"baka open_clipboard", window->ui->actionOpen_Path_from_Clipboard},
        {"baka open_url", window->ui->actionOpen_URL},
        {"baka play +1", window->ui->actionPlay_Next_File},
        {"baka play -1", window->ui->actionPlay_Previous_File},
        {"baka playlist repeat off", window->ui->action_Off},
        {"baka playlist repeat playlist", window->ui->action_Playlist},
        {"baka playlist repeat this", window->ui->action_This_File},
        {"baka playlist shuffle", window->ui->actionSh_uffle},
        {"baka playlist toggle", window->ui->action_Show_Playlist},
        {"baka playlist full", window->ui->action_Hide_Album_Art},
        {"baka dim", window->ui->action_Dim_Lights},
        {"baka play_pause", window->ui->action_Play},
        {"baka quit", window->ui->actionE_xit},
        {"baka show_in_folder", window->ui->actionShow_in_Folder},
        {"baka stop", window->ui->action_Stop},
        {"baka volume +5", window->ui->action_Increase_Volume},
        {"baka volume -5", window->ui->action_Decrease_Volume},
        {"baka output", window->ui->actionShow_D_ebug_Output},
        {"baka preferences", window->ui->action_Preferences},
        {"baka online_help", window->ui->actionOnline_Help},
        {"baka update", window->ui->action_Check_for_Updates},
        {"baka about qt", window->ui->actionAbout_Qt},
        {"baka about", window->ui->actionAbout_Baka_MPlayer}
    };

    // apply default shortcut mappings
    input = default_input;

    // load settings defined input bindings
    for(Settings::SettingsGroupData::iterator entry = settings->map().begin(); entry != settings->map().end(); ++entry)
    {
        QStringList parts = entry.value().split('#', QString::SkipEmptyParts);
        QPair<QString, QString> pair;
        pair.first = parts.front().trimmed();
        parts.pop_front();
        if(!parts.empty())
            pair.second = parts.join('#').trimmed();
        input[entry.key()] = pair;
    }

    // map shortcuts to actions
    for(auto input_iter = input.begin(); input_iter != input.end(); ++input_iter)
    {
        auto commandAction = commandActionMap.find(input_iter->first);
        if(commandAction != commandActionMap.end())
            (*commandAction)->setShortcut(QKeySequence(input_iter.key()));
    }

    // map actions to commands
    for(auto action = commandActionMap.begin(); action != commandActionMap.end(); ++action)
    {
        const QString cmd = action.key();
        connect(*action, &QAction::triggered,
                [=] { Command(cmd); });
    }

    settings->endGroup();
}
