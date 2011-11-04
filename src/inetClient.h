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

#ifndef INETCLIENT_H
#define INETCLIENT_H

#include <QObject>
#include <QNetworkReply>

#include "class_list.h"

class inetClient
{
    public:
        inetClient(inetConnexion * inet);

        virtual void requestFinished(QByteArray res) { res.clear();}
        virtual QString getAuthLogin(bool * ok=NULL) {if(ok) *ok=false; return QString();}
        virtual QString getAuthPass(bool * ok=NULL) {if(ok) *ok=false; return QString();}
        virtual void authFailed(void) { clearCurrentRequest(); }
        virtual void inetError(void) { clearCurrentRequest(); }

        void resetReply();
        void clearCurrentRequest() { currentRequest=-1; hasProgress=false;}

        void setReply(QNetworkReply * myReply) { this->myReply = myReply; }
        QNetworkReply * getReply(void) { return myReply; }
        void setHasProgress(bool val) { hasProgress=val; }
        bool getHasProgress(void) { return hasProgress; }
        int  getCurrentRequest(void) { return currentRequest; }
        QString getName(){return this->name;}
        void setName(QString name){this->name=name;}
        bool getNeedAuth(void) {return needAuth;}

        inetConnexion * getInet(void) { return inet; }

        int nbAuth;

    protected:
       void inetGet(int currentRequest,QString url);
       void inetGetProgress(int currentRequest,QString requestUrl);
       void inetPost(int currentRequest,QString requestUrl,QString data);

       void inetGet(int currentRequest,QString url,QString host);
       void inetGetProgress(int currentRequest,QString requestUrl,QString host);
       void inetPost(int currentRequest,QString requestUrl,QString data,QString host);

       bool hasRequest(void) { return (myReply!=NULL); }
       bool hasInet(void) {return (inet!=NULL); }

       bool checkWSResult(QByteArray res,QString caller,QWidget * parent,QString order=QString());

       bool needAuth;
       void inetAbort(){myReply->abort();delete myReply;myReply=NULL;}

    private:
       int currentRequest;
       bool hasProgress;
       inetConnexion * inet;
       QNetworkReply * myReply;
       QString name;
};

#endif // INETCLIENT_H
