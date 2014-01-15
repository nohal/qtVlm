/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************/


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QObject>

#include "dataDef.h"

#define SETTINGS_FILE "qtVlm.ini"

static const QString boatTypes[]={"BOAT_VLM","BOAT_REAL",""};

struct SettingData {
    QString name;
    QString groupName;
    QVariant defaultValue;
};

#include "settings_def.h"

class Settings : QObject
{ Q_OBJECT
    public:
        static void     initSettings(void);
        static void     initSettingArray(void);
        static QVariant getSetting(const int &key,const int &boatType=BOAT_ANY);
        static QVariant getSettingDefault(const int &key);
        static void     setSetting(const int &key, const QVariant &value,const int &boatType=BOAT_ANY);

        static void     removeSetting(const QString &key,
                                   const QString &group="main",const int &boatType=BOAT_ANY);
        static bool     hasSetting(const int &key);
        static QStringList getAllKeys(const QString &group="main", const int &boatType=BOAT_ANY);

        static void     setSettingOld(const QString &key, const QVariant &value,
                                   const QString &group);
        static QVariant getSettingOld(const QString &key, const QVariant &defaultValue,
                                   const QString &group);

        static void saveGeometry(QWidget * obj);
        static void restoreGeometry(QWidget * obj,int * h, int * w, int * x, int * y);


    private:
        static QString  computeGroupe(const QString &group,const int &boatType);
        static void     setSetting(const QString &key, const QVariant &value,
                                   const QString &group="main",const int &boatType=BOAT_ANY);
        static QVariant getSetting(const QString &key, const QVariant &defaultValue,
                                   const QString &group="main",const int &boatType=BOAT_ANY);
        static bool     hasSetting(const QString &key, const QString &group="main", const int &boatType=BOAT_ANY);
};
Q_DECLARE_TYPEINFO(Settings,Q_MOVABLE_TYPE);

#endif // SETTINGS_H
