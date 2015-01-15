#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDate>

class BakaEngine;

class Settings : public QObject
{
    Q_OBJECT
public:
    typedef QMap<QString, QString> SettingsGroupData;
    typedef QMap<QString, SettingsGroupData> SettingsData;

    explicit Settings(QString file, QObject *parent = 0);
    ~Settings();

public slots:
    void Load();
    void Save();

    void beginGroup(QString grp) { group = grp; }
    void endGroup()              { group = QString(); }
    void clear()                 { if(group == QString()) { data.clear(); } else { data[group].clear(); } }
    SettingsGroupData &map()     { return data[group]; }

    QString     value(QString key, QString default_value = QString());
    QStringList valueQStringList(QString key, QStringList default_value = QStringList());
    QDate       valueQDate(QString key, QDate default_value = QDate());
    bool        valueBool(QString key, bool default_value = false);
    int         valueInt(QString key, int default_value = 0);
    double      valueDouble(QString key, double default_value = 0.0);

    void     setValue(QString key, QString val);
    void     setValueQStringList(QString key, QStringList val);
    void     setValueQDate(QString key, QDate val);
    void     setValueBool(QString key, bool val);
    void     setValueInt(QString key, int val);
    void     setValueDouble(QString key, double val);

    bool isEmpty() { return data.isEmpty(); }

protected:
    int ParseLine(QString line);
    QString FixKeyOnSave(QString key);
    QString FixKeyOnLoad(QString key);
    QStringList FixQStringListOnSave(QStringList list);
    QStringList SplitQStringList(QString list);

private:
    BakaEngine *baka;

    SettingsData data;
    QString group, file;
    //bool readOnly;
};

#endif // SETTINGS_H
