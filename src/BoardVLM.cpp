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

#include <QtGui>
#include <QMessageBox>

#include "BoardVLM.h"
#include <qextserialport.h>
#include "Orthodromie.h"
#include "Util.h"

#define VLM_NO_REQUEST     -1
#define VLM_REQUEST_LOGIN  0
#define VLM_DO_REQUEST     1
#define VLM_WAIT_RESULT    2

#define VLM_CMD_HD     1
#define VLM_CMD_ANG    2
#define VLM_CMD_WP     3
#define VLM_CMD_ORTHO  4
#define VLM_CMD_VMG    5

#define MAX_RETRY 5

#define SPEED_COLOR_UPDATE    "color: rgb(100, 200, 0);"
#define SPEED_COLOR_VLM       "color: rgb(255, 0, 0);"
#define SPEED_COLOR_NO_POLAR  "color: rgb(255, 170, 127);"

boardVLM::boardVLM(QMainWindow * mainWin,QWidget * parent) : QWidget(parent)
{
    setupUi(this);

    isComputing = false;

    connect(this,SIGNAL(showMessage(QString)),mainWin,SLOT(slotShowMessage(QString)));
    connect(mainWin,SIGNAL(setChangeStatus(bool)),this,SLOT(setChangeStatus(bool)));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this, SLOT(chkResult()));

    GPS_timer = new QTimer(this);
    GPS_timer->setSingleShot(true);
    connect(GPS_timer,SIGNAL(timeout()),this, SLOT(synch_GPS()));

    QDockWidget * VLMDock = new QDockWidget("VLM 1");
    VLMDock->setWidget(this);
    VLMDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    mainWin->addDockWidget(Qt::LeftDockWidgetArea,VLMDock);

    board2 = new boardVLM_part2();
    connect(board2,SIGNAL(showMessage(QString)),mainWin,SLOT(slotShowMessage(QString)));
    QDockWidget * VLMDock2 = new QDockWidget("VLM 2");
    VLMDock2->setWidget(board2);
    VLMDock2->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    mainWin->addDockWidget(Qt::RightDockWidgetArea,VLMDock2);
    connect(board2,SIGNAL(go_pilototo()),mainWin,SLOT(slotPilototo()));

    connect(board2,SIGNAL(sendCmd(int,float,float,float)),this,SLOT(sendCmd(int,float,float,float)));

    /* edit field keyPress */
    connect(editHeading,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));
    connect(editAngle,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));
    
    QString str;
    str.sprintf("%c",176);

    deg_unit_1->setText(str);

    /* inet init */

    inetManager = new QNetworkAccessManager(this);
    if(inetManager)
    {
        host = Util::getHost();
        connect(inetManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished (QNetworkReply*)));
        Util::paramProxy(inetManager,host);
    }


    currentRequest = VLM_NO_REQUEST;
    currentBoat = NULL;

    chk_GPS->setEnabled(Util::getSetting("gpsEmulEnable", "0").toString()=="1");
    if(!chk_GPS->isEnabled())
    {
        chk_GPS->setCheckState(Qt::Unchecked);
        chk_GPS->hide();
    }
    else
        chk_GPS->show();
    COM=Util::getSetting("serialName", "COM2").toString();
}

void boardVLM::paramChanged(void)
{
    chk_GPS->setEnabled(Util::getSetting("gpsEmulEnable", "0").toString()=="1");
    if(!chk_GPS->isEnabled())
    {
        chk_GPS->setCheckState(Qt::Unchecked);
        chk_GPS->hide();
    }
    else
        chk_GPS->show();
    COM=Util::getSetting("serialName", "COM2").toString();
}

void boardVLM::showGribPointInfo(const GribPointInfo &pf)
{
    board2->showGribPointInfo(pf);
}

#define calcAngleSign(VAL,ANGLE) { \
    showMessage(QString("comp %1, VLM %2").arg(VAL).arg(ANGLE));\
    if(qAbs(VAL)>180)              \
    {                              \
        if(VAL>0)                  \
            ANGLE=-ANGLE;          \
    }                              \
    else                           \
    {                              \
        if(VAL<0)                  \
            ANGLE=-ANGLE;          \
    }                              \
    showMessage(QString("new val %1").arg(ANGLE));\
}

void boardVLM::boatUpdated(boatAccount * boat)
{
    float angle;


    if(boat == NULL)
        return;

    isComputing = true;
    
    float WPLat = boat->getWPLat();
    float WPLon = boat->getWPLon();
    float dirAngle;
    float val=boat->getHeading()-boat->getWindDir();

    currentBoat = boat;

    angle = boat->getTWA();

    calcAngleSign(val,angle);

    editHeading->setValue(boat->getHeading());
    editAngle->setValue(angle);


    w_dir->setText(QString().setNum(boat->getWindDir()));
    w_speed->setText(QString().setNum(boat->getWindSpeed()));
    loch->setText(QString().setNum(boat->getLoch()));
    speed->setText(QString().setNum(boat->getSpeed()));
    avg->setText(QString().setNum(boat->getAvg()));

    if(WPLat != 0 && WPLon != 0)
    {
        Orthodromie orth = Orthodromie(boat->getLon(),boat->getLat(),WPLon,WPLat);
        dirAngle = orth.getAzimutDeg();
        showMessage("Angle to WP: " + QString().setNum(dirAngle));
    }
    else
        dirAngle=-1;

    windAngle->setValues(boat->getHeading(),boat->getWindDir(),boat->getWindSpeed(), dirAngle);

    /* boat info */
    if(boat->getAliasState())
        boatID_str->setText(boat->getAlias() + " (" + boat->getBoatId() + ")");
    else
        boatID_str->setText(boat->getLogin() + " (" + boat->getBoatId() + ")");
    
    boatName->setText(boat->getBoatName());
    boatScore->setText(boat->getScore());

    QString polar_str=boat->getCurrentPolarName();
    if(boat->getPolarState())
        polar_str+= " ("+tr("forcé")+")";
    if(!boat->getPolarData())
        polar_str+= " ("+tr("erreur")+")";
    polarName->setText(polar_str);

    /* boat position */
    latitude->setText(Util::pos2String(TYPE_LAT,boat->getLat()));
    longitude->setText(Util::pos2String(TYPE_LON,boat->getLon()));

    /* update board2 */

    board2->boatUpdated(boat);
    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
    isComputing = false;
    speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    setChangeStatus(boat->getLockStatus());

    /* send data as a GPS */
    synch_GPS();
}

void boardVLM::setWP(float lat,float lon,float wph)
{
    if(!currentBoat)
    {
        showMessage("No current boat for setWP");
        return;
    }
    sendCmd(VLM_CMD_WP,lat,lon,wph);
}

void boardVLM::chgHeading()
{
    sendCmd(VLM_CMD_HD,editHeading->value(),0,0);
}

void boardVLM::headingUpdated(double heading)
{
    if(!currentBoat) /*no current boat, nothing to do*/
        return;

    if(isComputing) return;
    isComputing=true;

    showMessage(QString("heading: cur=%1, new=%2").arg((float)heading).arg(currentBoat->getHeading()));
    
    if((float)heading==currentBoat->getHeading())
    {
        /* setting back to VLM value */
        showMessage("Back to VLM value");
        speed->setText(QString().setNum(currentBoat->getSpeed()));
        speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        float val=currentBoat->getHeading()-currentBoat->getWindDir();
        float angle = currentBoat->getTWA();
        calcAngleSign(val,angle);
        editAngle->setValue(angle);
    }
    else
    {
        /* heading value has changed => compute angle */
        float angle=heading-currentBoat->getWindDir();
        if(qAbs(angle)>180)
        {
            if(angle<0)
                angle=360+angle;
            else
                angle=angle-360;
        }
        /* set new angle */
        editAngle->setValue(angle);

        /* compute speed if a polar is known */
        if(currentBoat->getPolarData())
        {
            float newSpeed=currentBoat->getPolarData()->getSpeed(currentBoat->getWindSpeed(),angle);
            speed->setText(QString().setNum(((float)qRound(newSpeed*100))/100));
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
        }
        else
        {
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
        }
    }

    isComputing=false;
}

void boardVLM::angleUpdated(double angle)
{
    if(!currentBoat) /*no current boat, nothing to do*/
        return;

    if(isComputing) return;
    isComputing=true;

/* compute VLM angle */
    float val=currentBoat->getHeading()-currentBoat->getWindDir();
    float oldAngle=currentBoat->getTWA();
    calcAngleSign(val,oldAngle);
    oldAngle=((float)qRound(oldAngle*10))/10;

    if(angle==oldAngle)
    {
/* setting back to VLM value */
        speed->setText(QString().setNum(currentBoat->getSpeed()));
        editHeading->setValue(currentBoat->getHeading());
        speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    }
    else
    {
        /* angle has changed */
        /* compute heading */
        float heading = currentBoat->getWindDir() + angle;
        if(heading<0) heading+=360;
        else if(heading>360) heading-=360;
        editHeading->setValue(heading);
/* compute speed if a polar is known */
        if(currentBoat->getPolarData())
        {
            float newSpeed=currentBoat->getPolarData()->getSpeed(currentBoat->getWindSpeed(),angle);
            speed->setText(QString().setNum(((float)qRound(newSpeed*100))/100));
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
        }
        else
        {
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
        }
    }
    isComputing=false;
}

void boardVLM::doVirer()
{
    float val = editAngle->value();
    editAngle->setValue(-val);
}

void boardVLM::chgAngle()
{
    sendCmd(VLM_CMD_ANG,editAngle->value(),0,0);
}

void boardVLM::doSynch()
{
    if(currentBoat)
    {
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
        currentBoat->getData();
    }
}

void boardVLM::synch_GPS()
{
  if(!currentBoat)
      return;

  if(chk_GPS->isChecked())
  {
      showMessage("Starting gps synch");
      QextSerialPort * port = new QextSerialPort(COM);
      port->setBaudRate(BAUD19200);
      port->setFlowControl(FLOW_OFF);
      port->setParity(PAR_NONE);
      port->setDataBits(DATA_8);
      port->setStopBits(STOP_1);

      port->open(QIODevice::ReadWrite);

      if(!port->isOpen())
      {
          showMessage("Port not open");
          return;
      }
      showMessage("Port ok");

      QString data1;
      QString data;
      float lat=qAbs(currentBoat->getLat());
      int deg=((int)lat);
      lat=(lat-deg)*60;
      deg=deg*100;
      lat=lat+deg;
      float lon=qAbs(currentBoat->getLon());
      deg=((int)lon);
      lon=(lon-deg)*60;
      deg=deg*100;
      lon=lon+deg;
      QDateTime now = QDateTime::currentDateTime();

      data1.sprintf("%04.2f,%s,%05.2f,%s,",lat,currentBoat->getLat()<0?"S":"N",lon,currentBoat->getLon()<0?"W":"E");

      for(int i=0;i<15;i++)
      {
          data="$GPGLL,"+data1+now.toString("HHmmss")+",A\x0D\x0A";
          now.addSecs(1);
          port->write(data.toAscii(),data.length());
      }

      delete port;
      GPS_timer->start(30*1000);
    }
    else
        showMessage("btn not checked => not sending");
}

void boardVLM::setChangeStatus(bool status)
{
    bool st=!status;
    btn_chgHeading->setEnabled(st);
    editHeading->setEnabled(st);
    chgAngle_2->setEnabled(st);
    btn_chgAngle->setEnabled(st);
    editAngle->setEnabled(st);
    board2->setChangeStatus(status);
}

/*********************/
/* http requests     */

void boardVLM::updateProxy(void)
{
    /* update connection */
    Util::paramProxy(inetManager,host);
}

void boardVLM::sendCmd(int cmdNum,float  val1,float val2, float val3)
{
    if(inetManager && currentRequest == VLM_NO_REQUEST && currentBoat)
    {
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
        currentRequest=VLM_REQUEST_LOGIN;
        currentCmdNum=cmdNum;
        cmd_val1=val1;
        cmd_val2=val2;
        cmd_val3=val3;
        QString page;
        QTextStream(&page)
                           << host
                        << "/myboat.php?"
                        << "pseudo=" << currentBoat->getLogin()
                        << "&password=" << currentBoat->getPass()
                        << "&lang=fr&type=login"
                        ;

        emit showMessage(QString("login for cmd %1 (%2,%3,%4)").arg(cmdNum).arg(cmd_val1)
                        .arg(cmd_val2).arg(cmd_val3));

        inetManager->get(QNetworkRequest(QUrl(page)));
    }
    else
    {
        emit showMessage(QString("error sendCmd %2").arg(currentRequest));
    }
}

void boardVLM::requestFinished ( QNetworkReply* inetReply)
{
    QString page;
    if (inetReply->error() != QNetworkReply::NoError) {
        emit showMessage("Error doing inetGet:" + QString().setNum(inetReply->error()));
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
        currentRequest=VLM_NO_REQUEST;
    }
    else
    {
        switch(currentRequest)
        {
            case VLM_NO_REQUEST:
                btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
                return;
            case VLM_REQUEST_LOGIN:
                emit showMessage("Login done");
                //emit showMessage(inetReply->readAll());
                currentRequest=VLM_DO_REQUEST;

                switch(currentCmdNum)
                {
                    case VLM_CMD_HD:
                        QTextStream(&page) << host
                                   << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                   << currentBoat->getBoatId().toInt()
                                   << "&pilotmode=autopilot&boatheading="+QString().setNum(cmd_val1)
                                ;
                        break;
                    case VLM_CMD_ANG:
                        QTextStream(&page) << host
                                   << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                   << currentBoat->getBoatId().toInt()
                                   << "&pilotmode=windangle&pilotparameter="+QString().setNum(cmd_val1)
                                ;
                        break;
                    case VLM_CMD_VMG:
                        QTextStream(&page) << host
                                   << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                   << currentBoat->getBoatId().toInt()
                                   << "&pilotmode=bestvmg"
                                ;
                        break;
                    case VLM_CMD_ORTHO:
                        QTextStream(&page) << host
                                   << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                   << currentBoat->getBoatId().toInt()
                                   << "&pilotmode=orthodromic"
                                ;
                        break;
                    case VLM_CMD_WP:
                        QTextStream(&page) << host
                                   << "/myboat.php?type=savemywp"
                                   << "&pseudo=" << currentBoat->getLogin()
                                   << "&password=" << currentBoat->getPass()
                                   << "&lang=fr"
                                   << "&targetlat="+QString().setNum(cmd_val1)
                                   << "&targetlong="+QString().setNum(cmd_val2)
                                   << "&targetandhdg="+QString().setNum(cmd_val3)
                                ;
                        break;
                }
                showMessage("Request: " + page);
        inetManager->get(QNetworkRequest(QUrl(page)));
                break;
            case VLM_DO_REQUEST:
                emit showMessage("Request done");
                //emit showMessage(inetReply->readAll());
                currentRequest=VLM_WAIT_RESULT;
                /* update boat info */
                nbRetry=0;
                timer->start(1000);
                break;
        }
    }
}

void boardVLM::chkResult(void)
{
    bool done=false;
    float data;
    float val;
    currentBoat->getData();
    switch(currentCmdNum)
    {
        case VLM_CMD_HD:
            data=currentBoat->getHeading();
            showMessage(QString("HD cur=%1 should be %2 pilot %3").arg(data)
                    .arg(cmd_val1).arg(currentBoat->getPilotType()));
            if(data == cmd_val1 && currentBoat->getPilotType() == 1)
                done=true;
            break;
        case VLM_CMD_ANG:
            data = currentBoat->getTWA();
            val=currentBoat->getHeading()-currentBoat->getWindDir();
            calcAngleSign(val,data);

            showMessage(QString("ANG cur=%1 should be %2 pilot %3").arg(data)
                    .arg(cmd_val1).arg(currentBoat->getPilotType()));
            if(((int)data) == ((int)cmd_val1) && currentBoat->getPilotType() == 2)
                done=true;
            break;
        case VLM_CMD_VMG:
            showMessage(QString("Pilot cur=%1 should be 4").arg(currentBoat->getPilotType()));
            if(currentBoat->getPilotType() == 4)
                done=true;
            break;
        case VLM_CMD_ORTHO:
            showMessage(QString("Pilot cur=%1 should be 3").arg(currentBoat->getPilotType()));
            if(currentBoat->getPilotType() == 3)
                done=true;
            break;
        case VLM_CMD_WP:
            if(currentBoat->getWPLat()==cmd_val1 && currentBoat->getWPLon()==cmd_val2 && currentBoat->getWPHd()==cmd_val3)
                done=true;
            break;
    }
    if(done)
    {
        currentRequest=VLM_NO_REQUEST;
        boatUpdated(currentBoat);
    }
    else
    {
        nbRetry++;
        if(nbRetry>MAX_RETRY)
        {
            showMessage("Failed to synch");
            currentRequest=VLM_NO_REQUEST;
            btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 170, 0);"));
            return;
        }
        showMessage("Retry...");
        timer->start(1000);
    }
}

void boardVLM::edtSpinBox_key()
{
    QObject *s = sender();
    if(s==NULL) return;
    
    if(s==editHeading)
        chgHeading();
    else if(s==editAngle)
        chgAngle();
}

/************************/
/* Board 2              */

boardVLM_part2::boardVLM_part2(QWidget * parent) : QWidget(parent)
{
    setupUi(this);

    currentBoat=NULL;

    QString str;
    str.sprintf("%c",176);

    deg_unit_2->setText(str);
    deg_unit_3->setText(str);
    deg_unit_4->setText(str);
    deg_unit_5->setText(str);

    WP_conv_lat->setText("");
    WP_conv_lon->setText("");

    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this, SLOT(updateNxtVac()));
}

void boardVLM_part2::boatUpdated(boatAccount * boat)
{
    currentBoat = boat;

    float WPLat = boat->getWPLat();
    float WPLon = boat->getWPLon();
    float WPHd = boat->getWPHd();
    int nbS,j,h,m;
    QString txt;

    float angle_val;
        
    timer->stop();

    emit showMessage(QString("WP: lat=%1 lon=%2 @%3").arg(WPLat).arg(WPLon).arg(WPHd));

    if(WPLat == 0 && WPLon == 0)
    {
        WP_lat->setText("");
        WP_lon->setText("");
        WP_heading->setText("");
    }
    else
    {
        WP_lat->setText(QString().setNum(WPLat));
        WP_lon->setText(QString().setNum(WPLon));
        if(WPHd==-1)
            WP_heading->setText("");
        else
            WP_heading->setText(QString().setNum(WPHd));
    }

    map_lon->setText("");
    map_lat->setText("");
    map_wind_angle->setText("");
    map_wind_spd_kts->setText("");
    map_wind_spd_bf->setText("");

    /* pilot type */
    switch(boat->getPilotType())
    {
        case 1: /*heading*/
            pilotType->setText("Barre automatique " + boat->getPilotString() + tr("°"));
            break;
        case 2: /*constant angle*/
            pilotType->setText("Régulateur d'allure " + boat->getPilotString()+ tr("°"));
            break;
        case 3: /*pilotortho*/
            pilotType->setText("Ortho-->" + boat->getPilotString());
            break;
        case 4: /*VMG*/
            pilotType->setText("BVMG-->" + boat->getPilotString());
            break;
    }

    QString Eta = boat->getETA();
    QDateTime dtm =QDateTime::fromString(Eta,"yyyy-MM-dd HH:mm:ss");
    dtm.setTimeSpec(Qt::UTC);
    QDateTime now = (QDateTime::currentDateTime()).toUTC();

    nbS=now.secsTo(dtm);
    j = nbS/(24*3600);
    nbS-=j*24*3600;
    h=nbS/3600;
    nbS-=h*3600;
    m=nbS/60;
    nbS-=m*60;

    txt.sprintf("(%dj %02dh%02dm%02ds)",j,h,m,nbS);
    pilotETA->setText(dtm.toString("dd-MM-yyyy, HH:mm:ss")+ " " +txt);

    dnm->setText(QString().setNum(boat->getDnm()));
    ortho->setText(QString().setNum(boat->getOrtho()));
    loxo->setText(QString().setNum(boat->getLoxo()));
    vmg->setText(QString().setNum(boat->getVmg()));
    
    angle_val=boat->getOrtho()-boat->getWindDir();    
    if(qAbs(angle_val)>180)
    {
	if(angle_val<0)
	    angle_val=360+angle_val;
	else
	    angle_val=angle_val-360;
    }
    angle->setText(QString().setNum(angle_val));

    QDateTime lastVac_date;
    lastVac_date.setTime_t(boat->getPrevVac());
    lastVac_date.setTimeSpec(Qt::UTC);

    lastVac->setText(lastVac_date.toString("dd-MM-yyyy, HH:mm:ss"));
    nextVac->setText(QString().setNum(boat->getNextVac()));
    nxtVac_cnt=boat->getNextVac();
    timer->start(1000);

    /* compute nb Pilototo instructions */
    QStringList * lst = boat->getPilototo();
    QString pilototo_txt=tr("Pilototo");
    if(boat->getHasPilototo())
    {
        int nb=0;
        for(int i=0;i<lst->count();i++)
            if(lst->at(i)!="none") nb++;
        if(nb!=0)
            pilototo_txt=pilototo_txt+" ("+QString().setNum(nb)+")";
        goPilototo->setToolTip("");
    }
    else
    {
        goPilototo->setToolTip(tr("Imp. de lire le pilototo de VLM"));
        pilototo_txt=pilototo_txt+" (!)";
    }
    goPilototo->setText(pilototo_txt);
}

void boardVLM_part2::updateNxtVac()
{
    nxtVac_cnt--;
    if(nxtVac_cnt<0)
        nxtVac_cnt=300;
    nextVac->setText(QString().setNum(nxtVac_cnt));
}

void boardVLM_part2::setChangeStatus(bool status)
{
    bool st=!status;
    btn_clear->setEnabled(st);
    btn_saveWP->setEnabled(st);
    btn_paste->setEnabled(st);
    btn_paste_2->setEnabled(st);
    goPilotOrtho->setEnabled(st);
    WP_heading->setEnabled(st);
    WP_lat->setEnabled(st);
    WP_lon->setEnabled(st);
    goVMG->setEnabled(st);
    goPilototo->setEnabled(st);
}

void boardVLM_part2::doPilotOrtho()
{
    sendCmd(VLM_CMD_ORTHO,0,0,0);
}

void boardVLM_part2::doVmg()
{
    sendCmd(VLM_CMD_VMG,0,0,0);
}

void boardVLM_part2::doClearWP()
{
    WP_lat->setText("");
    WP_lon->setText("");
    WP_heading->setText("");
}

void boardVLM_part2::chgLat()
{
    if(WP_lat->text().isEmpty())
        WP_conv_lat->setText("");
    else
    {
        float val = WP_lat->text().toFloat();
        WP_conv_lat->setText(Util::pos2String(TYPE_LAT,val));
    }
}

void boardVLM_part2::chgLon()
{
    if(WP_lon->text().isEmpty())
        WP_conv_lon->setText("");
    else
    {
        float val = WP_lon->text().toFloat();
        WP_conv_lon->setText(Util::pos2String(TYPE_LON,val));
    }
}

void boardVLM_part2::doSaveWP()
{

    if(WP_lat->text().isEmpty() && WP_lon->text().isEmpty())
    {
        sendCmd(VLM_CMD_WP,0,0,-1);
    }
    else
    {
        sendCmd(VLM_CMD_WP,WP_lat->text().toFloat(),WP_lon->text().toFloat(),
            WP_heading->text().isEmpty()?-1:WP_heading->text().toFloat());
    }
}

void boardVLM_part2::showGribPointInfo(const GribPointInfo &pf)
{
    QString s, res;

    map_lon->setText(Util::pos2String(TYPE_LON,pf.x));
    map_lat->setText(Util::pos2String(TYPE_LAT,pf.y));

    if (pf.hasWind()) {
        float v = sqrt(pf.vx*pf.vx + pf.vy*pf.vy);

        float dir = -atan2(-pf.vx, pf.vy) *180.0/M_PI + 180;
        if (dir < 0)
            dir += 360.0;
        if (dir >= 360)
            dir -= 360.0;

        map_wind_angle->setText(s.sprintf("%.0f", dir));
        map_wind_spd_kts->setText(s.sprintf("%.1f",v*3.6/1.852));
        map_wind_spd_bf->setText(s.sprintf("%2d", Util::kmhToBeaufort(v*3.6)));
    }
    else {
        map_wind_angle->setText("");
        map_wind_spd_kts->setText("");
        map_wind_spd_bf->setText("");
    }
}

void boardVLM_part2::doPaste()
{
    float lat,lon,wph;
    if(!currentBoat)
        return;
    if(!Util::getWPClipboard(&lat,&lon,&wph,NULL)) /*no need to get timestamp*/
        return;
    WP_lat->setText(QString().setNum(lat));
    WP_lon->setText(QString().setNum(lon));
    WP_heading->setText(QString().setNum(wph));
}

void boardVLM_part2::doCopy()
{
    if(!currentBoat)
        return;
    Util::setWPClipboard(WP_lat->text().toFloat(),WP_lon->text().toFloat(),
        WP_heading->text().toFloat());

}

void boardVLM_part2::doPilototo()
{
    emit go_pilototo();
}

/************************/
/* Board custom spinBox */


tool_edtSpinBox::tool_edtSpinBox(QWidget * parent): QDoubleSpinBox(parent)
{
    this->parent=parent;
}

void tool_edtSpinBox::keyPressEvent ( QKeyEvent * event )
{
    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
    {
        emit hasEvent();
        return;
    }
    QDoubleSpinBox::keyPressEvent(event);
}

void tool_edtSpinBox::keyReleaseEvent ( QKeyEvent * event )
{
    QDoubleSpinBox::keyReleaseEvent (event);
}

/*********************/
/* VLM100 nav center */

tool_navCenter::tool_navCenter(QWidget * parent):QWidget(parent)
{
    /* load image */
    img_fond = new QImage("img/tool_2zones_2.png");
    w=img_fond->width();
    h=img_fond->height();
    setFixedSize(w,h);



    /* init var */
    lat=lon=speed=avg=heading=dnm=loch=ortho=loxo=vmg=0;

}

void tool_navCenter::setValues(float lat, float lon, float speed, float avg, float heading,
                               float dnm, float loch, float ortho, float loxo, float vmg)
{
    this->lat=lat;
    this->lon=lon;
    this->speed=speed;
    this->avg=avg;
    this->heading=heading;
    this->dnm=dnm;
    this->loch=loch;
    this->ortho=ortho;
    this->loxo=loxo;
    this->vmg=vmg;
    update();
}

void tool_navCenter::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setViewport(0,0,w,h);

    draw(&painter);
}

void tool_navCenter::draw(QPainter * painter)
{
    if(!img_fond->isNull())
    {
        QString str;
        painter->drawImage(0,0,*img_fond);
        painter->setLayoutDirection(Qt::LeftToRight);

        /* title */
        painter->setPen(QColor(220, 200, 140));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(11);
        painter->setFont(font);
        painter->translate(11, 176) ;
        painter->rotate(-90);
        painter->drawText(QRect(0,0,162,23),Qt::AlignCenter,"VLM100 Nav-Center");
        painter->rotate(90);
        painter->translate(-11, -176) ;

        /* lat and lon */
        painter->setPen(QColor(0, 0, 0));
        font.setBold(false);
        int d1,m1,s1,d2,m2,s2;
        float l;
        l=lat<0?-lat:lat;
        d1=(int)l;
        m1=(int)((l-d1)*60);
        s1=(int)((l-d1-(float)m1/60)*3600);
        l=lon<0?-lon:lon;
        d2=(int)l;
        m2=(int)((l-d2)*60);
        s2=(int)((l-d2-(float)m2/60)*3600);
        QRect rect;
        str.sprintf("%02d%c%02d'%02d\" %s\n%03d%c%02d'%02d\" %s",
                    d1,176,m1,s1,lat<0?"S":"N",
                    d2,176,m2,s2,lon<0?"W":"E");
        painter->drawText(QRect(45,24,203-45,92-24),Qt::AlignHCenter|Qt::AlignTop,str,&rect);

        /* speed, avg, heading => 1/3 of width for each data*/

        font.setPointSize(9);
        painter->setFont(font);

        int y=24+rect.height()+2;
        int w= (203-45)/3;

        str.sprintf("Speed\n%.2f",speed);
        painter->drawText(QRect(45,y,w,92-y),Qt::AlignHCenter|Qt::AlignTop,str);
        str.sprintf("Avg\n%.2f",avg);
        painter->drawText(QRect(45+w,y,w,92-y),Qt::AlignHCenter|Qt::AlignTop,str);
        str.sprintf("Heading\n%.1f%c",heading,176);
        painter->drawText(QRect(45+2*w,y,w,92-y),Qt::AlignHCenter|Qt::AlignTop,str);

        /* dnm,loch => 1/2 of width for each data*/

        w=(178-22)/2;
        str.sprintf("DNM\n%.2f",dnm);
        painter->drawText(QRect(45,99,w,163-99),Qt::AlignHCenter|Qt::AlignTop,str,&rect);
        str.sprintf("Loch\n%.2f",loch);
        painter->drawText(QRect(45+w,99,w,163-99),Qt::AlignHCenter|Qt::AlignTop,str);

        /* ortho, loxo, vmg => 1/3 of width for each data*/
        y=99+rect.height()+2;
        w= (203-45)/3;
        str.sprintf("Ortho\n%.1f%c",ortho,176);
        painter->drawText(QRect(45,y,w,163-y),Qt::AlignHCenter|Qt::AlignTop,str);
        str.sprintf("Loxo\n%.1f%c",loxo,176);
        painter->drawText(QRect(45+w,y,w,163-y),Qt::AlignHCenter|Qt::AlignTop,str);
        str.sprintf("VMG\n%.2f",vmg);
        painter->drawText(QRect(45+2*w,y,w,163-y),Qt::AlignHCenter|Qt::AlignTop,str);
    }
}

/*********************/
/* VLM20 windAngle  */

tool_windAngle::tool_windAngle(QWidget * parent):QWidget(parent)
{
    /* load image */
    img_fond = new QImage("img/tool_1zone_2.png");
    w=img_fond->width();
    h=img_fond->height();
    setFixedSize(w,h);

    img_compas = new QImage("img/tool_compas.png");
    img_boat = new QImage("img/tool_boat.png");

    heading =windDir=windSpeed=0;
    WPdir = -1;
}

void tool_windAngle::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setViewport(0,0,w,h);

    draw(&painter);
}

void tool_windAngle::draw(QPainter * painter)
{
    if(!img_fond->isNull() && !img_compas->isNull() && !img_boat->isNull())
    {
        int x,y;
        painter->drawImage(0,0,*img_fond);

        x=23+((w-23)-img_compas->width())/2;
        y=23; /*not centering */
        painter->drawImage(x,y,*img_compas);

        /* center */
        x=45+(203-45)/2;
        y=24+(163-24)/2;
        int h=163-24;
        painter->translate(x, y) ;

        /* rotate + boat */
        painter->rotate(heading);
        painter->drawImage(-img_boat->width()/2,-img_boat->height()/2,*img_boat);
        painter->rotate(-heading);

        /* rotate + wind dir */
        painter->rotate(windDir);
        QPen curPen = painter->pen();
        painter->setPen(windSpeed_toColor());
        painter->fillRect(-2,-h/2,4,40,QBrush(windSpeed_toColor()));
        painter->setPen(curPen);
        painter->rotate(-windDir);

        /* rotate + WP dir */
        if(WPdir != -1)
        {
            painter->rotate(WPdir);
            QPen curPen = painter->pen();
            painter->setPen(QColor(0,0,0));
            painter->fillRect(-2,-h/2,4,20,QColor(0,0,0));
            painter->setPen(curPen);
            painter->rotate(-WPdir);
        }

        painter->translate(-x, -y) ;
        /* title */
        painter->setPen(QColor(220, 200, 140));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(11);
        painter->setFont(font);
        painter->translate(11, 176) ;
        painter->rotate(-90);
        painter->drawText(QRect(0,0,162,23),Qt::AlignCenter,"VLM20 Wind-Angle");
        painter->rotate(90);
        painter->translate(-11, -176) ;

    }
}

QColor tool_windAngle::windSpeed_toColor()
{
   //color from VLM code: http://www.virtual-loup-de-mer.org
   // <=F0 : blanc
   if (windSpeed <= 1) return QColor(255, 255, 255);
   // <=F1 : bleu clair legerement gris
   else if (windSpeed <= 3) return QColor(150, 150, 225 );
   // <=F2 : bleu un peu plus soutenu
   else if (windSpeed <= 6) return QColor(80, 140, 205);
   // <=F3 : bleu plus fonc�
   else if (windSpeed <= 10) return QColor(60, 100, 180);
   // <=F4 : vert
   else if (windSpeed <= 15) return QColor(65, 180, 100);
   // <=F5 : jaune l�g�rement vert
   else if (windSpeed <= 21) return QColor(180, 205, 10);
   // <=F6 : jaune orang�
   else if (windSpeed <= 26) return QColor(210, 210, 22);
   // <=F7 : jaune orang� un peu plus rougeatre
   else if (windSpeed <= 33) return QColor(225, 210, 32);
   // <=F8 : orange fonc�
   else if (windSpeed <= 40) return QColor(255, 179, 0);
   // <=F9 : rouge
   else if (windSpeed <= 47) return QColor(255, 111, 0);
   // <=F10 rouge / marron
   else if (windSpeed <= 55) return QColor(255, 43, 0);
   // <=F11 marron
   else if (windSpeed <= 63) return QColor(230, 0, 0);
   // F12  rouge/noir
   else return QColor(127, 0, 0);

}

void tool_windAngle::setValues(float heading,float windDir, float windSpeed, float WPdir)
{
    this->heading=heading;
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->WPdir=WPdir;
    update();
}

/**********************/
/* VLM10 windStation  */

tool_windStation::tool_windStation(QWidget * parent):QWidget(parent)
{
    /* load image */
    img_fond = new QImage("img/tool_1zone_2.png");
    w=img_fond->width();
    h=img_fond->height();
    setFixedSize(w,h);

    windSpeed=windDir=windAngle=0;
}

void tool_windStation::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setViewport(0,0,w,h);

    draw(&painter);
}

#define NV_WIND_SPACE 5
#define NV_WIND_DATA_SIZE 25
#define NV_WIND_UNIT_SIZE 8

void tool_windStation::draw(QPainter * painter)
{
    if(!img_fond->isNull())
    {
        int y,w;
        QRect rect;
        QFont font;
        QString str;


        painter->drawImage(0,0,*img_fond);

        /* title */
        painter->setPen(QColor(220, 200, 140));
        font = painter->font();
        font.setBold(true);
        font.setPointSize(10);
        painter->setFont(font);
        painter->translate(11, 176) ;
        painter->rotate(-90);
        painter->drawText(QRect(0,0,162,23),Qt::AlignCenter,"VLM10 Wind-Station");
        painter->rotate(90);
        painter->translate(-11, -176) ;

        /* data */
        painter->setPen(QColor(0, 0, 0));


        y=24;

        /* windSpeed */
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(QRect(45,y,203-45,163-y),Qt::AlignLeft|Qt::AlignTop,"Wind Speed",&rect);
        y+=rect.height()-NV_WIND_SPACE;
        font.setPointSize(NV_WIND_DATA_SIZE);
        font.setBold(true);
        painter->setFont(font);
        str.sprintf("%.1f",windSpeed);
        painter->drawText(QRect(45,y,203-45,163-y),Qt::AlignHCenter|Qt::AlignTop,str,&rect);
        font.setPointSize(NV_WIND_UNIT_SIZE);
        font.setBold(true);
        painter->setFont(font);
        w=rect.x()+rect.width();
        painter->drawText(QRect(45+w,y,203-w,163-y),Qt::AlignLeft|Qt::AlignTop,"kts");

        y+=rect.height()-NV_WIND_SPACE;

        /* windDir */
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(QRect(45,y,203-45,163-y),Qt::AlignLeft|Qt::AlignTop,"Wind Direction",&rect);
        y+=rect.height()-NV_WIND_SPACE;
        font.setPointSize(NV_WIND_DATA_SIZE);
        font.setBold(true);
        painter->setFont(font);
        str.sprintf("%.1f",windDir);
        painter->drawText(QRect(45,y,203-45,163-y),Qt::AlignHCenter|Qt::AlignTop,str,&rect);
        font.setPointSize(NV_WIND_UNIT_SIZE);
        font.setBold(true);
        painter->setFont(font);
        str.sprintf("%c",176);
        w=rect.x()+rect.width();
        painter->drawText(QRect(22+w,y,178-w,163-y),Qt::AlignLeft|Qt::AlignTop,str);

        y+=rect.height()-NV_WIND_SPACE;

        /* windAngle */
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(QRect(45,y,203-45,163-y),Qt::AlignLeft|Qt::AlignTop,"Wind Angle",&rect);
        y+=rect.height()-NV_WIND_SPACE;
        font.setPointSize(NV_WIND_DATA_SIZE);
        font.setBold(true);
        painter->setFont(font);
        str.sprintf("%.1f",windAngle);
        painter->setPen(windAngle>0?QColor(0, 160, 32):QColor(200, 32, 32));
        painter->drawText(QRect(22,y,178-22,163-y),Qt::AlignHCenter|Qt::AlignTop,str,&rect);
        font.setPointSize(NV_WIND_UNIT_SIZE);
        font.setBold(true);
        painter->setFont(font);
        str.sprintf("%c",176);
        w=rect.x()+rect.width();
        painter->drawText(QRect(45+w,y,203-w,163-y),Qt::AlignLeft|Qt::AlignTop,str);

    }
}

void tool_windStation::setValues(float windDir, float windSpeed, float windAngle)
{
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->windAngle=windAngle;
    update();
}
