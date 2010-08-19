#include "vlmPoint.h"
#include "Util.h"
#include "Orthodromie.h"
#include "boatAccount.h"
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
    this->initialCap=0;
    this->startLon=0;
    this->startLat=0;
}
