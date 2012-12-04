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
    static QString formatDegres(double x);           // 123.4 -> 123°24.00'
    static QString formatPosition(double x, double y);    // 123°24.00'W 45°67.89'N
    static QString formatLongitude(double x);
    static QString formatLatitude(double y);

    static QString formatDateLong(time_t t);

    static QString formatDateTimeLong(time_t t);
    static QString formatDateTimeShort(time_t t);
    static QString formatDateTime_date(time_t t);
    static QString formatDateTime_hour(time_t t);

    static QString formatSpeed(double meterspersecond);
    static QString formatDistance(double mille);
    static QString formatTemperature(double tempKelvin);
    static QString formatTemperature_short(double tempKelvin);
    static QString formatPercentValue(double v);

    static int    kmhToBeaufort(double v);
    static double  kmhToBeaufort_F(double v);
    static double  BeaufortToKmh_F(double bf);
    static QPointF calculateSumVect(double angle1,double length1,double angle2,double length2);

    static void paramProxy(QNetworkAccessManager *inetManager,QString host);
    static bool getWPClipboard(QString *,double * lat,double * lon, double * wph, int * tStamp);
    static void setWPClipboard(double lat,double lon, double wph);
    static bool convertPOI(const QString & str,QString * name,double * lat,double * lon,double * wph,int * tstamp,
                           int type);
    static void getCoordFromDistanceAngle(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceLoxo(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon);
    static void getCoordFromDistanceAngle2(double latitude, double longitude,
                                           double distance,double heading, double * res_lat,double * res_lon);
    static QString pos2String(int type,double value);
    static QString getHost();
    static void computePos(Projection * proj, double lat, double lon, int * x, int * y);
    static void computePosDouble(Projection * proj, double lat, double lon, double * x, double * y);
    static void addAgent(QNetworkRequest & request);
    static bool lineIsCrossingRect(const QLineF line, const QRectF rect);
    static double cLFA(const double lon, const double xW);
    static double myDiffAngle(double a1,double a2);
    static double A360(double hdg);
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
inline double Util::cLFA(double lon, double xW)
//convertLonForAntiMeridian
{
    if(xW>=0 && lon>=0) return lon;
    if(xW<=0 && lon<=0) return lon;
    if(qAbs(qRound(qAbs(lon-xW))-qRound(myDiffAngle(A360(lon),A360(xW))))<=2) return lon;
    if(xW>=0)
    {
        return xW+myDiffAngle(xW,lon+360.0);
    }
    else
    {
        if(xW<-180)
            return lon-360;
        else
            return xW-myDiffAngle(A360(xW),lon);
    }
}
inline double Util::A360(double hdg)
{
    while (hdg>=360.0) hdg=hdg-360.0;
    while (hdg<0.0) hdg=hdg+360.0;
    return hdg;
}
inline double Util::myDiffAngle(double a1,double a2)
{
    return qAbs(A360(qAbs(a1)+ 180.0 -qAbs(a2)) -180.0);
}

inline int Util::kmhToBeaufort(double v) {
    return (int)(kmhToBeaufort_F(v)+0.5);
}
//-----------------------------------------------------------------------------
inline double Util::kmhToBeaufort_F(double v) {
    double bf = pow( v*v/9.0 , 0.33333);
    if (bf > 12.0)
        bf = 12.0;
    else if (bf < 0.0)
        bf = 0.0;
    return bf;
}
//-----------------------------------------------------------------------------
inline double Util::BeaufortToKmh_F(double bf) {
    double v = sqrt(bf*bf*bf*9.0);
    return v;
}
inline QPointF Util::calculateSumVect(double angle1,double length1,double angle2,double length2)
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

#endif
