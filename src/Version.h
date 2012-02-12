/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

Original code zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr
***********************************************************************/

#ifndef VERSION_H
#define VERSION_H

#define QTVLM_VERSION_NUM   "3.2"
#define QTVLM_SUB_VERSION_NUM   "5"
#define QTVLM_VERSION_DATE  __DATE__

#ifdef __WIN_QTVLM
        #define QTVLM_APP_NAME   "qtVlm"
        #define QTVLM_OS "Windows"
#endif
#ifdef __UNIX_QTVLM
        #define QTVLM_APP_NAME   "qtVlm"
        #define QTVLM_OS "Linux"
#endif
#ifdef __MAC_QTVLM
        #define QTVLM_APP_NAME   "qtVlm"
        #define QTVLM_OS "Macintosh"
#endif

#include <QString>

class Version {

    public:
        static QString getVersion() {
#ifdef QTVLM_SUB_VERSION_NUM
            return QString(QTVLM_VERSION_NUM)+"-"+QString(QTVLM_SUB_VERSION_NUM);
#else
            return QString(QTVLM_VERSION_NUM);
#endif

        }
        static QString get_OS() {
            return QString(QTVLM_OS);
        }
        static QString getDate() {
            return QString(QTVLM_VERSION_DATE);
        }
        static QString getCompleteName() {
            return QString(QString(QTVLM_APP_NAME)+" - ver "+getVersion()+" ("+getDate()+") for "+get_OS());
        }
};



#endif
