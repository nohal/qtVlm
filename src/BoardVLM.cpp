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
#include <QDebug>

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
    this->mainWin = mainWin;

    connect(mainWin,SIGNAL(setChangeStatus(bool)),this,SLOT(setChangeStatus(bool)));
    connect(this,SIGNAL(VLM_Sync()),mainWin,SLOT(slotVLM_Sync()));

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

    /* wpDialog */
    wpDialog = new WP_dialog();
    connect(wpDialog,SIGNAL(sendCmd(int,float,float,float)),this,SLOT(sendCmd(int,float,float,float)));
    connect(wpDialog,SIGNAL(selectPOI()),mainWin,SLOT(slotSelectWP_POI()));
    connect(mainWin,SIGNAL(editWP_POI(POI*)),wpDialog,SLOT(show_WPdialog(POI *)));

    /* edit field keyPress */
    connect(editHeading,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));
    connect(editAngle,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));

    QString str;
    str.sprintf("%c",176);

    deg_unit_1->setText(str);
    deg_unit_2->setText(str);
    deg_unit_5->setText(str);

    default_styleSheet = btn_chgHeading->styleSheet();

    btn_WP->setText("No WP");

    /* inet init */
    conn=new inetConnexion(this);
    isWaiting=false;

    currentBoat = NULL;

    //boatList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    /*finfo = cbBoatList->fontInfo();
    QFont font2("", finfo.pointSize(), QFont::Normal, false);
    font2.setStyleHint(QFont::TypeWriter);
    font2.setStretch(QFont::SemiCondensed);
    cbBoatList->setFont(font2);*/
    /*connect(boatList, SIGNAL(activated(int)),
            mainWin, SLOT(slotChgBoat(int)));*/

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

#define calcAngleSign(VAL,ANGLE) { \
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
}

float boardVLM::computeWPdir(boatAccount * boat)
{
    float dirAngle;
    float WPLat = boat->getWPLat();
    float WPLon = boat->getWPLon();
    if(WPLat != 0 && WPLon != 0)
    {
        Orthodromie orth = Orthodromie(boat->getLon(),boat->getLat(),WPLon,WPLat);
        dirAngle = orth.getAzimutDeg();
    }
    else
        dirAngle=-1;
    return dirAngle;
}

void boardVLM::boatUpdated(boatAccount * boat)
{
    float angle_val;

    if(boat == NULL)
        return;

    isComputing = true;
    float val=boat->getHeading()-boat->getWindDir();

    currentBoat = boat;

    if(boat->getPilotType() == 2)
        angle_val = boat->getPilotString().toFloat();
    else
    {
        angle_val = boat->getTWA();
        calcAngleSign(val,angle_val);
    }

    editHeading->setValue(boat->getHeading());
    editAngle->setValue(angle_val);

    w_dir->setText(QString().setNum(boat->getWindDir()));
    w_speed->setText(QString().setNum(boat->getWindSpeed()));
    loch->setText(QString().setNum(boat->getLoch()));
    speed->setText(QString().setNum(boat->getSpeed()));
    avg->setText(QString().setNum(boat->getAvg()));

    windAngle->setValues(boat->getHeading(),boat->getWindDir(),boat->getWindSpeed(), computeWPdir(boat), -1);

    boatName->setText(boat->getBoatName());

    if(boat->getAliasState())
        boatName->setText(boat->getAlias() + " (" + boat->getLogin() + " - " + boat->getBoatId() + ")");
    else
        boatName->setText(boat->getLogin() + " ("  + boat->getBoatId() + ")");

    boatScore->setText(boat->getScore());

    /* boat position */
    latitude->setText(Util::pos2String(TYPE_LAT,boat->getLat()));
    longitude->setText(Util::pos2String(TYPE_LON,boat->getLon()));

    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    isComputing = false;
    speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    setChangeStatus(boat->getLockStatus());

    /* update WP button */
    update_btnWP();

    /* WP direction */
    dnm->setText(QString().setNum(boat->getDnm()));
    ortho->setText(QString().setNum(boat->getOrtho()));
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

    /* Pilot mode */
    /* clearing all buttons*/
    btn_chgHeading->setStyleSheet(default_styleSheet);
    goVMG->setStyleSheet(default_styleSheet);
    goPilotOrtho->setStyleSheet(default_styleSheet);
    btn_chgAngle->setStyleSheet(default_styleSheet);
    switch(boat->getPilotType())
    {
        case 1: /*heading*/
             btn_chgHeading->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
            break;
        case 2: /*constant angle*/
            btn_chgAngle->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
            break;
        case 3: /*pilotortho*/
            goPilotOrtho->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
            break;
        case 4: /*VMG*/
            goVMG->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
            break;
    }

    /* send data as a GPS */
    synch_GPS();
}

void boardVLM::setWP(float lat,float lon,float wph)
{
    if(!currentBoat)
    {
        qWarning("No current boat for setWP");
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


    if((float)heading==currentBoat->getHeading())
    {
        /* setting back to VLM value */
        speed->setText(QString().setNum(currentBoat->getSpeed()));
        speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        float val=currentBoat->getHeading()-currentBoat->getWindDir();
        float angle = currentBoat->getTWA();
        calcAngleSign(val,angle);
        editAngle->setValue(angle);
        /*changing boat rotation*/
        windAngle->setValues(currentBoat->getHeading(),currentBoat->getWindDir(),
                             currentBoat->getWindSpeed(), computeWPdir(currentBoat), -1);
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
        /*changing boat rotation*/
        windAngle->setValues(currentBoat->getHeading(),currentBoat->getWindDir(),
                             currentBoat->getWindSpeed(), computeWPdir(currentBoat), heading);
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
        /*changing boat rotation*/
        windAngle->setValues(currentBoat->getHeading(),currentBoat->getWindDir(),
                             currentBoat->getWindSpeed(), computeWPdir(currentBoat), -1);
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
        /*changing boat rotation*/
        windAngle->setValues(currentBoat->getHeading(),currentBoat->getWindDir(),
                             currentBoat->getWindSpeed(), computeWPdir(currentBoat), heading);
    }
    isComputing=false;
}

void boardVLM::update_btnWP(void)
{
    if(!currentBoat)
        return;

    float WPLat = currentBoat->getWPLat();
    float WPLon = currentBoat->getWPLon();
    float WPHd = currentBoat->getWPHd();

    if(WPLat==0 && WPLon==0)
        btn_WP->setText("No WP");
    else
    {
        QString str = "WP: ";
        if(WPLat==0)
            str+="0 N";
        else
            str+=Util::pos2String(TYPE_LAT,WPLat);

        str+=", ";

        if(WPLon==0)
            str+="0 E";
        else
            str+=Util::pos2String(TYPE_LON,WPLon);

        if(WPHd!=-1)
        {
            str+=" @";
            str+=QString().setNum(WPHd);
            str+=tr("°");
        }

        btn_WP->setText(str);
    }
}

void boardVLM::doWP_edit()
{
    wpDialog->show_WPdialog(currentBoat);
}

void boardVLM::chgAngle()
{
    sendCmd(VLM_CMD_ANG,editAngle->value(),0,0);
}

void boardVLM::doSync()
{
    if(currentBoat)
    {
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
        emit VLM_Sync();
    }
}

void boardVLM::doVirer()
{
    float val = editAngle->value();
    editAngle->setValue(-val);
}

void boardVLM::doPilotOrtho()
{
    sendCmd(VLM_CMD_ORTHO,0,0,0);
}

void boardVLM::doVmg()
{
    sendCmd(VLM_CMD_VMG,0,0,0);
}

void boardVLM::disp_boatInfo()
{
    if(currentBoat)
    {
        QString polar_str=currentBoat->getCurrentPolarName();
        if(currentBoat->getPolarState())
            polar_str+= " ("+tr("forcé")+")";
        if(!currentBoat->getPolarData())
            polar_str+= " ("+tr("erreur")+")";

        QString boatID;

        if(currentBoat->getAliasState())
            boatID=currentBoat->getAlias() + " (" + currentBoat->getBoatId() + ")";
        else
            boatID=currentBoat->getLogin() + " (" + currentBoat->getBoatId() + ")";

        QMessageBox::information(this,tr("Information sur")+" " + boatID,
                             tr("Login:") + " " + currentBoat->getLogin() + "\n" +
                             (currentBoat->getAliasState()?tr("Alias:")+ " " + currentBoat->getAlias() + "\n":"") +
                             tr("ID:") + " " + currentBoat->getBoatId() + "\n" +
                             tr("Nom:") + " " + currentBoat->getBoatName() + "\n" +
                             tr("Email:") + " " + currentBoat->getEmail() + "\n" +
                             tr("Course:") + " (" + currentBoat->getRaceId() + ") " + currentBoat->getRaceName() + "\n" +
                             tr("Score:") + " " + currentBoat->getScore() + "\n" +
                             tr("Polaire:") + " " + polar_str
                             );
    }

}

#define CHKSUM(DATA) ({             \
    char __c=0;                     \
    char * __str=DATA.toAscii().data(); \
    while(*__str) {                 \
        __c^=*__str;                \
        __str++;                    \
    }                               \
    __c;                            \
})

void boardVLM::synch_GPS()
{
  if(!currentBoat)
      return;

  if(chk_GPS->isChecked())
  {
        QextSerialPort * port = new QextSerialPort(COM);
        port->setBaudRate(BAUD19200);
        port->setFlowControl(FLOW_OFF);
        port->setParity(PAR_NONE);
        port->setDataBits(DATA_8);
        port->setStopBits(STOP_1);

        port->open(QIODevice::ReadWrite);


        if(!port->isOpen())
        {
            qWarning("Serial Port not open");
            return;
        }

        port->setTimeout(0,100);

        QString data1;
        QString data;
        QString data2;
        char ch;
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

        /*preparing main content */
        data1.sprintf("%07.2f,%s,%08.2f,%s,",lat,currentBoat->getLat()<0?"S":"N",lon,currentBoat->getLon()<0?"W":"E");
        data2.sprintf("%05.1f,%05.1f,",currentBoat->getSpeed(),currentBoat->getHeading());

        /*sending it 10 times */
        for(int i=0;i<10;i++)
        {
            data="GPGLL,"+data1+now.toString("HHmmss")+",A";
            ch=(char)CHKSUM(data);
            data="$"+data+"*"+QString().setNum(ch,16);
            qWarning() << "GPS-GLL: " << data;
            data=data+"\x0D\x0A";
            if(port->write(data.toAscii(),data.length())!=data.length())
            {
                delete port;
                chk_GPS->setCheckState(Qt::Unchecked);
                QMessageBox::warning ( this, tr("GPS synchronisation"),
                    tr("Impossible d'envoyer les donnees sur le port serie"));
                return;
            }

            data="GPRMC,"+now.toString("HHmmss")+",A,"+data1+data2+now.toString("ddMMyy")+",000.0,E";
            ch=(char)CHKSUM(data);
            data="$"+data+"*"+QString().setNum(ch,16);
            qWarning() << "GPS-RMC: " << data;
            data=data+"\x0D\x0A";
            //port->write(data.toAscii(),data.length());
            if(port->write(data.toAscii(),data.length())!=data.length())
            {
                delete port;
                chk_GPS->setCheckState(Qt::Unchecked);
                QMessageBox::warning ( this, tr("GPS synchronisation"),
                    tr("Impossible d'envoyer les donnees sur le port serie"));
                return;
            }

            now.addSecs(1);
        }
        /* one last RMC to confirm speed */
        data="GPRMC,"+now.toString("HHmmss")+",A,"+data1+data2+now.toString("ddMMyy")+",000.0,E";
        ch=(char)CHKSUM(data);
        data="$"+data+"*"+QString().setNum(ch,16);
        qWarning() << "GPS-RMC: " << data;
        data=data+"\x0D\x0A";
        //port->write(data.toAscii(),data.length());
        if(port->write(data.toAscii(),data.length())!=data.length())
        {
            delete port;
            chk_GPS->setCheckState(Qt::Unchecked);
            QMessageBox::warning ( this, tr("GPS synchronisation"),
                tr("Impossible d'envoyer les donnees sur le port serie"));
            return;
        }

        delete port;
        /* we will send this again in 30 secs */
        GPS_timer->start(30*1000);
    }
}

void boardVLM::setChangeStatus(bool status)
{
    bool st=!status;
    btn_chgHeading->setEnabled(st);
    editHeading->setEnabled(st);
    btn_virer->setEnabled(st);
    btn_chgAngle->setEnabled(st);
    editAngle->setEnabled(st);    
    goPilotOrtho->setEnabled(st);
    goVMG->setEnabled(st);
    btn_WP->setEnabled(st);
    btn_Synch->setEnabled(!((MainWindow*)mainWin)->get_selPOI_instruction());
}

/*********************/
/* http requests     */

void boardVLM::updateInet(void)
{
    /* update connection */
    if(conn) conn->updateInet();
}

void boardVLM::sendCmd(int cmdNum,float  val1,float val2, float val3)
{
    if(conn && currentBoat && !isWaiting && conn->isAvailable())
    {
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
        currentCmdNum=cmdNum;
        cmd_val1=val1;
        cmd_val2=val2;
        cmd_val3=val3;
        QString page;
        QTextStream(&page)
                        << "/myboat.php?"
                        << "pseudo=" << currentBoat->getLogin()
                        << "&password=" << currentBoat->getPass()
                        << "&lang=fr&type=login"
                        ;

        qWarning() << "login for cmd " << cmdNum << "(" << cmd_val1 << ","
                                                 << cmd_val2 << ","
                                                 << cmd_val3 << ")";

        conn->doRequestGet(VLM_REQUEST_LOGIN,page);
    }
    else
    {
        qWarning() <<  "error sendCmd " << cmdNum << " - bad state";
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    }
}

void boardVLM::requestFinished (int currentRequest,QString)
{
    QString page;
    QNetworkRequest request;

    switch(currentRequest)
    {
        case VLM_REQUEST_LOGIN:
            switch(currentCmdNum)
            {
                case VLM_CMD_HD:
                    QTextStream(&page)
                                << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                << currentBoat->getBoatId().toInt()
                                << "&pilotmode=autopilot&boatheading="+QString().setNum(cmd_val1)
                            ;
                    break;
                case VLM_CMD_ANG:
                    QTextStream(&page)
                                << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                << currentBoat->getBoatId().toInt()
                                << "&pilotmode=windangle&pilotparameter="+QString().setNum(cmd_val1)
                            ;
                    break;
                case VLM_CMD_VMG:
                    QTextStream(&page)
                                << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                << currentBoat->getBoatId().toInt()
                                << "&pilotmode=bestvmg"
                            ;
                    break;
                case VLM_CMD_ORTHO:
                    QTextStream(&page)
                                << "/update_angle.php?expertcookie=yes&lang=fr&idusers="
                                << currentBoat->getBoatId().toInt()
                                << "&pilotmode=orthodromic"
                            ;
                    break;
                case VLM_CMD_WP:
                    QTextStream(&page)
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
            qWarning() << "Send cmd: " << page;

            conn->doRequestGet(VLM_DO_REQUEST,page);
            break;
        case VLM_DO_REQUEST:
            isWaiting=true;
            qWarning() << "Request done";
            /* update boat info */
            nbRetry=0;
            timer->start(1000);
            break;
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
            if(compFloat(data,cmd_val1) && currentBoat->getPilotType() == 1)
                done=true;
            break;
        case VLM_CMD_ANG:
            data = currentBoat->getTWA();
            val=currentBoat->getHeading()-currentBoat->getWindDir();
            calcAngleSign(val,data);

            if(((int)data) == ((int)cmd_val1) && currentBoat->getPilotType() == 2)
                done=true;
            break;
        case VLM_CMD_VMG:
            if(currentBoat->getPilotType() == 4)
                done=true;
            break;
        case VLM_CMD_ORTHO:
            if(currentBoat->getPilotType() == 3)
                done=true;
            break;
        case VLM_CMD_WP:
            if(compFloat(currentBoat->getWPLat(),cmd_val1) &&
                compFloat(currentBoat->getWPLon(),cmd_val2) &&
                compFloat(currentBoat->getWPHd(),cmd_val3))
                done=true;
            break;
    }
    if(done)
    {
        isWaiting=false;
        boatUpdated(currentBoat);
    }
    else
    {
        nbRetry++;
        if(nbRetry>MAX_RETRY)
        {
            qWarning("Failed to synch");
            isWaiting=false;
            btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 170, 0);"));
            return;
        }
        qWarning("Retry...");
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
/* Dialog WP            */

WP_dialog::WP_dialog(QWidget * parent) : QDialog(parent)
{
    setupUi(this);

    currentBoat=NULL;

    WP_conv_lat->setText("");
    WP_conv_lon->setText("");
}

void WP_dialog::show_WPdialog(boatAccount * boat)
{
    currentBoat=boat;

    initDialog(boat->getWPLat(),boat->getWPLon(),boat->getWPHd());

    exec();
}

void WP_dialog::show_WPdialog(POI * poi)
{
    if(poi)
        initDialog(poi->getLatitude(),poi->getLongitude(),poi->getWph());
    exec();
}

void WP_dialog::initDialog(float WPLat,float WPLon,float WPHd)
{
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
}

void WP_dialog::done(int result)
{
    if(result == QDialog::Accepted)
    {
        if(WP_lat->text().isEmpty() && WP_lon->text().isEmpty())
            sendCmd(VLM_CMD_WP,0,0,-1);
        else
            sendCmd(VLM_CMD_WP,WP_lat->text().toFloat(),WP_lon->text().toFloat(),
                    WP_heading->text().isEmpty()?-1:WP_heading->text().toFloat());
    }
    QDialog::done(result);
}

void WP_dialog::doClearWP()
{
    WP_lat->setText("");
    WP_lon->setText("");
    WP_heading->setText("");
}

void WP_dialog::chgLat()
{
    if(WP_lat->text().isEmpty())
        WP_conv_lat->setText("");
    else
    {
        float val = WP_lat->text().toFloat();
        WP_conv_lat->setText(Util::pos2String(TYPE_LAT,val));
    }
}

void WP_dialog::chgLon()
{
    if(WP_lon->text().isEmpty())
        WP_conv_lon->setText("");
    else
    {
        float val = WP_lon->text().toFloat();
        WP_conv_lon->setText(Util::pos2String(TYPE_LON,val));
    }
}

void WP_dialog::doPaste()
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

void WP_dialog::doCopy()
{
    if(!currentBoat)
        return;
    Util::setWPClipboard(WP_lat->text().toFloat(),WP_lon->text().toFloat(),
        WP_heading->text().toFloat());
    QDialog::done(QDialog::Rejected);

}

void WP_dialog::doSelPOI()
{
    emit selectPOI();
    QDialog::done(QDialog::Rejected);
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
    img_boat2 = new QImage("img/tool_boat_2.png");

    heading =windDir=windSpeed=0;
    WPdir = -1;
    newHeading=-1;
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

        /* rotate + new heading boat */
        if(newHeading!=-1 && !img_boat->isNull())
        {
            painter->rotate(newHeading);
            painter->drawImage(-img_boat2->width()/2,-img_boat2->height()/2,*img_boat2);
            painter->rotate(-newHeading);
        }

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

void tool_windAngle::setValues(float heading,float windDir, float windSpeed, float WPdir,float newHeading)
{
    this->heading=heading;
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->WPdir=WPdir;
    this->newHeading=newHeading;
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
