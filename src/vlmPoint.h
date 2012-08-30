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
        const  vlmPoint *origin;
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
        bool   needRoute;
        double  vmgSpeed;
        bool   isBroken;
        double  x;
        double  y;
        double  capVmg;
        bool   notSimplificable;
        bool   operator==(const vlmPoint &other) const
               {return (qRound(this->lon*1000)==qRound(other.lon*1000) &&
                        qRound(this->lat*1000)==qRound(other.lat*1000));}
        double  maxDistIso;
        ROUTAGE * routage;
        bool    isPOI;
        double  xP1,yP1,xM1,yM1;
        double  speed;
        time_t timeStamp;
        double current_speed;
        double current_angle;
};


#endif // VLMPOINT_H
