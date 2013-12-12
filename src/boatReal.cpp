/********************************************************************
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

#include <QTime>
#include <QDebug>
#include <QStringList>
#include <QGraphicsSceneEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>

#include "DialogRealBoatPosition.h"

#include "settings.h"
#include "boatReal.h"
#include "dataDef.h"
#include "nmea.h"
#include "Util.h"
#include "Orthodromie.h"
#include "Polar.h"
#include "orthoSegment.h"
#include "Projection.h"
#include "GpsReceiver.h"

boatReal::boatReal(QString pseudo, bool activated, Projection * proj,MainWindow * main,
                   myCentralWidget * parent): boat(pseudo,activated,proj,main,parent)
{
    qWarning()<<"creating boat real";

    setData(0,BOATREAL_WTYPE);
    isMoving=false;

    set_boatType(BOAT_REAL);
    setFont(QApplication::font());

    /* init thread */
    gpsReader = NULL;
    gpsReaderType=GPS_NONE;
    nmea_zero_INFO(&info);
    this->lat=0;
    this->lon=0;
    this->vacLen=300;
    trace=new vlmLine(proj,parent->getScene(),Z_VALUE_ESTIME);
    previousLon=lon;
    previousLat=lat;
    QPen penTrace;
    penTrace.setColor(Qt::red);
    penTrace.setBrush(Qt::red);
    penTrace.setWidthF(0.5);
    trace->setNbVacPerHour(3600/this->getVacLen());
    trace->setLinePen(penTrace);
    trace->slot_showMe();
    gotPosition=false;
    WP=QPointF(0,0);
    this->speed=0;
    this->heading=0;
    this->windSpeed=-1;
    this->setStatus(true);
    this->playerName=pseudo;
    this->fix=1;
    this->sig=0;
    this->eta=-1;
    changeLocked=false;
    forceEstime=false;
    this->declinaison=Settings::getSetting("declinaison",0).toDouble();
    this->minSpeedForEngine=Settings::getSetting("minSpeedForEngine",0).toDouble();
    this->speedWithEngine=Settings::getSetting("speedWithEngine",4).toDouble();

    myCreatePopUpMenu();
    connect(this->popup,SIGNAL(aboutToShow()),parent,SLOT(slot_resetGestures()));
    connect(this->popup,SIGNAL(aboutToHide()),parent,SLOT(slot_resetGestures()));
    this->lastUpdateTime=QDateTime().currentDateTimeUtc().toTime_t();
    this->displayNMEA=false;
    this->pause=true;
    hide();
}

boatReal::~boatReal() {
    if(gpsReader && gpsReader->isRunning())
        this->stopRead();

    if(gpsReader) delete gpsReader;

    if( !parent->getAboutToQuit() )
        if(trace) {
            parent->getScene()->removeItem(trace);
            delete trace;
        }
}

void boatReal::setWP(QPointF point,double w) {
    setWP(point.y(),point.x(),w);
}

void boatReal::setWP(double la, double lo, double w)
{
    WP=QPointF(lo,la);
    this->WPHd=w;
    Orthodromie orth(this->lon,this->lat,lo,la);
    this->dnm=qRound(orth.getDistance()*100)/100.00;
    this->loxo=qRound(orth.getLoxoCap()*100)/100.00;
    this->ortho=qRound(orth.getAzimutDeg()*100)/100.00;
    if(loxo<0)
        loxo+=360;
    if(ortho<0)
        ortho+=360;
    this->vmg=qRound(this->speed*cos(degToRad(qAbs(this->heading-this->loxo)))*100)/100.00;
    if(vmg<=0)
        eta=-1;
    else
    {
        eta=QDateTime::currentDateTimeUtc().toTime_t();
        eta=eta+(3600/vmg)*dnm;
    }
    this->drawEstime();
}

void boatReal::myCreatePopUpMenu(void)
{
    ac_chgPos = new QAction(tr("Definir la position"),popup);
    popup->addAction(ac_chgPos);
    connect(ac_chgPos,SIGNAL(triggered()),this,SLOT(slot_chgPos()));
}

void boatReal::startRead() {
    int curDeviceType=Settings::getSetting("DeviceType",GPS_SERIAL,"GPS").toInt();
    /* start loop */
    cnt=0;
    if(gpsReader && gpsReader->get_deviceType()!=curDeviceType) {
        if(gpsReader->isRunning())
            this->stopRead();
        delete gpsReader;
        gpsReader=NULL;
    }

    if(!gpsReader) {
        switch(curDeviceType) {
            case GPS_NONE:
                return;
            case GPS_SERIAL:
                gpsReader = new SerialReceiverThread(this);
                break;
            case GPS_FILE:
                gpsReader = new FileReceiverThread(this);
                break;
            case GPS_GPSD:
                gpsReader = new GPSdReceiverThread(this);
                break;
        }
    }


    if(!gpsReader->isRunning())
    {
        if(!gpsReader->initDevice())
            return;
    }

    this->pause=false;

    if(!gpsReader->isRunning())
        gpsReader->start();
}

void boatReal::stopRead()
{
    this->pause=true;
    if(gpsReader)
        gpsReader->wait();
}
void boatReal::slot_threadStartedOrFinished()
{
    emit boatUpdated(this,false,false);
}

bool boatReal::gpsIsRunning()
{
    if(!gpsReader)
        return false;
    if(!gpsReader->isRunning())
        return false;
    return true;
}

void boatReal::updateBoat(nmeaINFO info)
{
    this->info=info;
    QDateTime now;
    now=now.currentDateTime();
//    QString s=now.toString("hh:mm:ss");
//    qWarning()<<s<<
//        "speed:"<<info.speed<<"cap:"<<info.direction<<"lat:"<<info.lat;
    this->fix=info.fix;
    this->sig=info.sig;
    if(info.declination!=0)
        this->declinaison=info.declination;
    else
        this->declinaison=Settings::getSetting("declinaison",0).toDouble();
    this->pdop=info.PDOP;
    if(sig<0 || sig>3)
        qWarning()<<"strange sig value:"<<sig;
    this->speed=info.speed/1.852;
    this->heading=info.direction;
    if(info.fix==1)
    {
//        qWarning()<<"bad gps signal, fix="<<info.fix<<",sig="<<info.sig;
        emit(boatUpdated(this,false,false));
        return;
    }
    this->lat=nmea_ndeg2degree(info.lat);
    this->lon=nmea_ndeg2degree(info.lon);
    this->lastUpdateTime=QDateTime().currentDateTimeUtc().toTime_t();
    if(previousLon!=lon || previousLat!=lat)
        updateBoatData();
    if(gotPosition && (previousLon!=lon || previousLat!=lat))
    {
        trace->addPoint(lat,lon);
        trace->slot_showMe();
    }
    gotPosition=true;
    previousLon=lon;
    previousLat=lat;
    this->setWP(WP,WPHd);
    emit(boatUpdated(this,false,false));
}

void boatReal::setPosition(double lat, double lon)
{
    this->lat=lat;
    this->lon=lon;
    this->lastUpdateTime=QDateTime().currentDateTimeUtc().toTime_t();
    this->setWP(WP,WPHd);
    updateBoatData();
    this->parent->emitUpdateRoute(this);
}

QString boatReal::parseMask(int mask)
{
    QString data;
    if(mask & GPGGA) data.append("GGA "); else data.append("    ");
    if(mask & GPGSA) data.append("GSA "); else data.append("    ");
    if(mask & GPGSV) data.append("GSV "); else data.append("    ");
    if(mask & GPRMC) data.append("RMC "); else data.append("    ");
    if(mask & GPVTG) data.append("VTG "); else data.append("    ");
    return data;
}

void boatReal::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && e->modifiers()==Qt::ShiftModifier)
    {
         previousLon=lon;
         previousLat=lat;
         isMoving=true;
         mouse_x=e->scenePos().x();
         mouse_y=e->scenePos().y();
         setPos(mouse_x,mouse_y-height/2);
         setCursor(Qt::BlankCursor);
         update();
     }
}

bool boatReal::tryMoving(int x, int y)
{
    if(isMoving) {
        mouse_x=x;
        mouse_y=y;
        double newlon,newlat;
        proj->screen2map(mouse_x+3,mouse_y, &newlon, &newlat);
        setPosition(newlat,newlon);
        emit boatUpdated(this,false,false);
        this->parent->emitUpdateRoute(this);
        return true;
    }
    return false;
}

void boatReal::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(isMoving)
    {
        double newlon,newlat;
        if(e->modifiers()==Qt::ShiftModifier)
        {
            proj->screen2map(mouse_x+3,mouse_y, &newlon, &newlat);
        }
        else
        {
            newlon=previousLon;
            newlat=previousLat;
        }
        setPosition(newlat,newlon);
        isMoving=false;
        setCursor(Qt::PointingHandCursor);
        emit boatUpdated(this,false,false);
        this->parent->emitUpdateRoute(this);
        return;
    }

    if(e->pos().x() < 0 || e->pos().x()>width || e->pos().y() < 0 || e->pos().y() > height)
    {
        e->ignore();
        return;
    }
    if (e->button() == Qt::LeftButton)
    {
        emit clearSelection();
    }
}

/**************************/
/* Data update            */
/**************************/

void boatReal::updateBoatString()
{
    my_str=pseudo;

    //qWarning() << "Updating Boat pseudo: " << pseudo << " (my= " << my_str << ") for a real boat, status=" << this->isVisible();

     /* computing widget size */
    QFont myFont=QFont(QApplication::font());
    QFontMetrics fm(myFont);
    prepareGeometryChange();
    width = fm.width("_"+my_str+"_")+2;
    height = qMax(fm.height()+2,10);
}

void boatReal::updateAll(void)
{
    this->updateBoatData();
}

void boatReal::updateHint(void)
{
#if 0
    QString str;
    /* adding score */
    str = getScore() + " - Spd: " + QString().setNum(getSpeed()) + " - ";
    switch(getPilotType())
    {
        case 1: /*heading*/
            str += tr("Hdg") + ": " + getPilotString() + tr("deg");
            break;
        case 2: /*constant angle*/
            str += tr("Angle") + ": " + getPilotString()+ tr("deg");
            break;
        case 3: /*pilotortho*/
            str += tr("Ortho" ) + "-> " + getPilotString();
            break;
        case 4: /*VMG*/
            str += tr("BVMG") + "-> " + getPilotString();
            break;
       case 5: /*VBVMG*/
            str += tr("VBVMG") /*+ "-> " + getPilotString()*/;
            break;
    }
    QString desc;
    if(!polarData) desc=tr(" (pas de polaire chargee)");
    else if (polarData->getIsCsv()) desc=polarData->getName() + tr(" (format CSV)");
    else desc=polarData->getName() + tr(" (format POL)");
    str=str.replace(" ","&nbsp;");
    desc=desc.replace(" ","&nbsp;");
    setToolTip(desc+"<br>"+str);
#endif
    setToolTip(pseudo);
}
void boatReal::unSelectBoat(bool needUpdate)
{
    selected = false;
    if(needUpdate)
    {
        drawEstime();
        update();
        updateTraceColor();
    }
}


void boatReal::setPolar(QString polarName)
{
    this->polarName=polarName;
    boat::reloadPolar(true);
}

void boatReal::slot_chgPos(void)
{
    DialogRealBoatPosition dialog(parent);
    dialog.showDialog(this);
}
