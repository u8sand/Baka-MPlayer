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

int Settings::ParseLine(QString line)
{
    for(int i = 0; i < line.length(); ++i)
    {
        if(line[i] == QChar('\\'))
            ++i; // skip the next char (escape sequence)
        else if(line[i] == QChar('='))
            return i;
    }
    return -1;
}

void Settings::Load()
{
    QFile f(file);
    if(f.open(QFile::ReadOnly))
    {
        QTextStream fin(&f);
        QString line;
        int i;
        group = QString(); // reset the group
        do
        {
            line = fin.readLine().trimmed();
            if(line.startsWith('[') && line.endsWith(']')) // group
                group = line.mid(1, line.length()-2); // [...] <- get ...
            else if((i = ParseLine(line)) != -1) // foo=bar
                data[group][line.left(i)] = line.mid(i+1); // data[group][foo]=bar
        } while(!line.isNull());
        f.close();
        group = QString(); // reset the group
    }
}

void Settings::Save()
{
    QFile f(file);
    if(f.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream fout(&f);
        QString tmp;
        for(SettingsData::iterator group_iter = data.begin(); group_iter != data.end(); ++group_iter)
        {
            fout << "[" << group_iter.key() << "]" << endl; // output: [foo]
            for(SettingsGroupData::iterator entry_iter = group_iter->begin(); entry_iter != group_iter->end(); ++entry_iter)
            {
                // output: foo=bar  if foo has = signs they get backslashed  eg.  Ctrl+= = bar=blah -> Ctrl+\= = bar=blah
                tmp = entry_iter.key();
                tmp = tmp.replace("=", "\\=");
                fout << tmp << "=" << entry_iter.value() << endl;
            }
            fout << endl;
        }
        f.close();
    }
}

QString Settings::value(QString key, QString default_value)
{
    SettingsGroupData::iterator iter = data[group].find(key);
    if(iter == data[group].end())
        data[group][key] = default_value;
    return data[group][key];
}

QStringList Settings::valueQStringList(QString key, QStringList default_value)
{
    return QStringList(QString(value(key, default_value.join(","))));
}

QDate Settings::valueQDate(QString key, QDate default_value)
{
    return QDate::fromString(QString(value(key, default_value.toString())));
}

bool Settings::valueBool(QString key, bool default_value)
{
    return !(QString(value(key, default_value ? "true" : "false")) == "false");
}

int Settings::valueInt(QString key, int default_value)
{
    return QString(value(key, QString::number(default_value))).toInt();
}

double Settings::valueDouble(QString key, double default_value)
{
    return QString(value(key, QString::number(default_value))).toDouble();
}

void Settings::setValue(QString key, QString val)
{
    data[group][key] = val;
}

void Settings::setValueQStringList(QString key, QStringList val)
{
    setValue(key, val.join(","));
}

void Settings::setValueQDate(QString key, QDate val)
{
    setValue(key, val.toString());
}

void Settings::setValueBool(QString key, bool val)
{
    setValue(key, val ? "true" : "false");
}

void Settings::setValueInt(QString key, int val)
{
    setValue(key, QString::number(val));
}

void Settings::setValueDouble(QString key, double val)
{
    setValue(key, QString::number(val));
}
