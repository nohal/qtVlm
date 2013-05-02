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
#ifdef QT_V5
#include <QtWidgets/QApplication>
#else
#include <QApplication>
#endif
#include <QObject>
#include <QString>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QLocale>
#include <QNetworkAccessManager>

#include "class_list.h"
#include "dataDef.h"
#include <QLineF>

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
    static QString formatDegres(const double &x);           // 123.4 -> 123°24.00'
    static QString formatPosition(const double &x, const double &y);    // 123°24.00'W 45°67.89'N
    static QString formatLongitude(double x);
    static QString formatLatitude(const double &y);

    static QString formatDateLong(const time_t &t);

    static QString formatDateTimeLong(const time_t &t);
    static QString formatDateTimeShort(const time_t &t);
    static QString formatDateTime_date(const time_t &t);
    static QString formatDateTime_hour(const time_t &t);

    static QString formatSpeed(const double &meterspersecond);
    static QString formatDistance(const double &mille);
    static QString formatTemperature(const double &tempKelvin);
    static QString formatTemperature_short(const double &tempKelvin);
    static QString formatPercentValue(double v);

    static int    kmhToBeaufort(const double &v);
    static double  kmhToBeaufort_F(const double &v);
    static double  BeaufortToKmh_F(const double &bf);
    static QPointF calculateSumVect(const double &angle1, const double &length1, const double &angle2, const double &length2);

    static void paramProxy(QNetworkAccessManager *inetManager,QString host);
    static bool getWPClipboard(QString *,double * lat,double * lon, double * wph, int * tStamp);
    static void setWPClipboard(double lat,double lon, double wph);
    static bool convertPOI(const QString & str,QString * name,double * lat,double * lon,double * wph,int * tstamp,
                           int type);
    static void getCoordFromDistanceAngle(double latitude, double longitude,
             double distance, double heading, double * res_lat, double * res_lon);
    static void getCoordFromDistanceLoxo(const double &latitude, const double &longitude,
             const double &distance,const double &heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceAngle2(const double &latitude, const double &longitude,
                                           const double &distance, const double &heading, double * res_lat,double * res_lon);
    static QString pos2String(const int &type,const double &value);
    static QString getHost();
    static void computePos(Projection * proj, const double &lat, const double &lon, int * x, int * y);
    static void computePosDouble(Projection * proj, const double &lat, const double &lon, double * x, double * y);
    static void addAgent(QNetworkRequest & request);
    static bool lineIsCrossingRect(const QLineF &line, const QRectF &rect);
    static double myDiffAngle(const double &a1, const double &a2);
    static double A360(const double &hdg);
    static double A180(double angle);
    static double distance_to_line_dichotomy_xing(const double &lat, const double &lon,
                                                 const double &lat_a, const double &lon_a,
                                                 const double &lat_b, const double &lon_b,
                                                 double *x_latitude, double *x_longitude);

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
            for (it=ls.begin(); it!=ls.end(); ++it) {
                delete *it;
                *it = NULL;
            }
            ls.clear();
        }

};

//======================================================================
inline double Util::A360(const double &hdg)
{
    double newhdg=hdg;
    while (newhdg>=360.0) newhdg-=360.0;
    while (newhdg<0.0) newhdg+=360.0;
    return newhdg;
}
inline double Util::myDiffAngle(const double &a1,const double &a2)
{
    return qAbs(A360(qAbs(a1)+ 180.0 -qAbs(a2)) -180.0);
}

inline int Util::kmhToBeaufort(const double &v) {
    return (int)(kmhToBeaufort_F(v)+0.5);
}
//-----------------------------------------------------------------------------
inline double Util::kmhToBeaufort_F(const double &v) {
    double bf = pow( v*v/9.0 , 0.33333);
    if (bf > 12.0)
        bf = 12.0;
    else if (bf < 0.0)
        bf = 0.0;
    return bf;
}
//-----------------------------------------------------------------------------
inline double Util::BeaufortToKmh_F(const double &bf) {
    double v = sqrt(bf*bf*bf*9.0);
    return v;
}
inline QPointF Util::calculateSumVect(const double &angle1,const double &length1,const double &angle2,const double &length2)
{
    QLineF line1(0,0,1,1);
    line1.setLength(length1);
    line1.setAngle(A360(angle1));
    QLineF line2(line1.p2().x(),line1.p2().y(),1,1);
    line2.setLength(length2);
    line2.setAngle(A360(angle2));
    QLineF temp(0,0,line2.p2().x(),line2.p2().y());
    QPointF pointF(temp.length(),A360(temp.angle()));
    return pointF;
}
inline void Util::getCoordFromDistanceAngle(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon)
{
    if(qAbs(latitude)>=89.9)
    {
        *res_lat=latitude;
        *res_lon=longitude;
        return;
    }
    double d, new_lat, t_lat, new_lon;
    latitude = degToRad(latitude);
    longitude = fmod(degToRad(longitude), TWO_PI);
    heading=degToRad(heading);
    d = degToRad(distance/60.0);
    new_lat = latitude + d*cos(heading);
    t_lat = (latitude + new_lat) / 2.0;
    new_lon =  longitude + (d*sin(heading))/cos(t_lat);
    if (new_lon > PI)
    {
        new_lon -= TWO_PI;
    }
    else if (new_lon < -PI)
    {
        new_lon += TWO_PI;
    }
    *res_lat = radToDeg(new_lat);
    *res_lon = radToDeg(new_lon);
}

#endif
