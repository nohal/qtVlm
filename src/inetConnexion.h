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
***********************************************************************/

#ifndef INET_CONNEXION_H
#define INET_CONNEXION_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>
#include <QDebug>

#include "Util.h"

class inetConnexion : public QWidget
{ Q_OBJECT
    public:
        inetConnexion(QWidget * parent);
        ~inetConnexion(void);
        void doRequestGet(int requestNum,QString requestUrl);
        void doRequestPost(int requestNum,QString requestUrl,QString data);
        bool isAvailable(void);
        void updateInet(void);

    public slots:
        void slotFinished();
        void slotError(QNetworkReply::NetworkError error);

    signals:
        void requestFinished(int,QString);

    private:
        QString host;
        bool hasRequest;
        int currentRequest;
        QNetworkAccessManager *inetManager;
        QNetworkReply * currentReply;


        void resetInet(void);
        void doRequest(int type,int requestNum,QString requestUrl,QString data);
};

#endif
