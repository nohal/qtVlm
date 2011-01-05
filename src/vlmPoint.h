#ifndef VLMPOINT_H
#define VLMPOINT_H
#include "class_list.h"
#include <QDateTime>
#include <QPointF>
class vlmPoint
{
    public:
        vlmPoint(float lon,float lat);
        vlmPoint(){vlmPoint(0,0);}
        QPointF getPointF() {return QPointF(lon,lat);}
        float lon;
        float lat;
        const vlmPoint *origin;
        bool  isStart;
        float startCap;
        bool  isDead;
        float wind_angle;
        float wind_speed;
        time_t eta;
        double distIso;
        double capOrigin;
	double distOrigin;
        double eyeLon;
        double eyeLat;
        bool reversedEye;
        double distStart;
        double capStart;
        double distArrival;
        double capArrival;
        int originNb;
        double lonProj;
        double latProj;
        bool needRoute;
        float vmgSpeed;
        bool isBroken;
        float x;
        float y;
        float capVmg;
        bool notSimplificable;
        bool operator==(const vlmPoint &other) const
            {return (qRound(this->lon*1000)==qRound(other.lon*1000) &&
                     qRound(this->lat*1000)==qRound(other.lat*1000));}
        float maxDistIso;
        ROUTAGE * routage;

};


#endif // VLMPOINT_H
