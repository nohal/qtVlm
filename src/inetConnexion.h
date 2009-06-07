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
#include <QDialog>
#include <QDebug>

#include "Util.h"

#include "ui_inetConn_progessDialog.h"

class inetConn_progressDialog : public QDialog, public Ui::inetConn_progressDialog_ui
{
    Q_OBJECT
    public:
	inetConn_progressDialog(QWidget * parent = 0);
	void showDialog(QString name);

    public slots:
	void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
};

class inetConnexion : public QWidget
{ Q_OBJECT
    public:
        inetConnexion(QWidget * main,QWidget * parent);
        inetConnexion(QString specHost,QWidget * main,QWidget * parent);
	~inetConnexion(void);
        void initConn(QWidget * main,QWidget * parent);
	void doRequestGet(int requestNum,QString requestUrl);
	void doRequestGetProgress(int requestNum,QString requestUrl);
	void doRequestPost(int requestNum,QString requestUrl,QString data);
	bool isAvailable(void);	

    public slots:
	void slotFinished();
	void slotError(QNetworkReply::NetworkError error);
	void slotProgess(qint64 bytesReceived, qint64 bytesTotal );
        void updateInet(void);

    signals:
	void requestFinished(int,QByteArray);

    private:
	QString host;
	bool hasSpecHost;
	bool hasRequest;
	bool hasProgress;
	int currentRequest;
	QNetworkAccessManager *inetManager;
	QNetworkReply * currentReply;

	inetConn_progressDialog * progressDialog;

        QWidget * parent;

	void resetInet(void);
	void doRequest(int type,int requestNum,QString requestUrl,QString data);
};

#endif
