#include "vlmPoint.h"
#include "Util.h"
#include "Orthodromie.h"

#include "Polar.h"
#include <QLineF>
vlmPoint::vlmPoint(float lon,float lat)
{
    this->lon=lon;
    this->lat=lat;
    this->origin=NULL;
    this->isStart=false;
    this->startCap=-1;
    this->isDead=false;
    this->eta=0;
    this->distIso=-1;
    this->capOrigin=0;
    this->eyeLon=0;
    this->eyeLat=0;
}
