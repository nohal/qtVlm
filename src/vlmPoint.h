#ifndef VLMPOINT_H
#define VLMPOINT_H
#include "class_list.h"
#include <QDateTime>
#include<QPointF>
class vlmPoint
{
    public:
        vlmPoint(float lon,float lat);
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
        double initialCap;
        double startLon;
        double startLat;
    private:
};


#endif // VLMPOINT_H
