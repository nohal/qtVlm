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

#define SETTINGS_FILE "qtVlm.ini"

class Settings : QObject
{ Q_OBJECT
    public:

        static void loadFromReg(void);
        static void initSettings(void);
        static void     setSetting(const QString &key, const QVariant &value);
        static QVariant getSetting(const QString &key, const QVariant &defaultValue);
};
Q_DECLARE_TYPEINFO(Settings,Q_MOVABLE_TYPE);

#endif // SETTINGS_H
