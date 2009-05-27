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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <cassert>

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QLocale>
#include <QNetworkAccessManager>

#include "Projection.h"

#define PI     M_PI
#define PI_2   M_PI_2
#define PI_4   M_PI_4
#define TWO_PI M_PI * 2

#define degToRad(angle) (((angle)/180.0) * PI)
#define radToDeg(angle) (((angle)*180.0) / PI)

#define msToKts(speed) (1.9438445*(speed))
#define ktsToMs(speed) (0.51444444*(speed))

#define TYPE_LON 1
#define TYPE_LAT 2

#define adjustFloat(VAR) ({ \
    VAR = ((float)((int)(VAR*1000)))/1000; \
    })

#define compFloat(VAR1,VAR2) ({ \
    bool _res;                 \
    float _v1=VAR1;            \
    adjustFloat(_v1);           \
    float _v2=VAR2;            \
    adjustFloat(_v2);           \
    _res = (_v1==_v2);         \
    _res;                      \
    })

#define NB_URL 4
extern QString url_name[NB_URL];
extern QString url_str[NB_URL];

class Util
{
    public:

    static void     setSetting(const QString &key, const QVariant &value);
    static QVariant getSetting(const QString &key, const QVariant &defaultValue);

    //-------------------------------------------------
    static QString formatDegres(float x);           // 123.4 -> 123°24.00'
    static QString formatPosition(float x, float y);    // 123°24.00'W 45°67.89'N
    static QString formatLongitude(float x);
    static QString formatLatitude(float y);

    static QString formatDateLong(time_t t);

    static QString formatDateTimeLong(time_t t);
    static QString formatDateTimeShort(time_t t);
    static QString formatDateTime_date(time_t t);
    static QString formatDateTime_hour(time_t t);
    static QString formatTime(time_t t);
    static QDateTime applyTimeZone(time_t t, QString *suffix);

    static QString formatSpeed(float meterspersecond);
    static QString formatDistance(float mille);
    static QString formatTemperature(float tempKelvin);
    static QString formatTemperature_short(float tempKelvin);
    static QString formatPercentValue(float v);

    static int    kmhToBeaufort(float v);
    static float  kmhToBeaufort_F(float v);
    static float  BeaufortToKmh_F(float bf);

    static void paramProxy(QNetworkAccessManager *inetManager,QString host);
    static bool getWPClipboard(float * lat,float * lon, float * wph, int * tStamp);
    static void setWPClipboard(float lat,float lon, float wph);
    static void getCoordFromDistanceAngle(float latitude, float longitude,
             float distance,float heading, float * res_lat,float * res_lon);
    static void getCoordFromDistanceAngle2(float latitude, float longitude,
             float distance,float heading, float * res_lat,float * res_lon);
    static QString pos2String(int type,float value);
    static QString getHost();
    static void computePos(Projection * proj, float lat, float lon, int * x, int * y);
    static void addAgent(QNetworkRequest & request);

    //-------------------------------------------------
    template <typename T>
        static bool isInRange(T v, T min, T max)
                    { return (v>=min && v<=max); }

    //-------------------------------------------------
    template <typename T>
        static T inRange(T v, T min, T max)
                    {
                        if (v<=min) return min;
                        else if (v>=max) return max;
                        else return v;
                    }

    //-------------------------------------------------
    template <typename T>
        static void cleanListPointers( std::list<T *> & ls)
        {
            typename std::list<T *>::iterator it;
            for (it=ls.begin(); it!=ls.end(); it++) {
                delete *it;
                *it = NULL;
            }
            ls.clear();
        }

};

//======================================================================
inline int Util::kmhToBeaufort(float v) {
    return (int)(kmhToBeaufort_F(v)+0.5);
}
//-----------------------------------------------------------------------------
inline float Util::kmhToBeaufort_F(float v) {
    float bf = pow( v*v/9.0 , 0.33333);
    if (bf > 12.0)
        bf = 12.0;
    else if (bf < 0.0)
        bf = 0.0;
    return bf;
}
//-----------------------------------------------------------------------------
inline float Util::BeaufortToKmh_F(float bf) {
    float v = sqrt(bf*bf*bf*9.0);
    return v;
}


#endif
