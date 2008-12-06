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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

/*************************

Download GRIB File on zygrib.free.fr


*************************/

#ifndef LOADGRIBFILE_H
#define LOADGRIBFILE_H

#include <QObject>
#include <QBuffer>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>

class LoadGribFile : public QObject
{ Q_OBJECT
    public:
        LoadGribFile();
        ~LoadGribFile();

        void getGribFile(
                    float x0, float y0, float x1, float y1,
                    float resolution, int interval, int days,
                    bool wind, bool pressure, bool rain, bool cloud, bool temp, bool humid);
        void stop();

    private:
        QNetworkAccessManager *inetManager;
        QNetworkProxy * inetProxy;
        QNetworkReply * step1_InetReply;
        QNetworkReply * step2_InetReply;

        QString host;
        QByteArray arrayContent;

        //int     idFileName;    // id des requêtes
        int     idGribContent;

        QString GribContent;
        QString fileName;
        QString checkSumSHA1;
        int     step;
        int     fileSize;

        QString zygriblog;
        QString zygribpwd;

        //QBuffer    ioBuffer;

    public slots:
        void requestFinished (QNetworkReply *);
        void dataReadProgress (qint64 done, qint64 total );

    signals:
        void signalGribDataReceived(QByteArray *content, QString);
        void signalGribReadProgress(int step, int done, int total);
        void signalGribSendMessage(QString msg);
        void signalGribStartLoadData();
        void signalGribLoadError(QString msg);
};


#endif
