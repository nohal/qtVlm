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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#ifndef GSHHSDWNLOAD_H
#define GSHHSDWNLOAD_H

#include <QObject>

#include "inetClient.h"

#include "class_list.h"

class GshhsDwnload: public QObject, public inetClient
{ Q_OBJECT
    public:
        GshhsDwnload(myCentralWidget * centralWidget,inetConnexion * inet);

        void requestFinished(QByteArray res);
        QString getAuthLogin(bool * ok=NULL) { return inetClient::getAuthLogin(ok);}
        QString getAuthPass(bool * ok=NULL) { return inetClient::getAuthPass(ok); }
        void authFailed(void) { inetClient::authFailed(); }
        void inetError(void) { inetClient::inetError(); }

        void getMaps(void);

    public slots:
        void slot_abort(void);

    private:
        myCentralWidget * centralWidget;

};

#endif // GSHHSDWNLOAD_H
