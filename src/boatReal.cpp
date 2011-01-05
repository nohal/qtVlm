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

#include <QTime>
#include <QDebug>
#include <QStringList>
#include <QGraphicsSceneEvent>

#include "DialogRealBoatPosition.h"

#include "settings.h"
#include "boatReal.h"
#include "dataDef.h"
#include "nmea.h"
#include "Util.h"

boatReal::boatReal(QString pseudo, bool activated, Projection * proj,MainWindow * main,
                   myCentralWidget * parent): boat(pseudo,activated,proj,main,parent)
{
    qWarning()<<"creating boat real";

    setData(0,BOATREAL_WTYPE);
    isMoving=false;

    this->boat_type=BOAT_REAL;
    setFont(QFont("Helvetica",9));

    /* init thread */
    gpsReader = new ReceiverThread(this);
    qWarning()<<"after creating thread";
    /* init nmea decoder */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
    if(parser.buff_size==0)
        qWarning()<<"no parser!!!";


    this->lat=0;
    this->lon=0;
    this->vacLen=300;
    trace=new vlmLine(proj,parent->getScene(),Z_VALUE_ESTIME);
    qWarning()<<"after creating trace for boatReal";
    previousLon=lon;
    previousLat=lat;
    QPen penTrace;
    penTrace.setColor(Qt::red);
    penTrace.setBrush(Qt::red);
    penTrace.setWidthF(0.5);
    qWarning()<<"before setNbVacPerHour";
    trace->setNbVacPerHour(3600/this->getVacLen());
    qWarning()<<"after setNbVacPerHour";
    trace->setLinePen(penTrace);
    qWarning()<<"after setLinePen";
    trace->slot_showMe();
    qWarning()<<"after boatReal show trace";
    gotPosition=false;
    this->WPLat=0;
    this->WPLon=0;
    this->speed=0;
    this->heading=0;
    this->windSpeed=-1;
    this->setStatus(true);
    this->vacLen=300;
    this->playerName=pseudo;
    changeLocked=false;
    forceEstime=false;
    //updateBoatData();

    myCreatePopUpMenu();
}

boatReal::~boatReal()
{
    qWarning()<<"inside ~boatReal";
    if(gpsReader && gpsReader->isRunning())
    {
        this->stopRead();
//        int dummy=0;/*trying to fix the crash on exit (not enough)*/
//        while(gpsReader->isRunning())
//            dummy++;
//        qWarning()<<"dummy="<<dummy;
        delete gpsReader;
    }
    if(trace && !parent->getAboutToQuit())
    {
        parent->getScene()->removeItem(trace);
        delete trace;
    }
    qWarning()<<"last instruction in ~boatReal";
}

void boatReal::myCreatePopUpMenu(void)
{
    ac_chgPos = new QAction("Definir la position",popup);
    popup->addAction(ac_chgPos);
    connect(ac_chgPos,SIGNAL(triggered()),this,SLOT(slot_chgPos()));
}

void boatReal::startRead()
{
    /* if running stop */
    stopRead();

    /* start loop */
    cnt=0;
    if(!gpsReader)
        gpsReader = new ReceiverThread(this);
    gpsReader->start();
}

void boatReal::stopRead()
{
    if(gpsReader)
    {
        if(gpsReader->isRunning())
        {
            qWarning() << "gpsThread running => kill it";
            emit terminateThread();
        }
    }
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

void boatReal::decodeData(QByteArray data)
{
    //data="$GPRMC,152051.718,A,4825.333333,N,00618.333333,W,0,0,151110,0,E,A*04";
    data.push_back('\r');
    data.push_back('\n');
    //qWarning()<<data<<"(size="<<data.size()<<")";
    char * record=data.data();
    nmea_parse(&parser, record, data.size(), &info);


    /*QWARN << cnt++ << ": Lat= " << nmea_ndeg2degree(info.lat) << " (" << info.lat << "), Lon= "
            << nmea_ndeg2degree(info.lon) << " (" << info.lon << "), Sig=" << info.sig << ", Fix=" << info.fix << ", mask=" << parseMask(info.smask);
    */
    qWarning()<<"speed:"<<info.speed<<"cap:"<<info.direction<<"lat:"<<info.lat;
    this->speed=info.speed/1.852;
    this->heading=info.direction;
    this->lat=nmea_ndeg2degree(info.lat);
    this->lon=nmea_ndeg2degree(info.lon);
    this->lastUpdateTime=QDateTime().currentDateTimeUtc().toTime_t();
    if(previousLon!=lon && previousLat!=lat)
        updateBoatData();
    if(gotPosition && !(previousLat==0 && previousLon==0)) /*a revoir*/
    {
        trace->addPoint(previousLat,previousLon);
        trace->slot_showMe();
    }
    gotPosition=true;
    previousLon=lon;
    previousLat=lat;
    emit boatUpdated(this,false,false);
}

void boatReal::setPosition(double lat, double lon)
{
    this->lat=lat;
    this->lon=lon;
    this->lastUpdateTime=QDateTime().currentDateTimeUtc().toTime_t();
    updateBoatData();
    emit boatUpdated(this,false,false);
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
    if(isMoving)
    {
        int new_x=x;
        int new_y=y-height/2;

        setPos(new_x,new_y);

        mouse_x=x;
        mouse_y=y;
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
    QFont myFont=QFont("Helvetica",9);
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

void boatReal::paramUpdated()
{
    gpsReader->initPort();
    startRead();
}

void boatReal::setPolar(QString polarName)
{
    this->polarName=polarName;
    boat::reloadPolar();
}

void boatReal::slot_chgPos(void)
{
    DialogRealBoatPosition dialog(parent);
    dialog.showDialog(this);
}

/****************************************************/
/* Receiver Thread                                  */
/****************************************************/

ReceiverThread::ReceiverThread(boatReal * parent)
{
    connect(parent,SIGNAL(terminateThread()),this,SLOT(terminate()));
    connect(this,SIGNAL(decodeData(QByteArray)),parent,SLOT(decodeData(QByteArray)));
    connect(this,SIGNAL(updateBoat(boat*,bool,bool)),parent,SIGNAL(boatUpdated(boat*,bool,bool)));
    connect(this,SIGNAL(finished()),parent,SLOT(slot_threadStartedOrFinished()));
    connect(this,SIGNAL(started()),parent,SLOT(slot_threadStartedOrFinished()));

    //port = new QextSerialPort();
    port=NULL;
    initPort();
    this->parent=parent;
    stop=false;
}

ReceiverThread::~ReceiverThread(void)
{
    if(port && port->isOpen())
    {
        port->close();
    }
    if(port)
        delete port;
}

bool ReceiverThread::initPort(void)
{
    /* close existing port */
    if(port && port->isOpen())
        port->close();
    if(!port)
        port=new QextSerialPort();
    /* init serial port */
    port->setPortName(Settings::getSetting("gpsPortName","COM1").toString());
    port->setBaudRate((BaudRateType)Settings::getSetting("gpsBaudRate",BAUD4800).toInt());
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    if(!port->open(QIODevice::ReadOnly))
    {
        qWarning() << "Can't open Serial port " << port->portName() << " (" << port->lastError() << ")";
        return false;
    }
    else
        return true;
}

void ReceiverThread::run()
{
    int numBytes = 0;
    int l = 512;

    if(!port || !port->isOpen())
    {
        /*retry to open the port*/
        this->initPort();
        if(!port || !port->isOpen())
        {
            qWarning() << "Port not open => not starting GPS thread";
            return;
        }
    }

    qWarning() << "Starting thread loop";

    stop=false;
    /*clear port*/
    while(port->bytesAvailable()>0)
        port->read(512);
    emit updateBoat((boat *) parent,false,false);
    while (!stop)
    {
        numBytes = port->bytesAvailable();
        if(numBytes > l)
        {
            QByteArray buffer;
            while(port->bytesAvailable()>0)
                buffer=buffer+port->read(512);
            QList<QByteArray> lines=buffer.split('\n');
            for (int n=0;n<lines.count();n++)
            {
                if(lines.at(n).count()!=0)
                {

                    //QString temp=QString(lines.at(n));
                    //if(temp.left(6)!="$GPRMC") continue;
                    emit decodeData(lines.at(n).left(lines.at(n).length()-1));
                }
            }
            emit updateBoat((boat *) parent,false,false);
        }
        qWarning()<<"sleeping";
#warning a mettre en parametre
        sleep(3);
    }
    port->close();
    qWarning() << "Exiting thread loop";
}

void ReceiverThread::terminate()
{
    stop=true;
}





/**************************************************************************************/
/*                                                                                    */
/*                     OLD CODE                                                       */
/*                                                                                    */
/**************************************************************************************/





#if 0
bool boatReal::chkData(QString data)
{
    char res=0;
    QStringList lst=data.split("*");
    char val;

    if(lst.size()!=2)
        return false;

    if(lst[0].at(0) != '$')
        return false;

    val=lst[1].toInt(0,16);

    for(int i=1;i<lst[0].length();i++)
        res^=lst[0].at(i).toAscii();

    //qWarning() << "chk comp=" << (int)res << " chk read" << (int)val;

    if(res!=val)
        return false;
    return true;
}

#define GET_POS(RECTYPE,DATA,NUM,VAL,TYPE) {   \
    bool _ok;                                \
    int _deg;                                \
    float _min;                              \
    if(DATA[NUM].size()==0) {                \
        VAL=0;                               \
    } else {                                 \
        QString _s1=(TYPE == TYPE_LON?"E":"N");  \
        QString _s2=(TYPE == TYPE_LON?"W":"S");  \
        VAL=DATA[NUM].toFloat(&_ok);             \
        if(!_ok || (DATA[NUM+1]!=_s1 && DATA[NUM+1]!=_s2))   \
        {                                                   \
            qWarning() << RECTYPE << ": field " << NUM            \
                    << " error reading " << (TYPE==TYPE_LON?"LON":"LAT" )  \
                    << ": " << DATA[NUM] << " " << DATA[NUM+1];    \
            return;                              \
        }                                        \
        _deg=(int)(VAL/100);                     \
        _min=VAL-_deg*100;                       \
        VAL=_deg+_min/60;                        \
        if(DATA[NUM+1]!=_s2)                     \
            VAL=-VAL;                            \
    }                                            \
}

#define GET_DATETIME(RECTYPE,DATA,NUM,V1,V2,V3,TYPE) { \
    bool _ok1,_ok2,_ok3;                          \
    if(DATA[NUM].size()!=6)                       \
    {                                             \
        qWarning() << RECTYPE << ": field " << NUM  \
            << " error (1) reading " << (TYPE==TYPE_DATE?"date":"time") \
            << ": " << DATA[NUM] ;                \
        return;                                   \
    }                                             \
    V1=DATA[NUM].mid(0,2).toInt(&_ok1);          \
    V2=DATA[NUM].mid(2,2).toInt(&_ok2);          \
    V3=DATA[NUM].mid(4,2).toInt(&_ok3);          \
    if(!_ok1 || !_ok2 || !_ok3)                   \
    {                                             \
        qWarning() << RECTYPE << ": field " << NUM  \
            << " error (2) reading " << (TYPE==TYPE_DATE?"date":"time") \
            << ": " << DATA[NUM] ;                \
        return;                                   \
    }                                             \
}

#define GET_DATE(RECTYPE,DATA,NUM,VAL) {   \
    int _v1,_v2,_v3;                                 \
    GET_DATETIME(RECTYPE,DATA,NUM,_v1,_v2,_v3,TYPE_DATE); \
    _v3=(_v3<=70?2000+_v3:1900+_v3);                 \
    VAL=QDate(_v3,_v2,_v1);                          \
}

#define GET_TIME(RECTYPE,DATA,NUM,VAL) {   \
    int _v1,_v2,_v3;                                 \
    GET_DATETIME(RECTYPE,DATA,NUM,_v1,_v2,_v3,TYPE_TIME); \
    VAL=QTime(_v1,_v2,_v3);                          \
}

#define GET_FLOAT(RECTYPE,DATA,NUM,VAL) { \
    bool _ok;                             \
    VAL=DATA[NUM].toFloat(&_ok);          \
    if(!_ok )                              \
    {                                                   \
        qWarning() << RECTYPE << ": field " << NUM      \
            << " error reading float:"                  \
             << DATA[NUM];                              \
        return;                           \
    }                                     \
}

#define GET_INT(RECTYPE,DATA,NUM,VAL) { \
    bool _ok;                             \
    VAL=DATA[NUM].toInt(&_ok);          \
    if(!_ok )                              \
    {                                                   \
        qWarning() << RECTYPE << ": field " << NUM      \
            << " error reading fint:"                  \
             << DATA[NUM];                              \
        return;                           \
    }                                     \
}

#define CHKSIZE(RECTYPE,DATA,RECSIZE) { \
    if((DATA.size()-1) != RECSIZE) {    \
        qWarning() << RECTYPE << ": Bad number of args : " \
            << DATA.size()-1 << " (required " \
            << RECSIZE << ")";          \
        return;                         \
    }                                   \
}


void boatReal::decodeData(QString data)
{
    if(!chkData(data))
    {
        qWarning() << "Error in data: " << data;
        return;
    }

    QStringList dataLst = data.split("*")[0].split(",");

    if(dataLst[0] == "$GPRMC")
    {
        QTime t;
        QDate d;
        float lat,lon;
        float spd;
        float angle;
        float mag;
        int   recValidity;
        /*check nb of param */
        CHKSIZE("RMC",dataLst,12);
        /* Get data */
        GET_TIME("RMC",dataLst,1,t);
        if(dataLst[2]!="A" && dataLst[2]!="V")
        {
            qWarning() << "RMC: field 2 / status - bad format:" << dataLst[2];
            return;
        }
        recValidity=dataLst[2]=="A"?REC_ACTIVE:REC_VOID;
        GET_POS("RMC",dataLst,3,lat,TYPE_LAT);
        GET_POS("RMC",dataLst,5,lon,TYPE_LON);
        GET_FLOAT("RMC",dataLst,7,spd);
        GET_FLOAT("RMC",dataLst,8,angle);
        GET_DATE("RMC",dataLst,9,d);
        GET_POS("RMC",dataLst,10,mag,TYPE_LON);

        /* Output data */
        qWarning() << "--------------- RMC";
        qWarning() << "Time: " << t << " Status: " << (recValidity==REC_ACTIVE?"Active":"Void");
        qWarning() << "Lat=" << lat << " - Lon=" << lon;
        qWarning() << "Speed=" << spd << " Angle=" << angle;
        qWarning() << "Date: " << d << " mag Variation: " << mag;
        qWarning() << "-------------------";
    }
    else if(dataLst[0] == "$GPGGA")
    {
        QTime t;
        float lat,lon,H_dilution,altitude,altitude_corr;
        int fixQuality,nbSat;
        QString altitude_unit;
        /*check nb of param */
        CHKSIZE("GGA",dataLst,14);
        /* Get data */
        GET_TIME("GGA",dataLst,1,t);
        GET_POS("GGA",dataLst,2,lat,TYPE_LAT);
        GET_POS("GGA",dataLst,4,lon,TYPE_LON);
        GET_INT("GGA",dataLst,6,fixQuality);
        GET_INT("GGA",dataLst,7,nbSat);
        GET_FLOAT("GGA",dataLst,8,H_dilution);
        GET_FLOAT("GGA",dataLst,9,altitude);
        altitude_unit=dataLst[10];
        GET_FLOAT("GGA",dataLst,11,altitude_corr);

        /* Output data */
        qWarning() << "--------------- GGA";
        qWarning() << "Time: " << t ;
        qWarning() << "Lat=" << lat << " - Lon=" << lon;
        qWarning() << "Fix Qual: " << fixQuality << " nb sat=" << nbSat << " DOP=" << H_dilution;
        qWarning() << "Altitude: " << altitude << " " << altitude_unit << " cor:" << altitude_corr;
        qWarning() << "-------------------";
    }

    else
    {
        //qWarning() << "UKN record: " << dataLst[0];
    }
}
#endif
