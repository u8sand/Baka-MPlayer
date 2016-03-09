#include "settings.h"

#include <QTextStream>
#include <QFile>

Settings::Settings(QString location, QObject *parent):
    QObject(parent)
{
    this->location = location;
}

Settings::~Settings()
{
}

void Settings::Load()
{
    QFile c(QString("%0/%1.ini").arg(location, SETTINGS_FILE));
    QFile r(QString("%0/%1").arg(location, QString("recent.json")));
    if(c.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString l = c.readLine();
        c.close();
        if(l.startsWith("{"))
        {
            if(c.open(QIODevice::ReadOnly | QIODevice::Text))
                config = QJsonDocument::fromJson(c.readAll());
            if(r.open(QIODevice::ReadOnly | QIODevice::Text))
                recent = QJsonDocument::fromJson(r.readAll());
        }
        else
            LoadIni();
    }
}

void Settings::LoadIni()
{
    QFile f(QString("%0/%1.ini").arg(location, SETTINGS_FILE));
    if(f.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream fin(&f);
        fin.setCodec("UTF-8");
        QString line;
        int i;
        QString group;
        QJsonObject group_obj,
                    root = getConfigRoot();
        do
        {
            line = fin.readLine().trimmed();
            if(line.startsWith('[') && line.endsWith(']')) // group
            {
                if(group != QString())
                    root[group] = group_obj;
                group = line.mid(1, line.length()-2); // [...] <- get ...
                group_obj = QJsonObject();
            }
            else if((i = ParseLine(line)) != -1) // foo=bar
            {
                QString key = FixKeyOnLoad(line.left(i)),
                        val = line.mid(i+1);
                bool    b = (val == "true" || val == "false");
                bool    iok;
                int     ival = val.toInt(&iok);
                bool    dok;
                double  dval = val.toDouble(&dok);
                if(group == "baka-mplayer")
                    root[key] = b ? QJsonValue(val=="true") : (iok ? (dok ? QJsonValue(dval) : QJsonValue(ival)) : QJsonValue(val));
                else
                    group_obj[key] = b ? QJsonValue(val=="true") : (iok ? (dok ? QJsonValue(dval) : QJsonValue(ival)) : QJsonValue(val));
            }
        } while(!line.isNull());
        f.close();

        if(group != QString())
            root[group] = group_obj;

        // remove excess baka-mplayer group
        root.remove("baka-mplayer");

        // fix input
        if(root.find("input") != root.end())
        {
            QJsonObject input_obj = root["input"].toObject();
            for(auto in : input_obj)
            {
                QJsonArray opts;
                for(auto &o : in.toString().split('#'))
                    opts.append(o.trimmed());
                in = opts;
            }
            root["input"] = input_obj;
        }

        // fix recent
        if(root.find("recent") != root.end())
        {
            QStringList recent = SplitQStringList(root["recent"].toString());
            QJsonArray R;
            for(auto str : recent)
            {
                QJsonObject r;
                r["path"] = str;
                R.append(r);
            }
            root["recent"] = R;
        }

        setConfigRoot(root);
    }
}

void Settings::Save()
{
    QFile c(QString("%0/%1.ini").arg(location, SETTINGS_FILE));
    if(c.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
    {
        c.write(config.toJson());
        c.close();
    }
    QFile r(QString("%0/%1").arg(location, QString("recent.json")));
    if(r.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
    {
        r.write(recent.toJson());
        r.close();
    }
}

QJsonObject Settings::getConfigRoot()
{
    return config.object();
}

void Settings::setConfigRoot(QJsonObject root)
{
    config.setObject(root);
}

QJsonObject Settings::getRecentRoot()
{
    return recent.object();
}

void Settings::setRecentRoot(QJsonObject root)
{
    recent.setObject(root);
}

int Settings::ParseLine(QString line)
{
    for(int i = 0; i < line.length(); ++i)
    {
        if(line[i] == '\\')
            ++i; // skip the next char (escape sequence)
        else if(line[i] == '=')
            return i;
    }
    return -1;
}

QString Settings::FixKeyOnLoad(QString key)
{
    for(int i = 0; i < key.length(); ++i)
    {
        // revert escaped characters
        if(key[i] == '\\')
            key.remove(i, 1);
    }
    return key;
}

QStringList Settings::SplitQStringList(QString str)
{
    QStringList list;
    QString entry;
    for(int i = 0; i < str.length(); ++i)
    {
        // revert escaped characters
        if(str[i] == '\\' && (str[i+1] == ',' || str[i+1] == '\\')) // treat invalid escape sequences as normal '\'s
            entry += str[++i];
        else if(str[i] == ',')
        {
            entry = entry.trimmed();
            if(entry != QString())
            {
                list.push_back(entry);
                entry = QString();
            }
        }
        else
            entry += str[i];

    }
    entry = entry.trimmed();
    if(entry != QString())
        list.push_back(entry);
    return list;
}
