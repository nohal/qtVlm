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
#include "AngleUtil.h"

#include <QLineF>

class Util
{
    public:

    //-------------------------------------------------
    static void setFontDialog(QObject * o);
    static void setFontDialog(QWidget * o);
    static void setSpecificFont(QMap<QWidget *, QFont> widgets);
    static QString formatDegres(const double &x);           // 123.4 -> 123°24.00'
    static QString formatPosition(const double &x, const double &y);    // 123°24.00'W 45°67.89'N
    static QString formatLongitude(const double &x);
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

    static QString formatSimpleIntUnit(int val,QString unit);
    static QString formatSimpleDoubleUnit(double val,QString unit);
    static QString formatData(int type,double val1,double val2=0);
    static QString formatSimpleData(int type,double val);

    static int    kmhToBeaufort(const double &v);
    static double  kmhToBeaufort_F(const double &v);
    static double  BeaufortToKmh_F(const double &bf);

    static int    msToBeaufort   (float v);
    static float  msToBeaufort_F (float v);
    static float  BeaufortToMs_F (float bf);

    static QPointF calculateSumVect(const double &angle1, const double &length1, const double &angle2, const double &length2);

    static void paramProxy(QNetworkAccessManager *inetManager,QString host);
    static bool getWPClipboard(QString *,double * lat,double * lon, double * wph, int * tStamp);
    static void setWPClipboard(double lat,double lon, double wph);
    static bool convertPOI(const QString & str,QString * name,double * lat,double * lon,double * wph,int * tstamp,
                           int type);
    static void getCoordFromDistanceAngle(const double &latitude, const double &longitude,
             const double &distance, const double &heading, double * res_lat, double * res_lon);
    static void getCoordFromDistanceLoxo(const double &latitude, const double &longitude,
             const double &distance,const double &heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceAngle2(const double &latitude, const double &longitude,
                                           const double &distance, const double &heading, double * res_lat,double * res_lon);
    static QString pos2String(const int &type,const double &value);

    static void computePos(Projection * proj, const QPointF &position, QPoint * screenCoord);
    static void computePos(Projection * proj, const double &lat, const double &lon, int * x, int * y);
    static void computePosDouble(Projection * proj, const double &lat, const double &lon, double * x, double * y);
    static void computePosDouble(Projection * proj, const QPointF &position, QPointF * screenCoord);
    static void addAgent(QNetworkRequest & request, bool overrideForce=false);
    static bool lineIsCrossingRect(const QLineF &line, const QRectF &rect);

    static QString generateKey(int size);

    static double distance_to_line_dichotomy_xing(const double &lat, const double &lon,
                                                 const double &lat_a, const double &lon_a,
                                                 const double &lat_b, const double &lon_b,
                                                 double *x_latitude, double *x_longitude);
    static double distToSegment(const QPointF point,const QLineF line);

    static QString formatElapsedTime(int elapsed);
    static QString currentPath();
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

        static double getOrthoDistance(const double &latitude1, const double &longitude1, const double &latitude2, const double &longitude2);
        static void getCoordFromDistanceAngle3(const double &latitude, const double &longitude, const double &distance, const double &heading, double *res_lat, double *res_lon);
};

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
    line1.setAngle(AngleUtil::A360(angle1));
    QLineF line2(line1.p2().x(),line1.p2().y(),1,1);
    line2.setLength(length2);
    line2.setAngle(AngleUtil::A360(angle2));
    QLineF temp(0,0,line2.p2().x(),line2.p2().y());
    QPointF pointF(temp.length(),AngleUtil::A360(temp.angle()));
    return pointF;
}
//-----------------------------------------------------------------------------
inline int Util::msToBeaufort (float v) {
    return (int)(msToBeaufort_F(v)+0.5);
}
inline float Util::msToBeaufort_F (float v) {
    float bf = pow (v*v*1.44 , 0.33333);
    if (bf > 12.0)
        bf = 12.0;
    return bf;
}
inline float Util::BeaufortToMs_F (float bf) {
    return sqrt (bf*bf*bf/1.44);
}
//-----------------------------------------------------------------------------
inline void Util::getCoordFromDistanceAngle3(const double &latitude, const double &longitude,
             const double &distance,const double &heading, double * res_lat,double * res_lon)
{
    double lat1=degToRad(AngleUtil::A360(latitude));
    double lon1=degToRad(AngleUtil::A360(longitude));
    double hdg=degToRad(AngleUtil::A360(heading));
    double R=6371.0/1.852; // nm
    double lat2 = asin( sin(lat1)*cos(distance/R) +
                  cos(lat1)*sin(distance/R)*cos(hdg) );
    double lon2 = lon1 + atan2(sin(hdg)*sin(distance/R)*cos(lat1),
                         cos(distance/R)-sin(lat1)*sin(lat2));
    if (lon2 > PI)
    {
        lon2 -= TWO_PI;
    }
    else if (lon2 < -PI)
    {
        lon2 += TWO_PI;
    }
    *res_lon=radToDeg(lon2);
    *res_lat=radToDeg(lat2);
    return;
}
inline void Util::getCoordFromDistanceAngle(const double &latitude, const double &longitude,
             const double &distance,const double &heading, double * res_lat,double * res_lon)
{
    if(qAbs(latitude)>=89.9)
    {
        *res_lat=latitude;
        *res_lon=longitude;
        return;
    }
#if 1
    getCoordFromDistanceAngle3(latitude,longitude,distance,heading,res_lat,res_lon);
    return;
#else
    double d, new_lat, t_lat, new_lon;
    double lat = degToRad(latitude);
    double lon = fmod(degToRad(longitude), TWO_PI);
    double hdg=degToRad(heading);
    d = degToRad(distance/60.0);
    new_lat = lat + d*cos(hdg);
    t_lat = (lat + new_lat) / 2.0;
    new_lon =  lon + (d*sin(hdg))/cos(t_lat);
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
#endif
}

#endif
