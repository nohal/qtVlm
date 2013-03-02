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
#include <QAuthenticator>
#include <QWidget>
#include <QDialog>
#include <QDebug>

#include "class_list.h"

class inetConnexion : public QObject
{
    Q_OBJECT
    public:
        inetConnexion(QWidget * main);
        DialogInetProgess * getProgressDialog() {return progressDialog;}
        ~inetConnexion(void);

        void doRequestGet(inetClient* client,QString requestUrl,bool needAuth);
        void doRequestGetProgress(inetClient* client,QString requestUrl,bool needAuth);
        void doRequestPost(inetClient* client,QString requestUrl,QString data,bool needAuth);

        void doRequestGet(inetClient* client,QString requestUrl, QString host,bool needAuth);
        void doRequestGetProgress(inetClient* client,QString requestUrl, QString host,bool needAuth);
        void doRequestPost(inetClient* client,QString requestUrl,QString data, QString host,bool needAuth);

        bool isAvailable(void);

    signals:
        void errorDuringGet();
    public slots:
        void slot_progess(qint64 bytesReceived, qint64 bytesTotal );
        void slot_updateInet(void);
        void slot_requestFinished(QNetworkReply * currentReply);
        void slot_authRequired(QNetworkReply*,QAuthenticator*);

    private:
        bool hasProgress;
	QNetworkAccessManager *inetManager;

        DialogInetProgess * progressDialog;

	void resetInet(void);
        void doRequest(int type,inetClient* client,QString requestUrl,QString data, QString host, bool needAuth);

        QList<inetClient*> replyList;
};
Q_DECLARE_TYPEINFO(inetConnexion,Q_MOVABLE_TYPE);

#endif
