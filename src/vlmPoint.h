#ifndef VLMPOINT_H
#define VLMPOINT_H
#include "class_list.h"
#include <QDateTime>
#include <QPointF>
class vlmPoint
{
    public:
        vlmPoint(double lon,double lat);
        vlmPoint(){vlmPoint(0,0);}
        QPointF getPointF() {return QPointF(lon,lat);}
        double  lon;
        double  lat;
        vlmPoint *origin;
        QList<vlmPoint> myChildren;
        bool   isStart;
        double  startCap;
        bool   isDead;
        double  wind_angle;
        double  wind_speed;
        time_t eta;
        double distIso;
        double capOrigin;
        double distOrigin;
        int    internal_1;
        double internal_2;
        int    debugInt;
        double distStart;
        double capStart;
        double distArrival;
        double capArrival;
        int    originNb;
        double lonProj;
        double latProj;
        bool   isBroken;
        double  x;
        double  y;
        double  capVmg;
        bool   notSimplificable;
        bool   operator==(const vlmPoint &other) const
               {return qRound(this->lon*10e6)==qRound(other.lon*10e6) &&
                       qRound(this->lat*10e6)==qRound(other.lat*10e6);}
        bool   operator!=(const vlmPoint &other) const
               {return qRound(this->lon*10e6)!=qRound(other.lon*10e6) ||
                       qRound(this->lat*10e6)!=qRound(other.lat*10e6);}
        ROUTAGE * routage;
        bool    isPOI;
        double  xP1,yP1,xM1,yM1;
        double  speed;
        time_t timeStamp;
        double current_speed;
        double current_angle;
        bool foundByNewtonRaphson;
        double convertionLon;
        double convertionLat;
        int isoIndex;
        int debug;
};
Q_DECLARE_TYPEINFO(vlmPoint,Q_MOVABLE_TYPE);


#endif // VLMPOINT_H
