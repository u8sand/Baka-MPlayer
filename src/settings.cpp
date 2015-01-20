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
    if(f.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream fin(&f);
        fin.setCodec("UTF-8");
        QString line;
        int i;
        group = QString(); // reset the group
        do
        {
            line = fin.readLine().trimmed();
            if(line.startsWith('[') && line.endsWith(']')) // group
                group = line.mid(1, line.length()-2); // [...] <- get ...
            else if((i = ParseLine(line)) != -1) // foo=bar
                data[group][FixKeyOnLoad(line.left(i))] = line.mid(i+1); // data[group][foo]=bar
        } while(!line.isNull());
        f.close();
        group = QString(); // reset the group
    }
}

void Settings::Save()
{
    QFile f(file);
    if(f.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
    {
        QTextStream fout(&f);
        fout.setCodec("UTF-8");
        fout.setGenerateByteOrderMark(true);
        for(SettingsData::iterator group_iter = data.begin(); group_iter != data.end(); ++group_iter)
        {
            if(group_iter->empty()) // skip empty groups
                continue;
            fout << "[" << group_iter.key() << "]" << endl; // output: [foo]
            for(SettingsGroupData::iterator entry_iter = group_iter->begin(); entry_iter != group_iter->end(); ++entry_iter)
                fout << FixKeyOnSave(entry_iter.key()) << "=" << entry_iter.value() << endl;
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
    return SplitQStringList(value(key, FixQStringListOnSave(default_value).join(",")));
}

QDate Settings::valueQDate(QString key, QDate default_value)
{
    return QDate::fromString(value(key, default_value.toString()));
}

bool Settings::valueBool(QString key, bool default_value)
{
    return !(value(key, default_value ? "true" : "false") == "false");
}

int Settings::valueInt(QString key, int default_value)
{
    return value(key, QString::number(default_value)).toInt();
}

double Settings::valueDouble(QString key, double default_value)
{
    return value(key, QString::number(default_value)).toDouble();
}

void Settings::setValue(QString key, QString val)
{
    data[group][key] = val;
}

void Settings::setValueQStringList(QString key, QStringList val)
{
    setValue(key, FixQStringListOnSave(val).join(","));
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

QString Settings::FixKeyOnSave(QString key)
{
    for(int i = 0; i < key.length(); ++i)
    {
        // escape special chars
        if(key[i] == '=' ||
           key[i] == '\\')
            key.insert(i++, '\\');
    }
    return key;
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

QStringList Settings::FixQStringListOnSave(QStringList list)
{
    for(auto &str : list)
    {
        for(int i = 0; i < str.length(); ++i)
        {
            // escape special chars
            if(str[i] == ',' ||
               str[i] == '\\')
                str.insert(i++, '\\');
        }
    }
    return list;
}

QStringList Settings::SplitQStringList(QString str)
{
    QStringList list;
    QString entry;
    for(int i = 0; i < str.length(); ++i)
    {
        // revert escaped characters
        if(str[i] == '\\')
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
