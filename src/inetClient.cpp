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

#include <parser.h>
#include <QMessageBox>
#include <QDateTime>

#include "inetClient.h"
#include "inetConnexion.h"
#include "Version.h"

inetClient::inetClient(inetConnexion * inet)
{
    myReply = NULL;
    currentRequest=-1;
    hasProgress=false;
    needAuth=false;
    this->inet=inet;
    this->nbAuth=0;
    this->name="Unknown";
}

void inetClient::inetGet(int currentRequest,QString requestUrl)
{
    inetGet(currentRequest,requestUrl,QString());
}

void inetClient::inetGetProgress(int currentRequest,QString requestUrl)
{
    inetGetProgress(currentRequest,requestUrl,QString());
}

void inetClient::inetPost(int currentRequest,QString requestUrl,QString data)
{
    inetPost(currentRequest,requestUrl,data,QString());
}

void inetClient::inetGet(int currentRequest,QString requestUrl,QString host)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestGet(this,requestUrl,host);
    }
}

void inetClient::inetGetProgress(int currentRequest,QString requestUrl,QString host)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestGetProgress(this,requestUrl,host);
    }
}

void inetClient::inetPost(int currentRequest,QString requestUrl,QString data,QString host)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestPost(this,requestUrl,data,host);
    }
}

void inetClient::resetReply()
{
    if(myReply)
    {
        myReply->close();
        myReply->deleteLater();
        myReply=NULL;
        this->nbAuth=0;
    }
}

bool inetClient::checkWSResult(QByteArray res,QString caller,QWidget * parent,QString order)
{
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (res, &ok).toMap();
    if (!ok) {
        qWarning() << "Error parsing json data " << res;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        return false;
    }

    if(result["success"].toBool())
    {
        return true;
    }
    else
    {
        QVariantMap errorData = result["error"].toMap();
        qWarning() << "Error doing " << caller << " cmd: code=" << errorData["code"].toString()
                << " - message=" << errorData["msg"].toString();
        QMessageBox::critical(parent,QObject::tr("Erreur de communication avec VLM ds ")+caller,
                              QObject::tr("Reporter l'erreur au dev qtVlm") + " (H= " + QDateTime::currentDateTime().toUTC().toString() + ")\n"
                              + QObject::tr("Version: ") + Version::getVersion() + " - build on: " + Version::getDate() + "\n"
                              + QObject::tr("Zone: ") + caller + "\n"
                              + QObject::tr("Code erreur:")+errorData["code"].toString() + "\n"
                              + QObject::tr("Msg erreur:")+errorData["msg"].toString() + "\n"
                              + QObject::tr("Complement: ")+errorData["custom_error_string"].toString() + "\n"
                              + QObject::tr("Ordre:")+order
                              );
        return false;
    }
}
