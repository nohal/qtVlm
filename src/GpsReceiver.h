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

#ifndef GPSRECEIVER_H
#define GPSRECEIVER_H

#include <QThread>
#include <QListWidget>
#include <QFile>

#include <qextserialport.h>
#include <nmea.h>

#include "class_list.h"
#include "dataDef.h"

enum {
    GPS_NONE=0,
    GPS_SERIAL,
    GPS_FILE,
    GPS_GPSD,
    GPS_MAX_TYPE
};

class ReceiverThread : public QThread
{ Q_OBJECT
    public:
        ReceiverThread(boatReal * parent);

        virtual void run(void) = 0;

        virtual bool initDevice(void);

        FCT_GET(int,deviceType)

    public slots:
        void copyClipBoard();

    signals:
        void decodeData(QByteArray data);
        void updateBoat(nmeaINFO info);

    protected:
        int deviceType;
        boatReal *parent;
        nmeaINFO info;
        nmeaPARSER parser;
        QListWidget * listNMEA;

};

class FileReceiverThread : public ReceiverThread
{ Q_OBJECT
    public:
        FileReceiverThread(boatReal * parent);
        ~FileReceiverThread(void);
        void run(void);

        bool initDevice(void);

    private:
        QFile file;
};

class SerialReceiverThread : public ReceiverThread
{ Q_OBJECT
    public:
        SerialReceiverThread(boatReal * parent);
        ~SerialReceiverThread(void);
        void run();

        bool initDevice(void);

    private:
        QextSerialPort * port;
};

class GPSdReceiverThread : public ReceiverThread
{ Q_OBJECT
    public:
        GPSdReceiverThread(boatReal * parent);
        ~GPSdReceiverThread(void);
        void run();

        bool initDevice(void);

    signals:
        void decodeData(QByteArray data);
        void updateBoat(nmeaINFO info);

    private:

};


#endif // GPSRECEIVER_H
