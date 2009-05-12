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
        
        double   x, y;       // position
        time_t   date;
        double   vx, vy;     // wind
        
	private:
        GribReader *gribReader;
        void initGribPointInfo();
        
};

#endif
