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

#include "boatReal.h"
#include "dataDef.h"
#include "nmea.h"

boatReal::boatReal(QString name, bool activated, Projection * proj,MainWindow * main,
                   myCentralWidget * parent): boat(name,activated,proj,main,parent)
{
    this->boat_type=BOAT_REAL;
    /* init thread */
    gpsReader = new ReceiverThread();
    connect(this,SIGNAL(terminateThread()),gpsReader,SLOT(terminate()));
    connect(gpsReader,SIGNAL(decodeData(QString)),this,SLOT(decodeData(QString)));

    /* init nmea decoder */
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    /* other init */
    setParam(name,activated);
}

boatReal::~boatReal()
{
    delete gpsReader;
}

void boatReal::readData()
{
    /* start/stop timer loop */

    if(gpsReader->isRunning())
    {
        emit terminateThread();
    }
    else
    {
        cnt=0;
        gpsReader->start();
    }

}

void boatReal::decodeData(QString data)
{
    char * record=data.toAscii().data();
    nmeaPOS dpos;
    nmea_parse(&parser, record, data.size(), &info);

    qWarning() << cnt++ << ": Lat= " << info.lat << ", Lon= " << info.lon
            << ", Sig=" << info.sig << ", Fix=" << info.fix << ", mask=" << parseMask(info.smask);
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

/**************************/
/* Data update            */
/**************************/

void boatReal::updateBoatName()
{
#if 0
    if(getAliasState())
        my_str=(alias.isEmpty()?login:alias);
    else
        my_str=login;

    //qWarning() << "Updating Boat name: " << login;

     /* computing widget size */
    QFontMetrics fm(font());
    prepareGeometryChange();
    width = fm.width(my_str) + 10 + 2;
    height = qMax(fm.height()+2,10);
#endif
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
            str += tr("Hdg") + ": " + getPilotString() + tr("°");
            break;
        case 2: /*constant angle*/
            str += tr("Angle") + ": " + getPilotString()+ tr("°");
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
    if(!polarData) desc=tr(" (pas de polaire charg�e)");
    else if (polarData->getIsCsv()) desc=polarData->getName() + tr(" (format CSV)");
    else desc=polarData->getName() + tr(" (format POL)");
    str=str.replace(" ","&nbsp;");
    desc=desc.replace(" ","&nbsp;");
    setToolTip(desc+"<br>"+str);
#endif
}


void boatReal::my_selectBoat(void)
{

}

void boatReal::my_unselectBoat(bool)
{

}

/****************************************************/
/* Receiver Thread                                  */
/****************************************************/

ReceiverThread::ReceiverThread()
{
    /* init serial port */
    port = new QextSerialPort("/dev/ttyUSB0");
    port->setBaudRate(BAUD4800);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    if(!port->open(QIODevice::ReadWrite))
        qWarning() << "Can't open Serial port (" << port->lastError() << ")";
    stop=false;
}

ReceiverThread::~ReceiverThread(void)
{

}

void ReceiverThread::run()
{
    int numBytes = 0;
    int l = 512;

    qWarning() << "Starting thread loop";

    if(!port || !port->isOpen())
    {
        qWarning() << "Port not open => exiting thread";
        return;
    }

    stop=false;

    while (!stop)
    {
        numBytes = port->bytesAvailable();
        if(numBytes > l) {
            if(numBytes > l) numBytes = l;
            char buff[l];
            qint64 lineLength = port->readLine(buff, sizeof(buff));
            emit decodeData(QString(buff));
        }
    }
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
