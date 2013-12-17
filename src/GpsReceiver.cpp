/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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
#include <QDebug>
#include <QClipboard>

#include <errno.h>

#include "GpsReceiver.h"
#include "boatReal.h"

#include "settings.h"

/****************************************************/
/* Receiver Thread                                  */
/****************************************************/

ReceiverThread::ReceiverThread(boatReal * parent) {
    this->parent=parent;

    qRegisterMetaType<GpsData>("GpsData");

    connect(this,SIGNAL(updateBoat(GpsData)),parent,SLOT(updateBoat(GpsData)));
    connect(this,SIGNAL(finished()),parent,SLOT(slot_threadStartedOrFinished()));
    connect(this,SIGNAL(started()),parent,SLOT(slot_threadStartedOrFinished()));

    clearGpsData(&gpsData);

    this->listNMEA=NULL;
}

bool ReceiverThread::initDevice(void) {
    if(parent->getDisplayNMEA()) {
        QDialog *NMEA=new QDialog();
        NMEA->setMinimumSize(600,300);
        NMEA->setWindowFlags(Qt::WindowStaysOnTopHint);
        NMEA->setModal(false);
        QGridLayout  *lay = new QGridLayout(NMEA);
        listNMEA=new QListWidget();
        listNMEA->setSelectionMode(QAbstractItemView::ContiguousSelection);
        lay->addWidget(listNMEA);
        QPushButton * clipBoard=new QPushButton(tr("Copier"));
        connect(clipBoard,SIGNAL(clicked()),this,SLOT(copyClipBoard()));
        lay->addWidget(clipBoard);
        NMEA->show();
    }
    return true;
}

void ReceiverThread::copyClipBoard() {
    QStringList liste;
    for (int i=0;i<listNMEA->count();++i)
        liste<<listNMEA->item(i)->text();
    QApplication::clipboard()->setText(liste.join("\n"));
}

/***************************
 * File instance         *
 ***************************/

FileReceiverThread::FileReceiverThread(boatReal * parent): ReceiverThread(parent) {
    deviceType=GPS_FILE;

    qRegisterMetaType<nmeaINFO>("nmeaINFO");
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
    if(parser.buff_size==0)
        qWarning()<<"no parser!!!";
}

FileReceiverThread::~FileReceiverThread(void) {

}

void FileReceiverThread::run(void) {
    QString fileName=Settings::getSetting("FileName","fakeGPS.data","GPS_FILE").toString();
    QFile fakeGPS(fileName);
    if(!fakeGPS.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning()<<"failed to open " << fileName;
        return;
    }

    clearGpsData(&gpsData);

    while (!parent->getPause() && fakeGPS.isOpen())
    {
        QByteArray buffer;

        buffer.clear();
        for(int pass=0;pass<50;++pass)
        {
            if(fakeGPS.atEnd())
            {
                fakeGPS.seek(0);
                qWarning()<<"restarting fakeGPS file";
            }
            buffer=fakeGPS.readLine();

            QList<QByteArray> lines=buffer.split('\n');
            for (int n=0;n<lines.count();++n)
            {
                if(lines.at(n).count()>1)
                {

                    QString temp=lines.at(n);

                    QByteArray data=lines.at(n);
                    data.push_back('\r' );
                    data.push_back('\n');
                    char * record=data.data();
                    nmea_parse(&parser, record, data.size(), &info);

                    nmea2gps(&info,&gpsData);

                    if(listNMEA)
                        listNMEA->addItem(temp);
                }
            }
            emit updateBoat(gpsData);

        }
        this->msleep(3000);
    }

    //qWarning() << "Exiting thread loop";
    this->exit();
}

bool FileReceiverThread::initDevice(void) {
    return ReceiverThread::initDevice();
}

/***************************
 * Serial instance         *
 ***************************/

SerialReceiverThread::SerialReceiverThread(boatReal * parent): ReceiverThread(parent) {
    port=NULL;
    deviceType=GPS_SERIAL;

    qRegisterMetaType<nmeaINFO>("nmeaINFO");
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
    if(parser.buff_size==0)
        qWarning()<<"no parser!!!";
}

SerialReceiverThread::~SerialReceiverThread(void) {
    if(port && port->isOpen())
    {
        port->close();
    }
    if(port)
        delete port;
    port=NULL;
}

void SerialReceiverThread::run(void) {
    int numBytes = 0;
    int l = 512;

    if(!port ||(port && !port->isOpen()))
    {
        qWarning()<<"anomaly 1";
        if(port)
        {
            delete port;
            port=NULL;
        }
        /*retry to open the port*/
        if(!this->initDevice())
        {
            qWarning() << "Port not open => not starting GPS thread";
            if(port) port->close();
            delete port;
            port=NULL;
            exit();
            return;
        }
    }

    clearGpsData(&gpsData);

    while (!parent->getPause() && port && port->isOpen())
    {

        numBytes = port->bytesAvailable();

        if(numBytes > l)
        {
            QByteArray buffer;
            while(port->bytesAvailable()>0)
                buffer=buffer+port->read(512);
            QList<QByteArray> lines=buffer.split('\n');
            for (int n=0;n<lines.count();++n)
            {
                if(lines.at(n).count()>1)
                {
                    QString temp=QString(lines.at(n).left(lines.at(n).length()-1));
                    QByteArray data=lines.at(n).left(lines.at(n).length()-1);
                    data.push_back('\r' );
                    data.push_back('\n');
                    char * record=data.data();
                    nmea_parse(&parser, record, data.size(), &info);
                    nmea2gps(&info,&gpsData);
                    if(listNMEA)
                    {
                        listNMEA->addItem(temp);
                    }
                }
            }
            emit updateBoat(gpsData);
        }
        this->msleep(3000);
    }

    if(port)
    {
        if(port->isOpen())
            port->close();
        delete port;
        port=NULL;
    }

    //qWarning() << "Exiting thread loop";
    this->exit();
}

bool SerialReceiverThread::initDevice(void) {
    /* close existing port */
    if(port==NULL)
        port=new QextSerialPort();
    if(port && port->isOpen())
        port->close();
    /* init serial port */
#ifdef __WIN_QTVLM
    port->setPortName("\\\\.\\"+Settings::getSetting("gpsPortName","COM1","GPS_SERIAL").toString());
#else
    port->setPortName(Settings::getSetting("gpsPortName","COM1","GPS_SERIAL").toString());
#endif
    port->setBaudRate((BaudRateType)Settings::getSetting("gpsBaudRate",BAUD4800,"GPS_SERIAL").toInt());
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    if(!port->open(QIODevice::ReadOnly | QIODevice::Unbuffered))
    {
        QMessageBox::critical (0,
            tr("Activation du GPS"),
            tr("Impossible d'ouvrir le port ")+Settings::getSetting("gpsPortName","COM1","GPS_SERIAL").toString()+"\n"+tr("Erreur: ")+port->lastError());
        return false;
    }

    return ReceiverThread::initDevice();
}

/***************************
 * Gpsd instance         *
 ***************************/
#ifdef __UNIX_QTVLM
GPSdReceiverThread::GPSdReceiverThread(boatReal * parent): ReceiverThread(parent) {
    deviceType=GPS_GPSD;
    ok=false;
}

GPSdReceiverThread::~GPSdReceiverThread(void) {

}

void GPSdReceiverThread::run(void) {
    if(!isOk()) {
        qWarning() << "GPSD device not ok";
        if(!this->initDevice()) {
            qWarning() << "Can't initDevice";
            exit();
        }
    }

    while (!parent->getPause() && isOk()) {
        if (!gps_waiting(&gps_data,500))
            continue;

        qWarning() << "We have data";

        if (gps_read(&gps_data) == -1) {
            qWarning() << "Read error.\n";
            break ;
        }


        if(gps_data.status==STATUS_NO_FIX) {
            qWarning() << "Bad fix";
            continue;
        }
        /* position ... */
        qWarning() << "Data ok";
        gpsData.latitude=gps_data.fix.latitude;
        gpsData.longitude=gps_data.fix.longitude;
        gpsData.declination=0;
        gpsData.elevation=gps_data.fix.altitude;
        gpsData.direction=gps_data.fix.track;
        gpsData.speed=gps_data.fix.speed;
        gpsData.fix=gps_data.fix.mode;
        gpsData.HDOP=gps_data.dop.hdop;
        gpsData.PDOP=gps_data.dop.pdop;
        gpsData.VDOP=gps_data.dop.vdop;
        gpsData.sig=gps_data.status;

        gpsData.inuse=gps_data.satellites_used;
        gpsData.inview=gps_data.satellites_visible;
        for(int i=0; i<MAXCHANNELS ;++i) {
            gpsData.sat[i].azimuth=gps_data.azimuth[i];
            gpsData.sat[i].elevation=gps_data.elevation[i];
            gpsData.sat[i].id=gps_data.PRN[i];
            gpsData.sat[i].in_use=gps_data.used[i];
            gpsData.sat[i].sigQ=gps_data.ss[i];
        }
        emit updateBoat(gpsData);

    }

    gps_stream(&gps_data,WATCH_DISABLE,NULL);
    gps_close(&gps_data);
    ok=false;
    exit();
}

bool GPSdReceiverThread::initDevice(void) {
    if(isOk()) {
        gps_stream(&gps_data,WATCH_DISABLE,NULL);
        gps_close(&gps_data);
    }

    memset(&gps_data,0,sizeof(struct gps_data_t));
    ok=false;

    if(gps_open("localhost", DEFAULT_GPSD_PORT,&gps_data)==-1) {
        qWarning() << "Error opening gpsd connection: " << errno << ": " << gps_errstr(errno);
        return false;
    }

    gps_stream(&gps_data,WATCH_ENABLE|WATCH_JSON,NULL);

    if(ReceiverThread::initDevice())
        ok=true;

    return ok;
}
#endif
