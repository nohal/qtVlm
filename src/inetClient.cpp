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

#ifdef QT_V5
#include <QtCore/QJsonDocument>
#else
#include <parser.h>
#endif

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
    this->inet=inet;
    this->nbAuth=0;
    this->name="Unknown";
}

void inetClient::inetGet(int currentRequest,QString requestUrl, bool needAuth)
{
    inetGet(currentRequest,requestUrl,QString(),needAuth);
}

void inetClient::inetGetProgress(int currentRequest,QString requestUrl, bool needAuth)
{
    inetGetProgress(currentRequest,requestUrl,QString(),needAuth);
}

void inetClient::inetPost(int currentRequest,QString requestUrl,QString data, bool needAuth)
{
    inetPost(currentRequest,requestUrl,data,QString(),needAuth);
}

void inetClient::inetGet(int currentRequest,QString requestUrl,QString host, bool needAuth)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestGet(this,requestUrl,host,needAuth);
    }
    else
        qWarning() << "Doing inetGet with no inet available";
}

void inetClient::inetGetProgress(int currentRequest,QString requestUrl,QString host, bool needAuth)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestGetProgress(this,requestUrl,host,needAuth);
    }
    else
        qWarning() << "Doing inetGetProgress with no inet available";
}

void inetClient::inetPost(int currentRequest,QString requestUrl,QString data,QString host, bool needAuth)
{
    if(inet)
    {
        this->currentRequest=currentRequest;
        this->nbAuth=0;
        inet->doRequestPost(this,requestUrl,data,host,needAuth);
    }
    else
        qWarning() << "Doing inetPost with no inet available";
}
void inetClient::inetAbort()
{
    if(myReply)
    {
        myReply->abort();
        resetReply();

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

bool inetClient::JSON_to_map(QByteArray buf,QVariantMap *map) {
    if(!map) return false;
#ifdef QT_V5
    QJsonParseError err;
    QVariant v = QJsonDocument::fromJson(buf,&err).toVariant();
    if(err.error!=0) {
        qWarning() << "Error parsing json data";
        qWarning() << "Error: " << err.errorString() << " - err num= " << err.error;
        qWarning() << "Json buffer:\n" << buf;
        QMessageBox::critical (NULL,
                               QObject::tr("Erreur"),
                               QObject::tr("Erreur de lecture json."));
        *map=QVariantMap();
        return false;
    }
    *map = v.toMap();
#else
    QJson::Parser parser;
    bool ok;
    *map = parser.parse (buf, &ok).toMap();
    if (!ok) {
        qWarning() << "Error parsing json data in " << name;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        qWarning() << "Json buffer:\n" << buf;
        QMessageBox::critical (NULL,
                               QObject::tr("Erreur"),
                               QObject::tr("Erreur de lecture json."));
        *map=QVariantMap();
        return false;
    }
#endif
    return true;
}

bool inetClient::JSON_to_list(QByteArray buf,QList<QVariant> *list) {
    if(!list) return false;
#ifdef QT_V5
    QJsonParseError err;
    QVariant v = QJsonDocument::fromJson(buf,&err).toVariant();
    if(err.error!=0) {
        qWarning() << "Error parsing json data";
        qWarning() << "Error: " << err.errorString() << " - err num= " << err.error;
        qWarning() << "Json buffer:\n" << buf;
        QMessageBox::critical (NULL,
                               QObject::tr("Erreur"),
                               QObject::tr("Erreur de lecture json."));
        *list=QList<QVariant>();
        return false;
    }
    *list = v.toList();
#else
    QJson::Parser parser;
    bool ok;
    *list = parser.parse (buf, &ok).toList();
    if (!ok) {
        qWarning() << "Error parsing json data in " << name;
        qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        qWarning() << "Json buffer:\n" << buf;
        QMessageBox::critical (NULL,
                               QObject::tr("Erreur"),
                               QObject::tr("Erreur de lecture json."));
        *list=QList<QVariant>();
        return false;
    }
#endif
    return true;
}

bool inetClient::map_to_JSON( QVariantMap map,QByteArray * json) {
    if(!json) return false;
#ifdef QT_V5
    QVariant v(map);
    *json = QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);
#else
    QJson::Serializer serializer;
    *json = serializer.serialize(map);
#endif
    return true;
}


bool inetClient::checkWSResult(QByteArray res,QString caller,QWidget * parent,QString order)
{
    QVariantMap result;
    if(!inetClient::JSON_to_map(res,&result))
        return false;

    if(result["success"].toBool())
    {
        return true;
    }
    else
    {
        QVariantMap errorData = result["error"].toMap();
//#warning Radiation de boatsit: a remplacer par le code erreur ad-hoc qd paparazia aura fait la modif necessaire
        if(errorData["code"].toString()=="XXXXXX") return true; /*cas de la radiation de boatsit*/
        qWarning() << "Error doing " << caller << " cmd: code=" << errorData["code"].toString()
                << " - message=" << errorData["msg"].toString();
        if(caller=="OppList_getTrack") return false; /*cas du opp qui a fini la course entre getranking et gettrack*/
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
