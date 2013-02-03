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

void Settings::initSettings(void)
{
#if 0
    if (! QFile::exists (appFolder.value("userFiles")+SETTINGS_FILE))
    {
        qWarning() << "No setting file found";
        QSettings loc_settings("qtVlm");
        loc_settings.beginGroup("main");
        if(loc_settings.childKeys().count() > 0)
        {
            /* Il y a des settings à transferer*/
            if( QMessageBox::question(NULL,tr("Ouverture des paramètres"),
                tr("Il existe des parametres locaux, les importer dans qtvlm.ini?"),
                QMessageBox::Yes | QMessageBox::No
                ) == QMessageBox::Yes)
            {
                Settings::loadFromReg();
            }
        }
    }
#endif
    if(!fileSettings)
        fileSettings = new QSettings(appFolder.value("userFiles")+SETTINGS_FILE, QSettings::IniFormat);
}

void Settings::setSetting(const QString &key, const QVariant &value, const QString &group)
{
    if (fileSettings) {
        fileSettings->beginGroup(group);
        fileSettings->setValue(key, value);
        fileSettings->endGroup();
        fileSettings->sync();
    }
}

QVariant Settings::getSetting(const QString &key, const QVariant &defaultValue, const QString &group)
{
    QVariant val=defaultValue;
    if (fileSettings != NULL) {
        fileSettings->beginGroup (group);
        val = fileSettings->value (key, defaultValue);
        fileSettings->endGroup();
        fileSettings->sync();
    }
    return val;
}

void Settings::removeSetting(const QString &key, const QString &group)
{
    if(key.isEmpty()) return ; /* prevent from deleting all key */

    fileSettings->beginGroup(group);
    fileSettings->remove(key);
    fileSettings->endGroup();
    fileSettings->sync();
}
