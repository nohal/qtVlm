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


#define QTVLM_VERSION_NUM   "1.5.3"
#define QTVLM_SUB_VERSION_NUM   "12"
#define QTVLM_VERSION_DATE  "2009-12-16"

#ifdef Q_OS_WIN32
        #define QTVLM_APP_NAME   "qtVlm_win"
#else
        #define QTVLM_APP_NAME   "qtVlm"
#endif



class Version {

    public:
        static QString getVersion() {
#ifdef QTVLM_SUB_VERSION_NUM
            return QString(QTVLM_VERSION_NUM)+"-"+QString(QTVLM_SUB_VERSION_NUM);
#else
            return QString(QTVLM_VERSION_NUM);
#endif

        }
        static QString getDate() {
            return QString(QTVLM_VERSION_DATE);
        }
        static QString getCompleteName() {
            return QString(QString(QTVLM_APP_NAME)+"-"+QTVLM_VERSION_NUM);
        }
};



#endif
