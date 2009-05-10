/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

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

/*************************************
Dessin des données GRIB (avec QT)
*************************************/

#ifndef GRIBPOINTINFO_H
#define GRIBPOINTINFO_H

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <set>

#include <QApplication>
#include <QPainter>

#include "GribReader.h"
#include "Projection.h"
#include "Util.h"



//===============================================================
class GribPointInfo
{
    public :
        GribPointInfo(GribReader *gribReader, double x, double y, time_t date);
        
        bool hasWind()     const {return vx!=GRIB_NOTDEF && vy!=GRIB_NOTDEF;}
        bool hasPressure() const {return pressure!=GRIB_NOTDEF;}
        bool hasTemp()     const {return temp!=GRIB_NOTDEF;}
        bool hasTempPot()  const {return tempPot!=GRIB_NOTDEF;}
        bool hasTempMin()  const {return tempMin!=GRIB_NOTDEF;}
        bool hasTempMax()  const {return tempMax!=GRIB_NOTDEF;}
        bool hasRain()     const {return rain!=GRIB_NOTDEF;}
        bool hasCloud()    const {return cloud!=GRIB_NOTDEF;}
        bool hasHumid()    const {return humid!=GRIB_NOTDEF;}
        bool hasHumidSpec()      const {return humidSpec!=GRIB_NOTDEF;}
        bool hasDewPoint()       const {return dewPoint!=GRIB_NOTDEF;}
        bool hasIsotherm0HGT()   const {return isotherm0HGT!=GRIB_NOTDEF;}
        bool hasSnowDepth()      const {return snowDepth!=GRIB_NOTDEF;}
        bool hasSnowCateg()      const {return snowCateg!=GRIB_NOTDEF;}
        bool hasFrzRainCateg()   const {return frzRainCateg!=GRIB_NOTDEF;}
        
        double   x, y;       // position
        time_t   date;
        double   vx, vy;     // wind
        double   pressure;
        double   temp;
        double   tempPot;
        double   tempMin;
        double   tempMax;
        double   rain;
        double   cloud;
        double   humid;
        double   humidSpec;
        double   dewPoint;
        double   isotherm0HGT;
        double   snowDepth;
        double   snowCateg;
        double   frzRainCateg;
        
	private:
        GribReader *gribReader;
        void initGribPointInfo();
        
};

#endif
