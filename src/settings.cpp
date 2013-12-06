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

#include <QMessageBox>
#include <QDir>
#include <QDebug>


#include "settings.h"
#include "dataDef.h"

QSettings *fileSettings;

void Settings::initSettings(void) {
    if(!fileSettings)
        fileSettings = new QSettings(appFolder.value("userFiles")+SETTINGS_FILE, QSettings::IniFormat);
}

void Settings::setSetting(const QString &key, const QVariant &value, const QString &group, const int &boatType) {
    if (fileSettings) {
        fileSettings->beginGroup(Settings::computeGroupe(group,boatType));
        fileSettings->setValue(key, value);
        fileSettings->endGroup();
        fileSettings->sync();
    }
}
QStringList Settings::getAllKeys(const QString &group, const int &boatType)
{
    if (fileSettings != NULL) {
        /* avons nous la cle avec boatType */
        fileSettings->beginGroup (Settings::computeGroupe(group,boatType));
        QStringList result=fileSettings->allKeys();
        fileSettings->endGroup();
        return result;
    }
    else
        return QStringList();
}

QVariant Settings::getSetting(const QString &key, const QVariant &defaultValue, const QString &group, const int &boatType) {
    QVariant val=defaultValue;
    if (fileSettings != NULL) {
        /* avons nous la cle avec boatType */
        fileSettings->beginGroup (Settings::computeGroupe(group,boatType));
        if(!fileSettings->contains(key)) {
            if(boatType!=BOAT_ANY && boatType!=BOAT_NOBOAT) {
                fileSettings->endGroup(); /* can't find key/value with a defined boatType */
                fileSettings->beginGroup(Settings::computeGroupe(group,BOAT_ANY));
                if(fileSettings->contains(key)) { /* searching for key using boatType=ANY */
                    val = fileSettings->value (key, defaultValue);
                    fileSettings->endGroup();
                    /* key found => remove the ANY key and create 1 specific key for each boat type */
    /* this is only for transition purpose */
                    Settings::removeSetting(key,group,BOAT_ANY);
                    Settings::setSetting(key,val,group,BOAT_VLM);
                    Settings::setSetting(key,val,group,BOAT_REAL);
                }
                else {
                    /* key not found for ANY => return default */
                    fileSettings->endGroup();
                }
            }
            else {
                /* key not found and boatType == ANY => return default */
                fileSettings->endGroup();
            }
        }
        else {
            /* get val */
            val = fileSettings->value (key, defaultValue);
            fileSettings->endGroup();
        }
        fileSettings->sync();
    }
    return val;
}

void Settings::removeSetting(const QString &key, const QString &group, const int &boatType) {
    if(key.isEmpty()) return ; /* prevent from deleting all key */

    fileSettings->beginGroup(Settings::computeGroupe(group,boatType));
    fileSettings->remove(key);
    fileSettings->endGroup();
    fileSettings->sync();
}

QString Settings::computeGroupe(const QString &group, const int &boatType) {
    if(boatType==BOAT_ANY || boatType==BOAT_NOBOAT)
        return group;
    else
        return group+"_"+boatTypes[boatType];
}
