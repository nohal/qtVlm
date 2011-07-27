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

#include "class_list.h"
#include "dataDef.h"

#ifdef __QTVLM_WITH_TEST
#define NB_URL 4
#else
#define NB_URL 3
#endif

extern QString url_name[NB_URL];
extern QString url_str[NB_URL];

class Util
{
    public:

    //-------------------------------------------------
    static void setFontDialog(QObject * o);
    static void setFontDialog(QWidget * o);
    static QString formatDegres(float x);           // 123.4 -> 123°24.00'
    static QString formatPosition(float x, float y);    // 123°24.00'W 45°67.89'N
    static QString formatLongitude(float x);
    static QString formatLatitude(float y);

    static QString formatDateLong(time_t t);

    static QString formatDateTimeLong(time_t t);
    static QString formatDateTimeShort(time_t t);
    static QString formatDateTime_date(time_t t);
    static QString formatDateTime_hour(time_t t);

    static QString formatSpeed(float meterspersecond);
    static QString formatDistance(float mille);
    static QString formatTemperature(float tempKelvin);
    static QString formatTemperature_short(float tempKelvin);
    static QString formatPercentValue(float v);

    static int    kmhToBeaufort(float v);
    static float  kmhToBeaufort_F(float v);
    static float  BeaufortToKmh_F(float bf);

    static void paramProxy(QNetworkAccessManager *inetManager,QString host);
    static bool getWPClipboard(QString *,float * lat,float * lon, float * wph, int * tStamp);
    static void setWPClipboard(float lat,float lon, float wph);
    static bool convertPOI(const QString & str,QString * name,float * lat,float * lon,float * wph,int * tstamp,
                           int type);
    static void getCoordFromDistanceAngle(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceLoxo(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceAngle2(double latitude, double longitude,
                                           double distance,double heading, double * res_lat,double * res_lon);
    static QString pos2String(int type,float value);
    static QString getHost();
    static void computePos(Projection * proj, float lat, float lon, int * x, int * y);
    static void computePosFloat(Projection * proj, double lat, double lon, double * x, double * y);
    static void addAgent(QNetworkRequest & request);
    static bool lineIsCrossingRect(const QLineF line, const QRectF rect);

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
