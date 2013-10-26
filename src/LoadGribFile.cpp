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

#include <QApplication>
#include <QTextStream>
#include <QDomDocument>
#include <QStringList>

#include <cassert>

#include "LoadGribFile.h"
#include "Util.h"
#include "Version.h"
#include "libs/sha1/sha1.h"
#include <QMessageBox>
#include <QDebug>

// zyGrib dwn n'utilise pas les classes std

//-------------------------------------------------------------------------------
LoadGribFile::LoadGribFile()
{
    step = 0;

    zygriblog = "a07622b82b18524d2088c9b272bb3feeb0eb1737";
    zygribpwd = "61c9b2b17db77a27841bbeeabff923448b0f6388";

    inetManager = new QNetworkAccessManager(this);
    step1_InetReply=step2_InetReply=step3_InetReply=step_checkVersion=NULL;
    assert(inetManager);

    host = "http://www.zygrib.org";
    Util::paramProxy(inetManager,host);

    connect(inetManager, SIGNAL(finished ( QNetworkReply *)),
            this, SLOT(requestFinished (QNetworkReply *)));
}
//-------------------------------------------------------------------------------
LoadGribFile::~LoadGribFile () {
    if (inetManager != NULL)
        delete inetManager;
}

//-------------------------------------------------------------------------------
void LoadGribFile::stop () {
    if(step1_InetReply)
        step1_InetReply->abort();
    if(step2_InetReply)
        step2_InetReply->abort();
    if(step3_InetReply)
        step3_InetReply->abort();
}

//-------------------------------------------------------------------------------
void LoadGribFile::getGribFile(
        double x0, double y0, double x1, double y1,
        double resolution, int interval, int days,
        bool wind, bool pressure, bool rain,
        bool cloud, bool temp, bool humid, bool isotherm0,
                bool tempPot, bool tempMin, bool tempMax,
                bool snowCateg, bool frzRainCateg,
                bool CAPEsfc,bool CINsfc,
                bool altitudeData200,
                bool altitudeData300,
                bool altitudeData500,
                bool altitudeData700,
                bool altitudeData850)
{
    QString page;

    //----------------------------------------------------------------
    // Etape 1 : Demande la creation du fichier Grib (nom en retour)
    //----------------------------------------------------------------

    QString parameters = "";
    if (wind) {
        parameters += "W;";
    }
    if (pressure) {
        parameters += "P;";
    }
    if (rain) {
        parameters += "R;";
    }
    if (cloud) {
        parameters += "C;";
    }
    if (temp) {
        parameters += "T;";
    }
    if (humid) {
        parameters += "H;";
    }
    if (isotherm0) {
        parameters += "I;";
    }
    if (tempPot) {
        parameters += "t;";
    }
    if (tempMin) {
        parameters += "m;";
    }
    if (tempMax) {
        parameters += "M;";
    }    
    if (snowCateg) {
        parameters += "s;";
    }
    if (frzRainCateg) {
        parameters += "Z;";
    }
    if (CAPEsfc) {
        parameters += "c";
    }
    if (CINsfc)
        parameters += "i;";

    if (altitudeData200) parameters += "2;";
    if (altitudeData300) parameters += "3;";
    if (altitudeData500) parameters += "5;";
    if (altitudeData700) parameters += "7;";
    if (altitudeData850) parameters += "8;";

//parameters += "r";	// PRATE

    if (parameters != "")
    {
        Util::paramProxy(inetManager,host);

        step = 1;
        emit signalGribSendMessage(tr("Preparation du fichier sur le serveur"));
        QTextStream(&page)
                << host
                << "/noaa/getzygribfile3.php?"
                << "but=prepfile"
                << "&la1=" << floor(y0)
                << "&la2=" << ceil(y1)
                << "&lo1=" << floor(x0)
                << "&lo2=" << ceil(x1)
                << "&res=" << resolution
                << "&hrs=" << interval
                << "&jrs=" << days
                << "&par=" << parameters
                << "&l=" << zygriblog
                << "&m=" << zygribpwd
                << "&runGFS=last"
                << "&client=" << "zyGrib-3.9.2"
                ;
        // missing param compared to zygrib: runGFS

        //qWarning() << "zygrib request: " << page;

        step1_InetReply=step2_InetReply=step3_InetReply=step_checkVersion=NULL;
        QNetworkRequest request;
        request.setUrl(QUrl(page));
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
        Util::addAgent(request);
        step1_InetReply=inetManager->get(request);
    }

    // Suite de la sequence de recuperation dans requestFinished()
}

//-------------------------------------------------------------------------------
void LoadGribFile::requestFinished ( QNetworkReply* inetReply)
{
    QString page;
    if (inetReply->error() != QNetworkReply::NoError) {
        qWarning()<<"inetReply error from LoadGribFile";
        if(inetReply==step_checkVersion) return;
        emit signalGribLoadError(QString("Http error: %1 (step=%2)").arg(inetReply->error()).arg(step));
    }
    else if(inetReply == step_checkVersion)
    {
        QString strbuf=inetReply->readAll();
        QString vers=QTVLM_VERSION_NUM;
        vers.append(".");
        vers.append(QTVLM_SUB_VERSION_NUM);
        vers.remove("+");
        if(vers.contains("beta") && strbuf=="3.4.2") return;
        if(vers.left(strbuf.size())!=strbuf)
        {
            QString m=tr("Vous n'utilisez pas la derniere version de qtVlm: ")+strbuf;
#ifdef __WIN_QTVLM
            m+="<br>"+tr("Emplacement:")+" <a href='http://www.virtual-winds.org/oxygen'>"+tr("qtVlm downloads")+"</a>";
#endif
#ifdef __MAC_QTVLM
            m+="<br>"+tr("Emplacement:")+" <a href='http://www.virtual-winds.org/oxygen/mac'>"+tr("qtVlm downloads")+"</a>";
#endif
            QMessageBox::warning (0,
                tr("qtVlm version"),
                m);
        }

    }
    else if(inetReply == step1_InetReply)
    {
        //-------------------------------------------
        // Retour de l'etape 1 : preparation du fichier
        //-------------------------------------------
        QString strbuf = inetReply->readAll();
        QStringList lsbuf = strbuf.split("\n");

/****
status:ok
params:par=WPRT&lo1=-14.8&lo2=6.3&la1=51.9&la2=40.8&jrs=5&hrs=6&res=1
file:20080504_165912_286.grb
size:50694
checksum:8685dcce3aef8dd717a7963c0f1e44f10aef6267
gfs_run_date:20080504
gfs_run_hour:6
***********/
        QString status;

        for (int i=0; i < lsbuf.size(); i++)
        {
            QStringList lsval = lsbuf.at(i).split(":");
            if (lsval.size() == 2) {
                if (lsval.at(0) == "status")
                    status = lsval.at(1);
                else if (lsval.at(0) == "file") {
                    fileName = QString(lsval.at(1)).replace(".grb","%20");
                    //qWarning() << "zygrib fname= " << lsval.at(1);
                }
                else if (lsval.at(0) == "size")
                    fileSize = lsval.at(1).toInt();
                else if (lsval.at(0) == "checksum")
                    checkSumSHA1 = lsval.at(1);
                else if(lsval.at(0) == "message") {
                    QString m = QUrl::fromPercentEncoding (lsval.at(1).toUtf8());
                    qWarning() << m;
                }

            }
        }

        //--------------------------------------------------
        // Etape 2 : Demande le contenu du fichier Grib
        //--------------------------------------------------
        if (status == "ok") {
            step = 2;
            emit signalGribSendMessage(tr("GetFileContent"));
            QString s;
            s = tr("Taille totale : ") + s.sprintf("%d",fileSize/1024) + " ko";
            emit signalGribSendMessage(s);
            emit signalGribStartLoadData();

            QTextStream(&page)
                           << host
                           << "/noaa/313O562/"+fileName;
            //printf("PAGE='%s'\n",qPrintable(page));

            step1_InetReply=NULL;
            QNetworkRequest request;
            request.setUrl(QUrl(page));
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
            Util::addAgent(request);
            step2_InetReply=inetManager->get(request);
            connect(step2_InetReply,SIGNAL(downloadProgress(qint64,qint64)),this,SIGNAL(progress(qint64,qint64)));
        }
        else {
            emit signalGribLoadError(tr("Pas de fichier cree sur le serveur:")+status);
        }
    }
    else if(inetReply == step2_InetReply)
    {
        //--------------------------------------------------
        // Reçu le contenu du fichier Grib
        //--------------------------------------------------
        arrayContent = inetReply->readAll();
        //--------------------------------------------------
        // Verifie le checksum
        //--------------------------------------------------
        emit signalGribSendMessage(tr("CheckSum control"));
        SHA1 sha1;
        unsigned char * digest;
        sha1.addBytes( arrayContent.data(), arrayContent.size() );
        digest = sha1.getDigest();
        assert( digest );
        QString s, strsha1 = "";
        for (int i=0; i<20; i++) {
            strsha1 += s.sprintf("%02x", digest[i]);
        }
        free(digest);
        if (strsha1 == checkSumSHA1)
        {
            //--------------------------------------------------
            // Signale la fin du telechargement
            //--------------------------------------------------
            emit signalGribDataReceived(&arrayContent, fileName.replace("%20",".grb"));
            emit signalGribSendMessage(tr("Termine"));
            emit clearSelection();
        }
        else {
            emit signalGribLoadError(tr("Checksum incorrect."));
        }
    }
    else if(inetReply == step3_InetReply)
    {
        arrayContent = inetReply->readAll();
        QString content= arrayContent;
        QMessageBox::information (0,
            tr("Informations sur le serveur zyGrib"),
            content);
        emit ungrayButtons();
    }
}
void LoadGribFile::getServerStatus()
{
    Util::paramProxy(inetManager,host);
    QNetworkRequest request;
    request.setUrl(QUrl(host+"/noaa/getServerStatus.php"));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
    Util::addAgent(request);
    step3_InetReply=inetManager->get(request);
}
void LoadGribFile::checkQtvlmVersion()
{
    Util::paramProxy(inetManager,host);
    QNetworkRequest request;
    request.setUrl(QUrl("http://www.virtual-winds.org/oxygen/getLastVersion.php"));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,QNetworkRequest::AlwaysNetwork);
    Util::addAgent(request);
    step_checkVersion=inetManager->get(request);
}

