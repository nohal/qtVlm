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

***********************************************************************/

#include <QDebug>

#include "Orthodromie.h"
#include "orthoSegment.h"
#include "Projection.h"

orthoSegment::orthoSegment(Projection * proj, QGraphicsScene * myScene,int z_level,bool roundedEnd) : QGraphicsWidget()
{
    lon1=lon2=lat1=lat2=0;
    this->proj=proj;
    this->roundedEnd=roundedEnd;
    myScene->addItem(this);
    this->setZValue(z_level);
    this->alsoDrawLoxo=false;
    isOrtho=true;
    myLine = new vlmLine(proj,myScene,z_level);
    myLine->setRoundedEnd(roundedEnd);
    myLine->setParent(this);
    hide();
    myLine->setHidden(true);
}
orthoSegment::~orthoSegment()
{
    delete myLine;
}
void orthoSegment::initSegment(const double &lon1, const double &lat1, const double &lon2, const double &lat2)
{
    this->lon1=lon1;
    this->lat1=lat1;
    this->lon2=lon2;
    this->lat2=lat2;
    if(lon1==lon2 && lat1==lat2)
    {
        hide();
        myLine->setHidden(true);
    }
    else
    {
        calculatePoly();
        show();
        myLine->setHidden(false);
    }
}

void orthoSegment::moveSegment(const double &lon2, const double &lat2)
{
    this->lon2=lon2;
    this->lat2=lat2;
    calculatePoly();
}


void orthoSegment::hideSegment(void)
{
    hide();
    myLine->setHidden(true);
}


void orthoSegment::draw_orthoSegment(const double &longitude1, const double &latitude1, const double longitude2, const double latitude2, const int &recurs)
{
    if (recurs > 10)
    {
        myLine->addVlmPoint(vlmPoint(longitude1,latitude1));
        myLine->addVlmPoint(vlmPoint(longitude2,latitude2));
        return;
    }
    double i1,i2,j1,j2;
    proj->map2screenDouble(longitude1,latitude1,&i1,&j1);
    proj->map2screenDouble(longitude2,latitude2,&i2,&j2);
    if (qAbs(i1-i2) > 10)
    {
        double xm, ym;

        Orthodromie ortho(longitude1,latitude1,longitude2,latitude2);
        ortho.getMidPoint(&xm, &ym);

        xm *= 180.0/M_PI;
        ym *= 180.0/M_PI;
        while (ym > 90)
            ym -= 180;
        while (ym < -90)
            ym += 180;
        draw_orthoSegment(longitude1,latitude1, xm,ym, recurs+1);
        draw_orthoSegment(xm,ym, longitude2,latitude2, recurs+1);
    }
    else
    {
        myLine->addVlmPoint(vlmPoint(longitude1,latitude1));
        myLine->addVlmPoint(vlmPoint(longitude2,latitude2));
    }
}
void orthoSegment::calculatePoly()
{
    myLine->deleteAll();
    if(!isOrtho)
    {
        myLine->addVlmPoint(vlmPoint(lon1,lat1));
        myLine->addVlmPoint(vlmPoint(lon2,lat2));
    }
    else
    {
        draw_orthoSegment(lon1,lat1,lon2,lat2,0);
    }
    if(alsoDrawLoxo && isOrtho)
    {
        myLine->addVlmPoint(vlmPoint(lon1,lat1));
        myLine->addVlmPoint(vlmPoint(lon2,lat2));
    }
    myLine->slot_showMe();
}

