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

#include <QNetworkCookieJar>
#include <QMessageBox>

#include "inetConnexion.h"

#include "inetClient.h"
#include "DialogInetProgess.h"
#include "Util.h"
#include "dataDef.h"

#define REQUEST_GET  0
#define REQUEST_POST 1

inetConnexion::inetConnexion(QWidget * main)
{
    inetManager = new QNetworkAccessManager(this);

    connect(main,SIGNAL(updateInet()),this,SLOT(slot_updateInet()));
    connect(inetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(slot_requestFinished(QNetworkReply*)));
    connect(inetManager,SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this,SLOT(slot_authRequired(QNetworkReply*,QAuthenticator*)));

    slot_updateInet();
    progressDialog=new DialogInetProgess(main);
    hasProgress=false;
}

inetConnexion::~inetConnexion(void)
{
    resetInet();
    if(inetManager)
        delete inetManager;
}

void inetConnexion::slot_updateInet(void)
{
    if(inetManager)
        resetInet();
}

bool inetConnexion::isAvailable(void)
{
    return (inetManager!=NULL);
}


void inetConnexion::resetInet(void)
{
    while(replyList.count() > 0)
    {
        inetClient * ptr = replyList.takeFirst();
        ptr->resetReply();
        ptr->clearCurrentRequest();
    }
    replyList.clear();
}

void inetConnexion::doRequestGet(inetClient* client,QString requestUrl,QString host,bool needAuth)
{
    hasProgress=false;
    doRequest(REQUEST_GET,client,requestUrl,QString(),host,needAuth);
}

void inetConnexion::doRequestGetProgress(inetClient* client,QString requestUrl,QString host,bool needAuth)
{
    if(hasProgress)
    {
        qWarning() << "Already running request with progress";
        doRequest(REQUEST_GET,client,requestUrl,QString(),host,needAuth);
    }
    else
    {
        hasProgress=true;
        progressDialog->showDialog(host+requestUrl);
        doRequest(REQUEST_GET,client,requestUrl,QString(),host,needAuth);
    }
}

void inetConnexion::doRequestPost(inetClient* client,QString requestUrl,QString data,QString host,bool needAuth)
{
    doRequest(REQUEST_POST,client,requestUrl,data,host,needAuth);
}

void inetConnexion::doRequest(int type,inetClient* client,QString requestUrl,QString data,QString host,bool needAuth)
{
    QString page;
    QNetworkReply * currentReply;

    if(host.isEmpty())
        host=Util::getHost();
    Util::paramProxy(inetManager,host);

    page = host+requestUrl;
    if(type == REQUEST_POST)
        page += "?" + data;
#if 0
    //if(page.contains("track"))
        qWarning() << "Doing inet request: " << page ;
#endif
    QNetworkRequest request;
    request.setUrl(QUrl(page));
    Util::addAgent(request);

    client->set_currentNeedAuth(needAuth);

    if(needAuth)
    {         
        QString concatenated = client->getAuthLogin() + ":" + client->getAuthPass();
        //qWarning() << "Adding auth data: " <<concatenated ;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());
        //qWarning() << "Adding auth data: " <<concatenated << " - " << headerData;
//        if(inetManager->cookieJar())
//            delete inetManager->cookieJar();
        inetManager->setCookieJar(new QNetworkCookieJar());
        //inetManager->cookieJar()->setParent(0);

    }

    if(type==REQUEST_POST)
    {
        //qWarning() << "Posting data: " << data;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
        currentReply=inetManager->post(request,"");
    }
    else
    {
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
        currentReply=inetManager->get(request);
    }

    replyList.push_back(client);
    client->setReply(currentReply);

    if(hasProgress)
    {
        client->setHasProgress(true);
        connect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(slot_progess(qint64,qint64)));
    }

    //qWarning() << "inetConn::doRequest FINISHED";

    //replyList.push_back(client);
}

void inetConnexion::slot_requestFinished(QNetworkReply * currentReply)
{
    /* Recherche du client correspondant � currentReply */
    bool found=false;
    inetClient * currentClient;
    QListIterator<inetClient*> i (replyList);

    //qWarning() << "Get reply";

    while(!found && i.hasNext())
    {        
        inetClient * ptr = i.next();
        if(ptr->getReply() == currentReply)
        {
            found=true;
            currentClient = ptr;
            replyList.removeAll(ptr);
            break;
        }
    }

    if(found)
    {
        /* managing progress bar */
        if(currentClient->getHasProgress())
        {
            qWarning() << "Removing dwnld progress";
            currentClient->setHasProgress(false);
            progressDialog->hide();
            disconnect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(slot_progess(qint64,qint64)));
        }

        if (currentReply->error() != QNetworkReply::NoError) {
            qWarning() << "Error doing inetGet (2):" << currentReply->error() << " - " << currentReply->errorString();
            if(currentReply->error()==3)
                QMessageBox::warning(0,QObject::tr("Erreur internet"),
                              QObject::tr("Serveur VLM inaccessible"));
            else if (currentReply->error()!=QNetworkReply::OperationCanceledError)
                QMessageBox::warning(0,QObject::tr("Erreur internet"),
                              currentReply->errorString());
            currentClient->inetError();
            currentClient->resetReply();
            currentClient->clearCurrentRequest();
            emit errorDuringGet();
        }
        else
        {
            QByteArray res=currentReply->readAll();
            //qWarning() << "res=" <<  res;
            currentClient->resetReply();
            currentClient->requestFinished(res);            
        }
    }
    else
    {
        qWarning() << "Client of request not found => Not processing reply (url=" << currentReply->url() << ")";
        currentReply->close();
        currentReply->deleteLater();
    }
}

void inetConnexion::slot_authRequired(QNetworkReply* currentReply,QAuthenticator* /*auth*/)
{
    /* Recherche du client correspondant � currentReply */
    bool found=false;
    inetClient * currentClient;
    QListIterator<inetClient*> i (replyList);

    while(!found && i.hasNext())
    {
        inetClient * ptr = i.next();
        if(ptr->getReply() == currentReply)
        {
            found=true;
            currentClient = ptr;
            //replyList.removeAll(ptr);
            break;
        }
    }

    if(found)
    {
        if(currentClient->get_currentNeedAuth())
        {
            qWarning() << "Auth failed for " << currentClient->getAuthLogin() << " " << currentClient->getAuthPass() <<
                    " reqType=" << currentClient->getCurrentRequest();
            currentClient->authFailed();
        }
    }
}

void inetConnexion::slot_progess(qint64 bytesReceived, qint64 bytesTotal )
{
    QNetworkReply* currentReply = (QNetworkReply*)sender();
    if(!currentReply) {
        qWarning() << "Draw progress - can't get sender";
        return;
    }
    inetClient * currentClient=NULL;
    QListIterator<inetClient*> i (replyList);

    bool found=false;

    while(!found && i.hasNext())
    {
        inetClient * ptr = i.next();
        if(ptr->getReply() == currentReply)
        {
            found=true;
            currentClient = ptr;
            //replyList.removeAll(ptr);
            break;
        }
    }

    if(!found) {
        qWarning() << "Draw progress - can't found client";
        return;
    }

    if(currentClient->getHasProgress()) {
        progressDialog->updateProgress(bytesReceived,bytesTotal);
        //qWarning() << "Progress " << bytesReceived << "/" << bytesTotal;
    }
}
