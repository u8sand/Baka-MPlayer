#include "settings.h"

#include <QTextStream>
#include <QFile>

Settings::Settings(QString file, QObject *parent):
    QObject(parent)
{
    this->file = file;
}

Settings::~Settings()
{
}

void Settings::Load()
{
    QFile f(file);
    if(f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString l = f.readLine();
        f.close();
        if(l.startsWith("{"))
        {
            if(f.open(QIODevice::ReadOnly | QIODevice::Text))
                document = QJsonDocument::fromJson(f.readAll());
        }
        else
            LoadIni();
    }
}

void Settings::LoadIni()
{
    QFile f(file);
    if(f.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream fin(&f);
        fin.setCodec("UTF-8");
        QString line;
        int i;
        QString group;
        QJsonObject group_obj,
                    root = getRoot();
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

        setRoot(root);
    }
}

void Settings::Save()
{
    QFile f(file);
    if(f.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
    {
        f.write(document.toJson());
        f.close();
    }
}

QJsonObject Settings::getRoot()
{
    return document.object();
}

void Settings::setRoot(QJsonObject root)
{
    document.setObject(root);
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
