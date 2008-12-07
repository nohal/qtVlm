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

#include <QMessageBox>
#include <cmath>
#include <cassert>

#include "DialogServerStatus.h"
#include "Util.h"

//-------------------------------------------------------------------------------
DialogServerStatus::DialogServerStatus() : QDialog()
{
    host = "www.zygrib.org";
    QString page = "/noaa/getGfsRunInfo.php";
    
    setWindowTitle(tr("Serveur"));
    QFrame *ftmp;
    QLabel *label;
    frameGui = createFrameGui(this);
    
    layout = new QGridLayout(this);
    int lig=0;
    //-------------------------
    lig ++;
    QFont font;
    font.setBold(true);
    label = new QLabel(tr("Statut du serveur de fichiers GRIB"), this);
    label->setFont(font);
    layout->addWidget( label,    lig,0, 1,-1, Qt::AlignCenter);
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    layout->addWidget( frameGui,  lig,0,   1, 2);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    btOK     = new QPushButton(tr("Ok"), this);
    layout->addWidget( btOK,    lig,0);
    
    //----------------------------------------
    http = new QHttp(this);
    assert(http);
    http->setHost(host);
 
    //===============================================================
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
    connect(http, SIGNAL(requestFinished (int, bool)),
            this, SLOT(requestFinished (int, bool)));
    //===============================================================
    timeLoad.start();
    ioBuffer.setBuffer(NULL);
    idHttpGetFile = http->get(page, &ioBuffer);
}
//-------------------------------------------------------------------------------
DialogServerStatus::~DialogServerStatus() {
    if (http != NULL)
        delete http;
}

//-------------------------------------------------------------------------------
void DialogServerStatus::requestFinished ( int id, bool error )
{
    QString strDur;
    int tp = timeLoad.elapsed();
    QTextStream(&strDur) << tp << " ms";
    if (error) {
        lbResponseTime->setText(strDur);
        lbResponseStatus->setText(tr("Erreur"));
        lbMessage->setText("Error: "+http->errorString());
    }
    else if (id==idHttpGetFile)
    {
        lbResponseTime->setText(strDur);
        lbResponseStatus->setText(tr("ok"));
        
        QString strbuf = ioBuffer.buffer();
        QStringList lsbuf = strbuf.split("\n");
        QString strDate = getNoaaRunDate(lsbuf);
        QString strHour = getNoaaRunHour(lsbuf);

        lbRunDate->setText(strDate);
        lbRunHour->setText(strHour);
    }
}

//-------------------------------------------------------------------------------
QString DialogServerStatus::getNoaaRunDate(QStringList lsbuf)
{
    QString tmp,   rep=tr("format invalide");
    for (int i=0; i < lsbuf.size(); i++)
    {
        QStringList lsval = lsbuf.at(i).split(":");
        if (lsval.size() > 1) {
            if (lsval.at(0) == "gfs_run_date") {
                tmp = lsval.at(1).trimmed();
                if (tmp.size()==8) {   // format: 20080523
                    QDateTime dt= QDateTime::fromString(tmp,"yyyyMMdd");
                    if (dt.isValid()) {
                        rep = Util::formatDateLong(dt.toTime_t());
                    }
                }
            }
        }
    }
    return rep;
}
//-------------------------------------------------------------------------------
QString DialogServerStatus::getNoaaRunHour(QStringList lsbuf)
{
    QString tmp,   rep=tr("format invalide");
    for (int i=0; i < lsbuf.size(); i++)
    {
        QStringList lsval = lsbuf.at(i).split(":");
        if (lsval.size() > 1) {
            if (lsval.at(0) == "gfs_run_hour") {
                rep = lsval.at(1).trimmed()+" h UTC";
            }
        }
    }
    return rep;
}

//-------------------------------------------------------------------------------
void DialogServerStatus::slotBtOK()
{
    
    accept();
}
//-------------------------------------------------------------------------------
void DialogServerStatus::slotBtCancel()
{
    reject();
}

//=============================================================================
// GUI
//=============================================================================
QFrame *DialogServerStatus::createFrameGui(QWidget *parent)
{
    QFrame * frm = new QFrame(parent);
    QFrame * ftmp;
    QLabel * label;
    QGridLayout  *lay = new QGridLayout(frm);
    int lig=0;
    //-------------------------
    lig ++;
    label = new QLabel(tr("Connexion :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    lbResponseStatus = new QLabel("", frm);
    lay->addWidget( lbResponseStatus, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Temps de réponse :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    lbResponseTime = new QLabel("", frm);
    lay->addWidget( lbResponseTime, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); lay->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Date de la prévision :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    lbRunDate = new QLabel("", frm);
    lay->addWidget( lbRunDate, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Heure :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    lbRunHour = new QLabel("", frm);
    lay->addWidget( lbRunHour, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); lay->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    lbMessage = new QLabel("", frm);
    lay->addWidget( lbMessage, lig,0, Qt::AlignLeft);

    return frm;
}











