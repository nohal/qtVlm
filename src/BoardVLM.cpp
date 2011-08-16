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
/* QJson */
#include <serializer.h>
#include <parser.h>

#include "dataDef.h"

#include "BoardVLM.h"
#include "boatVLM.h"
#include "Board.h"
#include <qextserialport.h>
#include "Orthodromie.h"
#include "Util.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "settings.h"
#include "Polar.h"
#include "POI.h"
#include "DialogWp.h"

boardVLM::boardVLM(MainWindow * mainWin, inetConnexion * inet, board * parent) : QWidget(mainWin), inetClient(inet)
{
    setupUi(this);
    Util::setFontDialog(this);
    isComputing = false;
    this->mainWin = mainWin;
    this->parent=parent;

    needAuth=true;


    connect(this,SIGNAL(VLM_Sync()),mainWin,SLOT(slotVLM_Sync()));    
    connect(this,SIGNAL(POI_selectAborted(POI*)),mainWin,SLOT(slot_POIselected(POI*)));

    GPS_timer = new QTimer(this);
    GPS_timer->setSingleShot(true);
    connect(GPS_timer,SIGNAL(timeout()),this, SLOT(synch_GPS()));

    /* wpDialog */
    wpDialog = new DialogWp();
    connect(wpDialog,SIGNAL(confirmAndSendCmd(QString,QString,int,float,float,float)),
            this,SLOT(confirmAndSendCmd(QString,QString,int,float,float,float)));
    connect(wpDialog,SIGNAL(selectPOI()),mainWin,SLOT(slotSelectWP_POI()));
    connect(mainWin,SIGNAL(editWP_POI(POI*)),wpDialog,SLOT(show_WPdialog(POI *)));

    /* edit field keyPress */
    connect(editHeading,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));
    connect(editAngle,SIGNAL(hasEvent()),this,SLOT(edtSpinBox_key()));

    /* ugly hack to have degree char*/
    QString str;
    str.sprintf("%c",176);
    deg_unit_1->setText(str);
    deg_unit_3->setText(str);
    deg_unit_2->setText(str);
    deg_unit_5->setText(str);
    deg_unit_6->setText(str);

    default_styleSheet = btn_chgHeading->styleSheet();

    btn_WP->setText(tr("Prochaine balise (0 WP)"));

    isWaiting=false;

    btn_Pilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    connect(btn_Pilototo,SIGNAL(clicked()),mainWin, SLOT(slotPilototo()));

    chk_GPS->setEnabled(Settings::getSetting("gpsEmulEnable", "0").toString()=="1");
    if(!chk_GPS->isEnabled())
    {
        chk_GPS->setCheckState(Qt::Unchecked);
        chk_GPS->hide();
    }
    else
        chk_GPS->show();

    COM=Settings::getSetting("serialName", "COM2").toString();

    /* Contextual Menu */
    popup = new QMenu(this);
    ac_showHideCompass = new QAction(tr("Cacher compas"),popup);
    ac_angle1 = new QAction(tr("Angles au degre"),popup);
    ac_angle10 = new QAction(tr("Angles au dixieme"),popup);
    ac_angle100 = new QAction(tr("Angles au centieme"),popup);
    ac_angle1->setCheckable(true);
    ac_angle10->setCheckable(true);
    ac_angle100->setCheckable(true);
    if(Settings::getSetting("anglePrecision",1)==0)
    {
        ac_angle1->setChecked(true);
        editHeading->setSingleStep(1.0);
        editAngle->setSingleStep(1.0);
    }
    else if (Settings::getSetting("anglePrecision",1)==1)
    {
        ac_angle10->setChecked(true);
        editHeading->setSingleStep(0.1);
        editAngle->setSingleStep(0.1);
    }
    else
    {
        ac_angle100->setChecked(true);
        editHeading->setSingleStep(0.01);
        editAngle->setSingleStep(0.01);
    }
    popup->addAction(ac_showHideCompass);
    popup->addSeparator();
    popup->addAction(ac_angle1);
    popup->addAction(ac_angle10);
    popup->addAction(ac_angle100);
    connect(ac_showHideCompass,SIGNAL(triggered()),this,SLOT(slot_hideShowCompass()));
    connect(ac_angle1,SIGNAL(triggered()),this,SLOT(slot_angle1()));
    connect(ac_angle10,SIGNAL(triggered()),this,SLOT(slot_angle10()));
    connect(ac_angle100,SIGNAL(triggered()),this,SLOT(slot_angle100()));

    /* Etat du compass */
    if(Settings::getSetting("boardCompassShown", "1").toInt()==1)
        windAngle->show();
    else
        windAngle->hide();
}

void boardVLM::confirmAndSendCmd(QString question,QString info,int cmdNum,float val1,float val2, float val3)
{
    if(confirmChange(question,info))
        sendCmd(cmdNum,val1,val2,val3);
}

bool boardVLM::confirmChange(QString question,QString info)
{    
    if(!currentBoat())
    {
        QMessageBox::warning(mainWin,tr("Erreur"),tr("Pas de bateau selectionne"));
        return false;
    }

    if(Settings::getSetting("askConfirmation","0").toInt()==0)
        return true;

    if(QMessageBox::question(mainWin,tr("Instruction pour ")+currentBoat()->getBoatPseudo(),
                             currentBoat()->getBoatPseudo()+": " + question,QMessageBox::Yes|QMessageBox::No,
                             QMessageBox::Yes)==QMessageBox::Yes)
    {
        emit showMessage(currentBoat()->getBoatPseudo()+": " + info,2000);
        return true;
    }
    return false;
}

void boardVLM::paramChanged(void)
{
    chk_GPS->setEnabled(Settings::getSetting("gpsEmulEnable", "0").toString()=="1");
    if(!chk_GPS->isEnabled())
    {
        chk_GPS->setCheckState(Qt::Unchecked);
        chk_GPS->hide();
    }
    else
        chk_GPS->show();
    COM=Settings::getSetting("serialName", "COM2").toString();
}

float boardVLM::computeWPdir(boatVLM * myBoat)
{
    float dirAngle;
    float WPLat = myBoat->getWPLat();
    float WPLon = myBoat->getWPLon();
    if(WPLat != 0 && WPLon != 0)
    {
        Orthodromie orth = Orthodromie(myBoat->getLon(),myBoat->getLat(),WPLon,WPLat);
        dirAngle = orth.getAzimutDeg();
    }
    else
        dirAngle=-1;
    return dirAngle;
}

boatVLM * boardVLM::currentBoat(void)
{
    if(parent)
    {
        if(parent->currentBoat()->getType()==BOAT_VLM)
            return (boatVLM*)parent->currentBoat();
        else
            return NULL;
    }
    else
        return NULL;
}

void boardVLM::boatUpdated(void)
{
    float angle_val;

    boatVLM * myBoat=currentBoat();

    //qWarning() << "boardVlm update boat";

    if(myBoat == NULL)
        return;

    isComputing = true;
    float val=myBoat->getHeading()-myBoat->getWindDir();

    if(myBoat->getPilotType() == 2)
        angle_val = myBoat->getPilotString().toFloat();
    else
    {
        angle_val = myBoat->getTWA();
        calcAngleSign(val,angle_val)
    }

    editHeading->setValue(myBoat->getHeading());
    editAngle->setValue(angle_val);

    w_dir->setText(QString().setNum(myBoat->getWindDir()));
    w_speed->setText(QString().setNum(myBoat->getWindSpeed()));
    loch->setText(QString().setNum(myBoat->getLoch()));
    speed->setText(QString().setNum(myBoat->getSpeed()));
    avg->setText(QString().setNum(myBoat->getAvg()));

    windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(), computeWPdir(myBoat), -1);
    bvmgU->setText(QString().setNum(myBoat->getBvmgUp(myBoat->getWindSpeed())));
    bvmgD->setText(QString().setNum(myBoat->getBvmgDown(myBoat->getWindSpeed())));

    boatName->setText(myBoat->getBoatPseudo());

    //boatName->setText(myBoat->getDispName());

    QString tt;
    boatScore->setText(myBoat->getScore()+" ("+tt.sprintf("%d",myBoat->getRank())+")");

    /* boat position */
    longitude->setText(Util::pos2String(TYPE_LON,myBoat->getLon()));
    latitude->setText(Util::pos2String(TYPE_LAT,myBoat->getLat()));

    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    isComputing = false;
    speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
    setChangeStatus(myBoat->getLockStatus());

    /* update WP button */
    update_btnWP();

    /* WP direction */
    dnm->setText(QString().setNum(myBoat->getDnm()));
    ortho->setText(QString().setNum(myBoat->getOrtho()));
    vmg->setText(QString().setNum(myBoat->getVmg()));

    angle_val=myBoat->getOrtho()-myBoat->getWindDir();
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
    goVBVMG->setStyleSheet(default_styleSheet);
    switch(myBoat->getPilotType())
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
        case 5: /*VMG*/
            goVBVMG->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 127);"));
            break;
    }

    /* send data as a GPS */
    synch_GPS();
}

void boardVLM::setWP(float lat,float lon,float wph)
{
//    QString debug;
//    debug=debug.sprintf("sending WPLon %.10f WPLat %.10f @WP %.10f",lon,lat,wph);
//    qWarning()<<debug;
    if(confirmChange(tr("Confirmer le changement du WP"),tr("WP change")))
       sendCmd(VLM_CMD_WP,lat,lon,wph);
}

void boardVLM::chgHeading()
{
    if(confirmChange(tr("Confirmer le mode pilotage 'Cap'"),tr("Mode de pilotage change en 'Cap'")))
        sendCmd(VLM_CMD_HD,editHeading->value(),0,0);
}

void boardVLM::headingUpdated(double heading)
{
    qWarning()<<"heading value changed"<<heading;
    if(!currentBoat()) /*no current boat, nothing to do*/
        return;

    //if(isComputing) return;
    this->editHeading->blockSignals(true);
    this->editAngle->blockSignals(true);
    isComputing=true;


    if((float)heading==currentBoat()->getHeading())
    {
        /* setting back to VLM value */
        speed->setText(QString().setNum(currentBoat()->getSpeed()));
        speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        float val=currentBoat()->getHeading()-currentBoat()->getWindDir();
        float angle = currentBoat()->getTWA();
        calcAngleSign(val,angle);
        editAngle->setValue(angle);
        /*changing boat rotation*/
        windAngle->setValues(currentBoat()->getHeading(),currentBoat()->getWindDir(),
                             currentBoat()->getWindSpeed(), computeWPdir(currentBoat()), -1);
        currentBoat()->drawEstime(heading,currentBoat()->getSpeed());
    }
    else
    {
        /* heading value has changed => compute angle */
        float angle=heading-currentBoat()->getWindDir();
        float newSpeed=0;
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
        if(currentBoat()->getPolarData())
        {
            newSpeed=currentBoat()->getPolarData()->getSpeed(currentBoat()->getWindSpeed(),angle);
            speed->setText(QString().setNum(((float)qRound(newSpeed*100))/100));
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            //qWarning() << "Angle=" << angle << " w spd=" << currentBoat()->getWindSpeed() << " => boat spd=" << newSpeed;
        }
        else
        {
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
        }
        /*changing boat rotation*/
        windAngle->setValues(currentBoat()->getHeading(),currentBoat()->getWindDir(),
                             currentBoat()->getWindSpeed(), computeWPdir(currentBoat()), heading);
        /* update estime */
        currentBoat()->drawEstime(heading,currentBoat()->getPolarData()?newSpeed:currentBoat()->getSpeed());
    }
    if(this->editHeading->value()!=heading)
        headingUpdated(this->editHeading->value());
    this->editHeading->blockSignals(false);
    this->editAngle->blockSignals(false);
    isComputing=false;
}

void boardVLM::angleUpdated(double angle)
{
    if(!currentBoat()) /*no current boat, nothing to do*/
        return;

    //if(isComputing) return;
    this->editHeading->blockSignals(true);
    this->editAngle->blockSignals(true);
    isComputing=true;

/* compute VLM angle */
    float val=currentBoat()->getHeading()-currentBoat()->getWindDir();
    float oldAngle=currentBoat()->getTWA();
    calcAngleSign(val,oldAngle)
    oldAngle=((float)qRound(oldAngle*100))/100;


    if(angle==oldAngle)
    {
/* setting back to VLM value */
        speed->setText(QString().setNum(currentBoat()->getSpeed()));
        editHeading->setValue(currentBoat()->getHeading());
        speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
        /*changing boat rotation*/
        windAngle->setValues(currentBoat()->getHeading(),currentBoat()->getWindDir(),
                             currentBoat()->getWindSpeed(), computeWPdir(currentBoat()), -1);
        currentBoat()->drawEstime(currentBoat()->getHeading(),currentBoat()->getSpeed());
    }
    else
    {
        /* angle has changed */
        /* compute heading */
        float heading = currentBoat()->getWindDir() + angle;
        float newSpeed=0;
        if(heading<0) heading+=360;
        else if(heading>360) heading-=360;
        editHeading->setValue(heading);
/* compute speed if a polar is known */
        if(currentBoat()->getPolarData())
        {
            newSpeed=currentBoat()->getPolarData()->getSpeed(currentBoat()->getWindSpeed(),angle);
            speed->setText(QString().setNum(((float)qRound(newSpeed*100))/100));
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));

            qWarning() << "Angle=" << angle << " w spd=" << currentBoat()->getWindSpeed() << " => boat spd=" << newSpeed;
        }
        else
        {
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            label_6->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
        }
        /*changing boat rotation*/
        windAngle->setValues(currentBoat()->getHeading(),currentBoat()->getWindDir(),
                             currentBoat()->getWindSpeed(), computeWPdir(currentBoat()), heading);
        /* update estime */
        currentBoat()->drawEstime(heading,currentBoat()->getPolarData()?newSpeed:currentBoat()->getSpeed());
    }
    isComputing=false;
    if(angle!=this->editAngle->value())
        this->angleUpdated(this->editAngle->value());
    this->editHeading->blockSignals(false);
    this->editAngle->blockSignals(false);
}

void boardVLM::update_btnWP(void)
{
    if(!currentBoat())
        return;
    if(isComputing) return;
    isComputing=true;

    float WPLat = currentBoat()->getWPLat();
    float WPLon = currentBoat()->getWPLon();
    float WPHd = currentBoat()->getWPHd();

    if(WPLat==0 && WPLon==0)
        btn_WP->setText(tr("Prochaine balise (0 WP)"));
    else
    {
        QString str = QString();
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
            str+=QString().sprintf("%.2f",WPHd);
            str+=tr("deg");
        }

        btn_WP->setText(str);
    }
    isComputing=false;
}

void boardVLM::doWP_edit()
{
    wpDialog->show_WPdialog(currentBoat());
}

void boardVLM::chgAngle()
{
    if(confirmChange(tr("Confirmer le mode pilotage 'Angle'"),tr("Mode de pilotage change en 'Angle'")))
        sendCmd(VLM_CMD_ANG,editAngle->value(),0,0);
}

void boardVLM::doSync()
{
    if(currentBoat())
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
    if(confirmChange(tr("Confirmer le mode 'Pilot Ortho'"),tr("Mode de pilotage change en 'Pilot Ortho'")))
        sendCmd(VLM_CMD_ORTHO,0,0,0);
}

void boardVLM::doVmg()
{
    if(confirmChange(tr("Confirmer le mode 'VMG'"),tr("Mode de pilotage change en 'VMG'")))
        sendCmd(VLM_CMD_VMG,0,0,0);
}

void boardVLM::doVbvmg()
{
    if(confirmChange(tr("Confirmer le mode 'VBVMG'"),tr("Mode de pilotage change en 'VBVMG'")))
        sendCmd(VLM_CMD_VBVMG,0,0,0);
}

void boardVLM::disp_boatInfo()
{
    if(currentBoat())
    {
        QString polar_str=currentBoat()->getCurrentPolarName();
        if(currentBoat()->getPolarData())
        {
            if(currentBoat()->getPolarState())
                polar_str+= " ("+tr("forcee")+")";
            if (currentBoat()->getPolarData()->getIsCsv())
                polar_str += " - format csv";
            else
                polar_str += " - format pol";
        }
        else
            polar_str+= " ("+tr("erreur chargement")+")";

        QString boatID=currentBoat()->getDispName();
        QMessageBox::information(this,tr("Information sur")+" " + boatID,
                             (currentBoat()->getAliasState()?tr("Alias:")+ " " + currentBoat()->getAlias() + "\n":"") +
                             tr("ID:") + " " + currentBoat()->getBoatId() + "\n" +
                             tr("Pseudo:") + " " + currentBoat()->getBoatPseudo() + "\n" +
                             tr("Nom:") + " " + currentBoat()->getBoatName() + "\n" +
                             tr("Email:") + " " + currentBoat()->getEmail() + "\n" +
                             tr("Course:") + " (" + currentBoat()->getRaceId() + ") " + currentBoat()->getRaceName() + "\n" +
                             tr("Score:") + " " + currentBoat()->getScore() + "\n" +
                             tr("Polaire:") + " " + polar_str
                             );
    }

}

char boardVLM::chkSum(QString data)
{
    char res=0;
    for(int i=0;i<data.length();i++)
        res^=data.at(i).toAscii();
    return res;
}


void boardVLM::synch_GPS()
{
  if(!currentBoat())
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

        QString data;
        QString data1;
        QString data2;
        QString TWD;
        QString TWA;
        QString TWS;
        char ch;
        float fTWA;
        float lat=qAbs(currentBoat()->getLat());
        int deg=((int)lat);
        lat=(lat-deg)*60;
        deg=deg*100;
        lat=lat+deg;
        float lon=qAbs(currentBoat()->getLon());
        deg=((int)lon);
        lon=(lon-deg)*60;
        deg=deg*100;
        lon=lon+deg;
        QDateTime now = QDateTime::currentDateTime();

        /*preparing main content */
        data1.sprintf("%07.2f,%s,%08.2f,%s,",lat,currentBoat()->getLat()<0?"S":"N",lon,currentBoat()->getLon()<0?"W":"E");
        data2.sprintf("%05.1f,%05.1f,",currentBoat()->getSpeed(),currentBoat()->getHeading());

        /*sending it 10 times */
        for(int i=0;i<10;i++)
        {
            data="GPGLL,"+data1+now.toString("HHmmss")+",A";
            ch=chkSum(data);
            data="$"+data+"*"+QString().setNum(ch,16);
            //qWarning() << "GPS-GLL: " << data;
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
            ch=chkSum(data);
            data="$"+data+"*"+QString().setNum(ch,16);
            //qWarning() << "GPS-RMC: " << data;
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
        ch=chkSum(data);
        data="$"+data+"*"+QString().setNum(ch,16);
        //qWarning() << "GPS-RMC: " << data;
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

        /* Sending TWD TWA and TWS , sentence $GPMWV,TWA,T,TWS,N*/
        fTWA = currentBoat()->getTWA();
        calcAngleSign((currentBoat()->getHeading()-currentBoat()->getWindDir()),fTWA)
        TWD.sprintf("%05.1f",currentBoat()->getWindDir());
        TWA.sprintf("%05.1f",(-1 * fTWA));
        TWS.sprintf("%05.1f",currentBoat()->getWindSpeed());
        data="GPMWV,"+TWA+",T,"+TWS+",N";
        ch=chkSum(data);
        data="$"+data+"*"+QString().setNum(ch,16);
        //qWarning() << "GPS-RMC: " << data;
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

        data="GPMWD,"+TWD+",T,"+TWS+",N";
        ch=chkSum(data);
        data="$"+data+"*"+QString().setNum(ch,16);
        //qWarning() << "GPS-RMC: " << data;
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
        GPS_timer->start(Settings::getSetting("GPS_DELAY",30).toInt()*1000);
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

void boardVLM::sendCmd(int cmdNum,double  val1,double val2, double val3)
{

    if(!hasInet() || hasRequest())
    {
        qWarning() <<  "error sendCmd " << cmdNum << " - bad state";
        btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
        return;
    }

    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    currentCmdNum=cmdNum;
    cmd_val1=QString().sprintf("%.10f",val1);
    cmd_val2=QString().sprintf("%.10f",val2);
    cmd_val3=QString().sprintf("%.10f",val3);
    QString url;
    QString data;
    QString phpScript;

    QVariantMap instruction;
    QVariantMap pip;
    instruction.insert("idu",currentBoat()->getBoatId().toInt());


    switch(currentCmdNum)
    {
        case VLM_CMD_HD:
            phpScript="pilot_set.php";
            instruction.insert("pim",1);
            instruction.insert("pip",cmd_val1);
            break;
        case VLM_CMD_ANG:
            phpScript="pilot_set.php";
            instruction.insert("pim",2);
            instruction.insert("pip",cmd_val1);
            break;
        case VLM_CMD_ORTHO:
            phpScript="pilot_set.php";
            instruction.insert("pim",3);
            break;
        case VLM_CMD_VMG:
            phpScript="pilot_set.php";
            instruction.insert("pim",4);
            break;
        case VLM_CMD_VBVMG:
            phpScript="pilot_set.php";
            instruction.insert("pim",5);
            break;
        case VLM_CMD_WP:
            /*phpScript="pilot_set.php";
            instruction.insert("pim",currentBoat()->getPilotType());*/
            phpScript="target_set.php";
            pip.insert("targetlat",cmd_val1);
            pip.insert("targetlong",cmd_val2);
            pip.insert("targetandhdg",cmd_val3);
            instruction.insert("pip",pip);
            break;
    }

    QJson::Serializer serializer;
    QByteArray json = serializer.serialize(instruction);

    QTextStream(&url) << "/ws/boatsetup/" << phpScript;

    QTextStream(&data) << "parms=" << json;
    QTextStream(&data) << "&select_idu=" << currentBoat()->getId();
    qWarning()<<"sending:"<<data;
    inetPost(VLM_DO_REQUEST,url,data);
}

void boardVLM::requestFinished (QByteArray res)
{    
    switch(getCurrentRequest())
    {
        case VLM_REQUEST_LOGIN:
            qWarning() << "Error: BoardVLM:requestFinished - res for request login";
            break;
        case VLM_DO_REQUEST:
            //qWarning() << "Request done";

            if(checkWSResult(res,"BoardVLM",mainWin))
                currentBoat()->slot_getData(true);
            else
                btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 170, 0);"));
            break;
    }
}

QString boardVLM::getAuthLogin(bool * ok=NULL)
{
    if(currentBoat())
    {
        if(ok) *ok=true;
        return currentBoat()->getAuthLogin();
    }
    else
    {
        if(ok) *ok=false;
        return QString();
    }
}

QString boardVLM::getAuthPass(bool * ok=NULL)
{
    if(currentBoat())
    {
        if(ok) *ok=true;
        return currentBoat()->getAuthPass();
    }
    else
    {
        if(ok) *ok=false;
        return QString();
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

void boardVLM::keyPressEvent ( QKeyEvent * event )
{
    if(event->key() == Qt::Key_Escape)
        emit POI_selectAborted(NULL);
}

void boardVLM::contextMenuEvent(QContextMenuEvent  *)
{
    if(windAngle->isVisible())
        ac_showHideCompass->setText(tr("Cacher le compas"));
    else
        ac_showHideCompass->setText(tr("Afficher le compas"));
    popup->exec(QCursor::pos());
}

void boardVLM::slot_hideShowCompass()
{
    setCompassVisible(!windAngle->isVisible());
}

void boardVLM::slot_angle1()
{
    editHeading->setSingleStep(1.0);
    editAngle->setSingleStep(1.0);
    ac_angle10->setChecked(false);
    ac_angle100->setChecked(false);
    Settings::setSetting("anglePrecision",0);
}

void boardVLM::slot_angle10()
{
    editHeading->setSingleStep(0.1);
    editAngle->setSingleStep(0.1);
    Settings::setSetting("anglePrecision",1);
    ac_angle1->setChecked(false);
    ac_angle100->setChecked(false);
}

void boardVLM::slot_angle100()
{
    editHeading->setSingleStep(0.01);
    editAngle->setSingleStep(0.01);
    Settings::setSetting("anglePrecision",2);
    ac_angle1->setChecked(false);
    ac_angle10->setChecked(false);
}


void boardVLM::setCompassVisible(bool status)
{
    if(status)
    {
        Settings::setSetting("boardCompassShown",1);
        windAngle->show();
        //this->setMaximumHeight(680);
    }
    else
    {
        Settings::setSetting("boardCompassShown",0);
        windAngle->hide();
        //this->setMaximumHeight(434);
    }
    //this->updateGeometry();
//    this->adjustSize();
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
        if(windSpeed!=-1)
        {
            painter->rotate(windDir);
            QPen curPen = painter->pen();
            painter->setPen(windSpeed_toColor());
            painter->fillRect(-2,-h/2,4,40,QBrush(windSpeed_toColor()));
            painter->setPen(curPen);
            painter->rotate(-windDir);
        }

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
    //qWarning() << "windAngle set: heading=" << heading << " windDir=" << windDir << " windSpeed=" << windSpeed << " WPdir=" << WPdir << " " << newHeading;
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
