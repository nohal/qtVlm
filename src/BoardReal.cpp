/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#include <QMessageBox>
#include "DialogSailDocs.h"
#include <QMenu>
#include <QDebug>
#include <QString>

#include "MainWindow.h"
#include "DataManager.h"
#include "mycentralwidget.h"

#include "BoardReal.h"
#include "Board.h"
#include "settings.h"
#include "Polar.h"
#include "boatReal.h"

#include "dataDef.h"
#include "Util.h"
#include "AngleUtil.h"

boardReal::boardReal(MainWindow * mainWin, board * parent) : QWidget(mainWin)
{
    setupUi(this);
    board::setFontDialog(this);
//    QString textColor="QPushButton{color: black;}";
//    this->btn_boatInfo->setStyleSheet(textColor);
    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setBold(true);
    wfont.setPointSizeF(14.0);
    exceptions.insert(latitude,wfont);
    exceptions.insert(longitude,wfont);
    wfont=QApplication::font();
    wfont.setPointSizeF(9.0);
    exceptions.insert(ortho,wfont);
    exceptions.insert(dnm_2,wfont);
    exceptions.insert(angle,wfont);
    exceptions.insert(vmg_2,wfont);
    Util::setSpecificFont(exceptions);
    this->mainWin=mainWin;
    this->parent=parent;

    /* Contextual Menu */
    popup = new QMenu(this);
    ac_showHideCompass = new QAction(tr("Cacher compas"),popup);
    popup->addAction(ac_showHideCompass);
    connect(ac_showHideCompass,SIGNAL(triggered()),this,SLOT(slot_hideShowCompass()));
    imgInfo=QPixmap(230,70);
    pntImgInfo.begin(&imgInfo);
    pntImgInfo.setRenderHint(QPainter::Antialiasing,true);
    pntImgInfo.setRenderHint(QPainter::TextAntialiasing,true);
    pntImgInfo.setPen(Qt::black);
    QFont font=pntImgInfo.font();
    font.setPointSize(6);
    pntImgInfo.setFont(font);
    this->gpsInfo->hide();
    this->statusBtn->setEnabled(false);
    connect(this->declinaison,SIGNAL(clicked()),this,SLOT(paramChanged()));
}

boatReal * boardReal::currentBoat(void)
{
    if(parent && parent->currentBoat())
    {
        if(parent->currentBoat()->get_boatType()==BOAT_REAL)
            return (boatReal*)parent->currentBoat();
        else
            return NULL;
    }
    else
        return NULL;
}

void boardReal::setWp(double lat,double lon,double wph)
{
    boatReal * myBoat=currentBoat();
    if(myBoat)
        myBoat->setWP(lat,lon,wph);
    if(myBoat->getWPLat()!=0 && myBoat->getWPLat()!=0)
    {
        dnm_2->setText(QString().setNum(myBoat->getDnm()));
        vmg_2->setText(QString().setNum(myBoat->getVmg()));
        if(!declinaison->isChecked())
        {
            ortho->setText(QString().setNum(myBoat->getOrtho()));
            angle->setText(QString().setNum(myBoat->getLoxo()));
        }
        else
        {
            double cap=AngleUtil::A360(myBoat->getOrtho()-myBoat->getDeclinaison());
            ortho->setText(QString().setNum(cap));
            cap=AngleUtil::A360(myBoat->getLoxo()-myBoat->getDeclinaison());
            angle->setText(QString().setNum(cap));
        }
    }
    else
    {
        dnm_2->setText("---");
        ortho->setText("---");
        vmg_2->setText("---");
        angle->setText("---");
    }
}

void boardReal::gribUpdated()
{
    boatUpdated();
}

void boardReal::boatUpdated(void)
{
    boatReal * myBoat=currentBoat();

    if(!myBoat)
    {
        return;
    }
    /* boat position */
    latitude->setText(Util::pos2String(TYPE_LAT,myBoat->getLat()));
    longitude->setText(Util::pos2String(TYPE_LON,myBoat->getLon()));

    /* boat heading */
    //windAngle->setValues(myBoat->getHeading(),0,myBoat->getWindSpeed(), -1, -1);
    if(this->declinaison->isChecked())
    {
        double cap=AngleUtil::A360(myBoat->getHeading()-myBoat->getDeclinaison());
        this->dir->display(cap);
    }
    else
        this->dir->display(myBoat->getHeading());
    QString EW=" E";
    if(myBoat->getDeclinaison()<0)
        EW=" W";
    this->declinaison->setText(tr("Inclure la declinaison (")+QString().sprintf("%.1f",qAbs(myBoat->getDeclinaison()))+tr("deg")+EW+")");

    /* boat speed*/
    this->speed->display(myBoat->getSpeed());

    /*WP*/
    if(myBoat->getWPLat()!=0 && myBoat->getWPLat()!=0)
    {
        dnm_2->setText(QString().setNum(myBoat->getDnm()));
        vmg_2->setText(QString().setNum(myBoat->getVmg()));
        if(this->declinaison->isChecked())
        {
            double cap=AngleUtil::A360(myBoat->getOrtho()-myBoat->getDeclinaison());
            ortho->setText(QString().setNum(cap));
            cap=AngleUtil::A360(myBoat->getLoxo()-myBoat->getDeclinaison());
            angle->setText(QString().setNum(cap));
        }
        else
        {
            ortho->setText(QString().setNum(myBoat->getOrtho()));
            angle->setText(QString().setNum(myBoat->getLoxo()));
        }
    }
    else
    {
        dnm_2->setText("---");
        ortho->setText("---");
        vmg_2->setText("---");
        angle->setText("---");
    }
    this->windInfo->setText("--");
    DataManager * dataManager=mainWin->getMy_centralWidget()->get_dataManager();
    if(dataManager && dataManager->isOk())
    {
        time_t now=QDateTime::currentDateTimeUtc().toTime_t();
        QString s=QString();
        if(now>dataManager->get_minDate() && now<dataManager->get_maxDate())
        {
            double tws,twd;
            dataManager->getInterpolatedWind((double) myBoat->getLon(),myBoat->getLat(),
                                   now,&tws,&twd,INTERPOLATION_DEFAULT);
            twd=radToDeg(twd);
            if(twd>360)
                twd-=360;
            double twa=AngleUtil::A180(myBoat->getWindDir()-twd);
            double Y=90-twa;
            double a=tws*cos(degToRad(Y));
            double b=tws*sin(degToRad(Y));
            double bb=b+myBoat->getSpeed();
            double aws=sqrt(a*a+bb*bb);
            double awa=90-radToDeg(atan(bb/a));
            s=s.sprintf("<BODY LEFTMARGIN=\"0\">TWS <FONT COLOR=\"RED\"><b>%.1fnds</b></FONT> TWD %.0fdeg TWA %.0fdeg<br>AWS %.1fnds AWA %.0fdeg",tws,twd,AngleUtil::A180(twa),aws,awa);
            if(myBoat->getPolarData())
            {
                QString s1;
                double bvmgUp=myBoat->getBvmgUp(tws);
                double bvmgDown=myBoat->getBvmgDown(tws);
                s=s+s1.sprintf("<br>Pres %.0fdeg Portant %.0fdeg",bvmgUp,bvmgDown);
            }
            s=s.replace("nds",tr("nds"));
            s=s.replace("deg",tr("deg"));
            s=s.replace("Pres",tr("Pres"));
            s=s.replace("Portant",tr("Portant"));
            this->windInfo->setText(s);
        }
    }
    /* GPS status */
    QString status;
    if(!myBoat->getPause() && !this->gpsInfo->isHidden())
    {
        GpsData info=myBoat->getInfo();
        imgInfo.fill(Qt::white);
        for(int n=0;n<12;n++)
        {
            if(info.sat[n].in_use==0)
                pntImgInfo.setBrush(Qt::red);
            else
                pntImgInfo.setBrush(Qt::green);
            pntImgInfo.drawRect(1+n*19,50,16,-info.sat[n].sigQ*.7);
            pntImgInfo.drawText(1+n*19,52,16,18,Qt::AlignHCenter | Qt::AlignVCenter,QString().setNum(info.sat[n].id));
        }
        gpsInfo->setPixmap(imgInfo);
        status=tr("Running")+"<br>";
        if(myBoat->getSig()==0)
            status=status+tr("Bad signal")+"<br>";
        else if(myBoat->getSig()==1)
            status=status+tr("Fix quality")+"<br>";
        else if (myBoat->getSig()==2)
            status=status+tr("Differential quality")+"<br>";
        else if (myBoat->getSig()==3)
            status=status+tr("Sensitive quality")+"<br>";
        if(myBoat->getFix()==1)
            status=status+tr("no position");
        else if(myBoat->getFix()==2)
            status=status+tr("2D position");
        else
            status=status+tr("3D position");
        if(myBoat->getFix()>1)
        {
            status=status+"<br>";
            if(myBoat->getPdop()<=1)
                status=status+tr("Ideal accuracy");
            else if(myBoat->getPdop()<=2)
                status=status+tr("Excellent accuracy");
            else if(myBoat->getPdop()<=1)
                status=status+tr("Good accuracy");
            else if(myBoat->getPdop()<=5)
                status=status+tr("Moderate accuracy");
            else if(myBoat->getPdop()<=20)
                status=status+tr("Not so good accuracy");
            else
                status=status+tr("Very bad accuracy");
        }
        status=status.replace(" ","&nbsp;");
        gpsInfo->setToolTip(status);
#if 0 /*show the image as a tooltip for startBtn (experimental)*/
        imgInfo.save("tempTip.png");
        startBtn->setToolTip("<p>"+status+"</p><img src=\"tempTip.png\">");
#endif
    }
}

void boardReal::setChangeStatus(bool /*status*/)
{

}

void boardReal::paramChanged()
{
    this->boatUpdated();
}

void boardReal::disp_boatInfo()
{
    QString t="TIME: "+QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm");
    QString letter;
    double lat=this->currentBoat()->getLat();
    double lon=this->currentBoat()->getLon();
    if(lat<0)
        letter="S";
    else
        letter="N";
    lat=qAbs(lat);
    int x1=floor(lat);
    int x2=qRound((lat-x1)*60.0);
    t=t+"<br>"+"LATITUDE: "+QString().sprintf("%02d-%02d",x1,x2)+letter;
    if(lon<0)
        letter="W";
    else
        letter="E";
    lon=qAbs(lon);
    x1=floor(lon);
    x2=qRound((lon-x1)*60.0);
    t=t+"<br>"+"LONGITUDE: "+QString().sprintf("%03d-%02d",x1,x2)+letter;
    t=t+"<br>"+QString().sprintf("COURSE: %d",qRound(currentBoat()->getHeading()));
    t=t+"<br>"+QString().sprintf("SPEED: %d",qRound(currentBoat()->getSpeed()));
    DialogSailDocs * s = new DialogSailDocs(t,this);
    s->label_3->hide();
    s->label_2->hide();
    s->lineEdit->hide();
    s->label->hide();
    s->setWindowTitle(tr("Information"));
    s->exec();
    delete s;
}

void boardReal::chgBoatPosition(void)
{
    boatReal * myBoat=currentBoat();
    if(myBoat)
        myBoat->slot_chgPos();
}

void boardReal::startGPS(void)
{
    boatReal * myBoat=currentBoat();
    if(!myBoat)
    {
        qWarning() << "No real boat to start GPS";
        return;
    }
    startBtn->setDisabled(true);
    statusBtn->setDisabled(true);
    if(!myBoat->getPause())
    {
        myBoat->stopRead();
        this->startBtn->setText(tr("Start GPS"));
        this->gpsInfo->hide();
    }
    else
    {
        myBoat->startRead();
        if(!myBoat->getPause())
        {
            this->startBtn->setText(tr("Stop GPS"));
            this->statusBtn->setEnabled(true);
        }
    }
    if(myBoat->getPause())
    {
        this->startBtn->setText(tr("Start GPS"));
        this->statusBtn->setEnabled(false);
        this->gpsInfo->hide();
    }
    else
    {
        this->startBtn->setText(tr("Stop GPS"));
        this->statusBtn->setEnabled(true);
    }
    this->startBtn->setEnabled(true);
}
 void boardReal::statusGPS(void)
 {
     if(this->gpsInfo->isHidden())
         gpsInfo->show();
     else
         gpsInfo->hide();
 }

void boardReal::slot_hideShowCompass()
{

}

void boardReal::setCompassVisible(bool /*status*/)
{

}

void boardReal::contextMenuEvent(QContextMenuEvent  *)
{

}
