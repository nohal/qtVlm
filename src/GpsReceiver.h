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

#include "class_list.h"
#include <QThread>
#include <QListWidget>
#include <QFile>
#include <QDateTime>
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <qextserialport.h>
#include <nmea.h>

#ifdef __UNIX_QTVLM
#include "libgps.h"
#include "gps.h"
#endif

#include "dataDef.h"

enum {
    GPS_NONE=0,
    GPS_SERIAL,
    GPS_FILE,
    GPS_GPSD,
    GPS_INTERNAL,
    GPS_MAX_TYPE
};

#define MAX_SAT 100

struct SatData
{
    int     id;         /**< Satellite PRN number */
    int     in_use;     /**< Used in position fix */
    int     elevation;        /**< Elevation in degrees, 90 maximum */
    int     azimuth;    /**< Azimuth, degrees from true north, 000 to 359 */
    int     sigQ;        /**< Signal, 00-99 dB */

};

struct GpsData{
    /* position */
    double latitude;    /** deg */
    double longitude;   /** deg */
    double  elevation;  /** m */
    double  speed;      /** kts */
    double  direction;  /** deg */
    double  declination;  /** deg */

    /* date */
    QDateTime time;

    /* Precision */
    double  PDOP;       /**< Position Dilution Of Precision */
    double  HDOP;       /**< Horizontal Dilution Of Precision */
    double  VDOP;       /**< Vertical Dilution Of Precision */

    /* status */
    int fix; /**< Operating mode, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */
    int sig; /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */

    /* sat */
    int     inuse;      /**< Number of satellites in use (not those in view) */
    int     inview;     /**< Total number of satellites in view */
    SatData sat[MAX_SAT];

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
        void updateBoat(GpsData info);

    protected:
        int deviceType;
        boatReal *parent;

        GpsData gpsData;

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
        nmeaINFO info;
        nmeaPARSER parser;
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
        nmeaINFO info;
        nmeaPARSER parser;
};

#ifdef __UNIX_QTVLM
class GPSdReceiverThread : public ReceiverThread
{ Q_OBJECT
    public:
        GPSdReceiverThread(boatReal * parent);
        ~GPSdReceiverThread(void);
        void run();

        bool isOk(void) { return ok; }

        bool initDevice(void);

    private:
        struct gps_data_t gps_data;
        bool ok;

};
#endif

inline void nmea2gps(nmeaINFO * info,GpsData * data) {
    data->latitude=nmea_ndeg2degree(info->lat);
    data->longitude=nmea_ndeg2degree(info->lon);
    data->elevation=info->elv;
    data->declination=info->declination;
    data->direction=info->direction;
    data->speed=info->speed/1.852;

    data->fix=info->fix;
    data->sig=info->sig;

    data->HDOP=info->HDOP;
    data->PDOP=info->PDOP;
    data->VDOP=info->VDOP;

    data->time=QDateTime(QDate(info->utc.year,info->utc.mon,info->utc.day),QTime(info->utc.hour,info->utc.min,info->utc.sec));

    data->inuse=info->satinfo.inuse;
    data->inview=info->satinfo.inview;
    for(int i=0;i<MAX_SAT;++i) {
        data->sat[i].azimuth=info->satinfo.sat[i].azimuth;
        data->sat[i].elevation=info->satinfo.sat[i].elv;
        data->sat[i].id=info->satinfo.sat[i].id;
        data->sat[i].in_use=info->satinfo.sat[i].in_use;
        data->sat[i].sigQ=info->satinfo.sat[i].sig;
    }
}

inline void clearGpsData(GpsData * data) {
    memset(data,0,sizeof(GpsData));
    data->sig=0;
    data->fix=1;
}
class InternalReceiverThread : public ReceiverThread
{ Q_OBJECT
    public:
        InternalReceiverThread(boatReal * parent);
        ~InternalReceiverThread(void);
        void run();

        bool initDevice(void);

    private:
        QGeoPositionInfoSource * geoPositionInfoSource;
        QGeoSatelliteInfoSource * geoSatelliteInfoSource;
};
#endif // GPSRECEIVER_H
