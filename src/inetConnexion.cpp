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

#include "inetConnexion.h"

#define REQUEST_GET  0
#define REQUEST_POST 1

inetConnexion::inetConnexion(QWidget * parent) : QWidget(parent)
{
    inetManager = new QNetworkAccessManager(this);
    connect(this,SIGNAL(requestFinished(int,QString)),parent,SLOT(requestFinished(int,QString)));
    currentReply=NULL;
    updateInet();
}

inetConnexion::~inetConnexion(void)
{
    resetInet();
    if(inetManager)
        delete inetManager;
}

void inetConnexion::updateInet(void)
{
    if(inetManager)
    {
        host=Util::getHost();
        Util::paramProxy(inetManager,host);
        resetInet();
    }
}

bool inetConnexion::isAvailable(void)
{
    return !hasRequest;
}

void inetConnexion::resetInet(void)
{
    hasRequest=false;
    if(currentReply)
    {
        currentReply->disconnect();
        currentReply->close();
        currentReply->deleteLater();
        currentReply=NULL;
    }
}
void inetConnexion::doRequestGet(int requestNum,QString requestUrl)
{
    doRequest(REQUEST_GET,requestNum,requestUrl,QString());
}

void inetConnexion::doRequestPost(int requestNum,QString requestUrl,QString data)
{
    doRequest(REQUEST_POST,requestNum,requestUrl,data);
}

void inetConnexion::doRequest(int type,int requestNum,QString requestUrl,QString data)
{
    QString page;

    resetInet();
    hasRequest=true;

    page=host+requestUrl;
    currentRequest=requestNum;

    //qWarning() << "boat Acc Doing request: " << page;

    QNetworkRequest request;
    request.setUrl(QUrl(page));
    Util::addAgent(request);

    if(type==REQUEST_POST)
        currentReply=inetManager->post(request,data.toAscii());
    else
        currentReply=inetManager->get(request);
    connect(currentReply, SIGNAL(finished()), this, SLOT(slotFinished()));
    connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void inetConnexion::slotFinished()
{
    if(currentReply && hasRequest)
    {
        if (currentReply->error() != QNetworkReply::NoError) {
            qWarning() << "Error doing inetGet (2):" << currentReply->error() << " - " << currentReply->errorString();
            resetInet();
        }
        else
        {
            QString res=currentReply->readAll();
            resetInet();
            emit requestFinished(currentRequest,res);
        }
    }
    else
    {
        qWarning() << "Not processing reply: currentReply = " << currentReply << ", hasRequest=" << hasRequest;
        resetInet();
    }
}

void inetConnexion::slotError(QNetworkReply::NetworkError error)
{
    qWarning() << "Error doing inetGet (1):" << error << " - " << (currentReply?currentReply->errorString():"");
    resetInet();
}

