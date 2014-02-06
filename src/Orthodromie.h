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
//
// Inspiration :
// http://www.acscdg.com/source_html/src_geo_js.html
// http://williams.best.vwh.net/avform.htm
//

#ifndef ORTHODROMIE_H
#define ORTHODROMIE_H

#include <cmath>
#include <cassert>
#include <QPainter>

#include "class_list.h"

class Orthodromie
{
    public:
        Orthodromie(double x0,double y0, double x1,double y1);
        Orthodromie(QPointF p1, QPointF p2);
        
        void  setPoints(double x0,double y0, double x1,double y1);
        void  setStartPoint (double x,double y);
        void  setEndPoint   (double x,double y);

        double getDistance()  const {return distanceNM;}
        double getAzimutDeg() const {return azimutDeg;}
        double getAzimutRad() const {return azimut;}
        double getLoxoCap() const;
        double getLoxoDistance() const;
        void  getMidPoint(double *x, double *y) const;
        
        //// Reduce an angle to (-PI/2, PI/2), for latitudes.
        double  reduceLat(double lat) const;
        //// Reduce and angle to (-PI, PI), for longitudes.
        double  reduceLng(double lng) const;
        //// Reduce an angle to (0, 2*PI), for direction and azimuth.
        double  reduceAzimut(double azimuth) const;

        //static void draw_OrthodromieSegment(Projection * proj, QPainter * pnt, double x0,double y0, double x1,double y1, int recurs=0);

    private:
        void initOrthodromie();

        double lat0,lat1,lon0,lon1;   // radians
        double distanceNM;
        
        double azimut, azimutDeg;
        
        double sinStartLat, cosStartLat;
        double sinEndLat,   cosEndLat;
        double sinAzimut,   cosAzimut;
        double sinDeltaLng, cosDeltaLng;


        
};
Q_DECLARE_TYPEINFO(Orthodromie,Q_MOVABLE_TYPE);



#endif
