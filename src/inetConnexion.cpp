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

inetConnexion::inetConnexion(QWidget * main,QWidget * parent) : QWidget(parent)
{
    hasSpecHost=false;
    initConn(main,parent);
}

inetConnexion::inetConnexion(QString specHost,QWidget * main,QWidget * parent) : QWidget(parent)
{
    if(specHost.isEmpty())
        hasSpecHost=false;
    else
    {
        hasSpecHost=true;
        host=specHost;
    }
    initConn(main,parent);
}

void inetConnexion::initConn(QWidget * main,QWidget * parent)
{
    this->parent=parent;

    inetManager = new QNetworkAccessManager(this);
    connect(this,SIGNAL(requestFinished(int,QByteArray)),parent,SLOT(requestFinished(int,QByteArray)));
    connect(main,SIGNAL(updateInet()),this,SLOT(updateInet()));
    currentReply=NULL;
    updateInet();
    progressDialog=new inetConn_progressDialog();
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
        qWarning() << "Update inet for " << parent;

        if(!hasSpecHost)
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
    hasProgress=false;
    doRequest(REQUEST_GET,requestNum,requestUrl,QString());
}

void inetConnexion::doRequestGetProgress(int requestNum,QString requestUrl)
{
    hasProgress=true;
    progressDialog->showDialog(host+requestUrl);
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
    connect(currentReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(slotProgess(qint64,qint64)));
}

void inetConnexion::slotFinished()
{
    progressDialog->hide();
    if(currentReply && hasRequest)
    {
        if (currentReply->error() != QNetworkReply::NoError) {
            qWarning() << "Error doing inetGet (2):" << currentReply->error() << " - " << currentReply->errorString();
            resetInet();
        }
        else
        {
            //qWarning() << "Received : " << currentReply->size();
            QByteArray res=currentReply->readAll();
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
    progressDialog->hide();
    resetInet();
}

void inetConnexion::slotProgess(qint64 bytesReceived, qint64 bytesTotal )
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

