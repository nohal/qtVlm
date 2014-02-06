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
#include <QFile>


#include "settings.h"
#include "dataDef.h"

QSettings *fileSettings;

SettingData settingData[SETTING_MAX];

#define SETTING_CUR_VERSION 1

void Settings::initSettings(void) {
    if(fileSettings)
        return;

    QString settingFname=appFolder.value("userFiles")+SETTINGS_FILE;

    // Check if file exists
    bool fExists=QFile(settingFname).exists();


    fileSettings = new QSettings(settingFname, QSettings::IniFormat);

    if(getSetting("settingVersion",-1,"main").toInt()!=SETTING_CUR_VERSION && fExists) {
        // need to remove current settings
        qWarning() << "[initSetting] bad version of setting";
        QMessageBox::information(NULL,tr("Setting file version"),tr("Wrong setting file version, previous one has been saved in qtVlm.ini.sav, new one created"));
        if(QFile(settingFname+".sav").exists())
            if(!QDir(appFolder.value("userFiles")).remove(QString(SETTINGS_FILE)+".sav"))
                qWarning() << "Can't remove sav file: " << settingFname+".sav";
        if(!QFile(settingFname).rename(settingFname+".sav"))
            qWarning() << "Can't rename setting file to sav: " << settingFname+".sav";
        if(fileSettings) delete fileSettings;
        fileSettings = new QSettings(settingFname, QSettings::IniFormat);
        setSetting("settingVersion",SETTING_CUR_VERSION,"main");
    }
    else {
        if(!fExists) {
            setSetting("settingVersion",SETTING_CUR_VERSION,"main");
            qWarning() << "[initSetting] no previous file";
        }
        else
            qWarning() << "[initSetting] setting OK";
    }

    initSettingArray();
}

#include "settings_ini.h"

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

void Settings::setSettingOld(const QString &key, const QVariant &value,
                           const QString &group) {
    setSetting(key,value,group);
}

QVariant Settings::getSettingOld(const QString &key, const QVariant &defaultValue,
                           const QString &group) {
    return getSetting(key,defaultValue,group);
}

QVariant Settings::getSetting(const int &key,const int &boatType) {
    if(key>=0 && key<SETTING_MAX)
        return Settings::getSetting(settingData[key].name,settingData[key].defaultValue,settingData[key].groupName,boatType);
    else {
        qWarning() << "[getSetting] Ukn key: " << key;
        return 0;
    }
}

bool Settings::hasSetting(const int &key) {
    if(key>=0 && key<SETTING_MAX)
        return Settings::hasSetting(settingData[key].name,settingData[key].groupName,BOAT_ANY);
    else
        return false;
}

void Settings::setSetting(const int &key, const QVariant &value,const int &boatType) {
    if(key>=0 && key<SETTING_MAX)
        Settings::setSetting(settingData[key].name,value,settingData[key].groupName,boatType);
    else
        qWarning() << "[setSetting] Ukn key: " << key;
}

QVariant Settings::getSettingDefault(const int &key) {
    if(key>=0 && key<SETTING_MAX)
        return settingData[key].defaultValue;
    else {
        qWarning() << "[getSettingDefault] Ukn key: " << key;
        return QVariant();
    }
}

bool Settings::hasSetting(const QString &key, const QString &group, const int &boatType) {
    bool res=false;
    if (fileSettings != NULL) {
        /* avons nous la cle avec boatType */
        fileSettings->beginGroup (Settings::computeGroupe(group,boatType));
        if(!fileSettings->contains(key)) {
            if(boatType!=BOAT_ANY && boatType!=BOAT_NOBOAT) {
                fileSettings->endGroup(); /* can't find key/value with a defined boatType */
                fileSettings->beginGroup(Settings::computeGroupe(group,BOAT_ANY));
                if(fileSettings->contains(key)) /* searching for key using boatType=ANY */
                   res=true;
            }
        }
        else
            res=true;
        fileSettings->endGroup();
    }
    return res;
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

void Settings::saveGeometry(QWidget * obj) {
    if(!obj) return ;
    Settings::setSetting(obj->objectName()+".height",obj->height(),"DialogGeometry");
    Settings::setSetting(obj->objectName()+".width",obj->width(),"DialogGeometry");
    Settings::setSetting(obj->objectName()+".positionx",obj->pos().x(),"DialogGeometry");
    Settings::setSetting(obj->objectName()+".positiony",obj->pos().y(),"DialogGeometry");
}

void Settings::restoreGeometry(QWidget * obj,int * h, int * w, int * x, int * y) {
    if(!obj || !h || ! w || !x || !y) return;
    *h=Settings::getSetting(obj->objectName()+".height",-1,"DialogGeometry").toInt();
    *w=Settings::getSetting(obj->objectName()+".width",-1,"DialogGeometry").toInt();
    *x=Settings::getSetting(obj->objectName()+".positionx",-1,"DialogGeometry").toInt();
    *y=Settings::getSetting(obj->objectName()+".positiony",-1,"DialogGeometry").toInt();
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
