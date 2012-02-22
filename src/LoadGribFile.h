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

#include <stdlib.h>

#include <QObject>
#include <QBuffer>

#include <QNetworkAccessManager>
#include <QNetworkReply>

class LoadGribFile : public QObject
{ Q_OBJECT
    public:
        LoadGribFile();
        ~LoadGribFile();

        void getGribFile(
                    double x0, double y0, double x1, double y1,
                    double resolution, int interval, int days,
                    bool wind, bool pressure, bool rain,
                    bool cloud, bool temp, bool humid, bool isotherm0,
                    bool tempPot, bool tempMin, bool tempMax, bool snowDepth,
                    bool snowCateg, bool frzRainCateg,
                    bool CAPEsfc,
                    bool altitudeData200,
                    bool altitudeData300,
                    bool altitudeData500,
                    bool altitudeData700,
                    bool altitudeData850);
        void stop();
        void getServerStatus();
        void checkQtvlmVersion();

    private:
        QNetworkAccessManager *inetManager;

        QNetworkReply * step1_InetReply;
        QNetworkReply * step2_InetReply;
        QNetworkReply * step3_InetReply;
        QNetworkReply * step_checkVersion;

        QString host;
        QByteArray arrayContent;

        QString GribContent;
        QString fileName;
        QString checkSumSHA1;
        int     step;
        int     fileSize;

        QString zygriblog;
        QString zygribpwd;

    public slots:
        void requestFinished (QNetworkReply*);

    signals:
        void signalGribDataReceived(QByteArray *content, QString);
        void signalGribSendMessage(QString msg);
        void signalGribStartLoadData();
        void signalGribLoadError(QString msg);
        void ungrayButtons();
        void progress(qint64,qint64);
};


#endif
