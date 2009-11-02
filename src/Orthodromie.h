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


class Orthodromie
{
    public:
        Orthodromie(float x0,float y0, float x1,float y1);
        
        void  setPoints(float x0,float y0, float x1,float y1);
        void  setStartPoint (float x,float y);
        void  setEndPoint   (float x,float y);

        float getDistance()     {return distanceNM;}
        float getAzimutDeg()    {return azimutDeg;}
        float getAzimutRad()    {return azimut;}

        void  getMidPoint(float *x, float *y);
        
        //// Reduce an angle to (-PI/2, PI/2), for latitudes.
        float  reduceLat(float lat);
        //// Reduce and angle to (-PI, PI), for longitudes.
        float  reduceLng(float lng);
        //// Reduce an angle to (0, 2*PI), for direction and azimuth.
        float  reduceAzimut(float azimuth);

    private:
        void initOrthodromie();

        float lat0,lat1,lon0,lon1;   // radians
        float distanceNM;
        
        float azimut, azimutDeg;
        
        float sinStartLat, cosStartLat;
        float sinEndLat,   cosEndLat;
        float sinAzimut,   cosAzimut;
        float sinDeltaLng, cosDeltaLng;


        
};



#endif
