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
    progressDialog=new inetConn_progressDialog();
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
    {
        /*if(!hasSpecHost)
            host=Util::getHost();
        Util::paramProxy(inetManager,host);*/
        resetInet();
    }
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

void inetConnexion::doRequestGet(inetClient* client,QString requestUrl,QString host)
{
    hasProgress=false;
    doRequest(REQUEST_GET,client,requestUrl,QString(),host);
}

void inetConnexion::doRequestGetProgress(inetClient* client,QString requestUrl,QString host)
{
    if(hasProgress)
    {
        qWarning() << "Already running request with progress";
        doRequest(REQUEST_GET,client,requestUrl,QString(),host);

    }
    else
    {
        hasProgress=true;
        progressDialog->showDialog(host+requestUrl);
        doRequest(REQUEST_GET,client,requestUrl,QString(),host);
    }
}

void inetConnexion::doRequestPost(inetClient* client,QString requestUrl,QString data,QString host)
{
    doRequest(REQUEST_POST,client,requestUrl,data,host);
}

void inetConnexion::doRequest(int type,inetClient* client,QString requestUrl,QString data,QString host)
{
    QString page;
    QNetworkReply * currentReply;

    if(host.isEmpty())
        host=Util::getHost();
    Util::paramProxy(inetManager,host);

    page=host+requestUrl;
#if 0
    qWarning() << "Doing inet request: " << page ;
#endif
    QNetworkRequest request;
    request.setUrl(QUrl(page));
    Util::addAgent(request);

    if(client->getNeedAuth())
    {         
        QString concatenated = client->getAuthLogin() + ":" + client->getAuthPass();
        //qWarning() << "Adding auth data: " <<concatenated ;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = "Basic " + data;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());
        //qWarning() << "Adding auth data: " <<concatenated << " - " << headerData;
        //delete inetManager->cookieJar();
        inetManager->setCookieJar(new QNetworkCookieJar());
    }

    if(type==REQUEST_POST)
    {
        //qWarning() << "Posting data: " << data;
        currentReply=inetManager->post(request,data.toAscii());
    }
    else
        currentReply=inetManager->get(request);

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
            currentClient->setHasProgress(false);
            progressDialog->hide();
            disconnect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(slot_progess(qint64,qint64)));
        }

        if (currentReply->error() != QNetworkReply::NoError) {
            qWarning() << "Error doing inetGet (2):" << currentReply->error() << " - " << currentReply->errorString();
            if(currentReply->error()==3)
                QMessageBox::warning(0,QObject::tr("Erreur internet"),
                              QObject::tr("Serveur VLM inaccessible"));
            currentClient->inetError();
            currentClient->resetReply();
            currentClient->clearCurrentRequest();
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
        if(currentClient->getNeedAuth())
        {
            qWarning() << "Auth failed for " << currentClient->getAuthLogin() << " " << currentClient->getAuthPass() <<
                    " reqType=" << currentClient->getCurrentRequest();
            currentClient->authFailed();
        }
    }
}

void inetConnexion::slot_progess(qint64 bytesReceived, qint64 bytesTotal )
{
    if(hasProgress)
        progressDialog->updateProgress(bytesReceived,bytesTotal);
}

/*********************************************
  Progress bar
  *******************************************/

inetConn_progressDialog::inetConn_progressDialog(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
}

void inetConn_progressDialog::showDialog(QString name)
{
    fileName->setText(name);
    progress->reset();
    progress_txt->setText("0/0 Kb");
    setWindowModality(Qt::ApplicationModal);
    show();
}
void inetConn_progressDialog::hideDialog()
{
    if(this->isVisible()) hide();
}

void inetConn_progressDialog::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QString type;
    QString str;
    double received,total;
    if(bytesTotal<1024)
    {
        type="b";
        received=((double)bytesReceived);
        total=((double)bytesTotal);
    }
    else if(bytesTotal<1024*1024)
    {
        type="Kb";
        received=((double)bytesReceived)/1024.0;
        total=((double)bytesTotal)/1024.0;
    }
    else
    {
        type="Mb";
        received=((double)bytesReceived)/1024.0/1024.0;
        total=((double)bytesTotal)/1024.0/1024.0;
    }

    progress->setMinimum(0);
    progress->setMaximum((int)bytesTotal);
    progress->setValue((int)bytesReceived);
    str.sprintf("%.2f/%.2f ",received,total);
    progress_txt->setText(str+type);
}

