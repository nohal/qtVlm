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

    if(!fileSettings)
    {
        fileSettings = new QSettings(appFolder.value("userFiles")+SETTINGS_FILE, QSettings::IniFormat);
    }
}

void Settings::loadFromReg(void)
{
    if(!fileSettings)
    {
        fileSettings = new QSettings(appFolder.value("userFiles")+SETTINGS_FILE, QSettings::IniFormat);
    }

    QSettings loc_settings("qtVlm");
    // Read All settings from global storage (childKeys)
    // and write it to user directory
    loc_settings.setFallbacksEnabled(false);
    loc_settings.beginGroup("main");
    fileSettings->beginGroup("main");
    QStringList oldkeys = loc_settings.childKeys();
    QStringListIterator it(oldkeys);
    while (it.hasNext()) {
        QString key = it.next();
        QVariant val = loc_settings.value(key);

        fileSettings->setValue(key, val);
    }
    loc_settings.endGroup();
    //Pas de suppression pour l'instant
    //loc_settings.clear();
    fileSettings->endGroup();
    fileSettings->sync();
}

void Settings::setSetting(const QString &key, const QVariant &value)
{
    if (fileSettings)
    {
        fileSettings->beginGroup("main");
        fileSettings->setValue(key, value);
        fileSettings->endGroup();
        fileSettings->sync();
    }
}

QVariant Settings::getSetting(const QString &key, const QVariant &defaultValue)
{
    QVariant val=defaultValue;
    if (fileSettings != NULL)
    {
        fileSettings->beginGroup ("main");
        val = fileSettings->value (key, defaultValue);
        fileSettings->endGroup();
        fileSettings->sync();
    }
    return val;
}
