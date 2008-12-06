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

#include <stdio.h>

#include "Orthodromie.h"

    
//------------------------------------------------------------------------------
Orthodromie::Orthodromie(float x0,float y0, float x1,float y1)
{
    setPoints(x0,y0, x1,y1);
}
//------------------------------------------------------------------------------
void Orthodromie::setPoints(float x0,float y0, float x1,float y1)
{
    lon0 = x0 *M_PI/180.0;
    lat0 = y0 *M_PI/180.0;
    lon1 = x1 *M_PI/180.0;
    lat1 = y1 *M_PI/180.0;
    initOrthodromie();
}
//------------------------------------------------------------------------------
void Orthodromie::initOrthodromie()
{
    sinStartLat = sin(lat0);
    cosStartLat = cos(lat0);
    sinEndLat = sin(lat1);
    cosEndLat = cos(lat1);
    float deltaLng = reduceLng(lon1 - lon0);
    cosDeltaLng = cos(deltaLng);
    sinDeltaLng = sin(deltaLng);

    float cosang = sinStartLat*sinEndLat + cosStartLat*cosEndLat*cosDeltaLng;
    cosang = (cosang < -1.0) ? -1.0 : (cosang > 1.0) ? 1.0 : cosang;
    distanceNM = 6378.0/1.852 * acos(cosang);
    
    azimut = reduceAzimut(
                 atan2(sin(lon1-lon0)*cos(lat1),
                       cos(lat0)*sin(lat1)-sin(lat0)*cos(lat1)*cos(lon1-lon0)));
    sinAzimut = sin(azimut);
    cosAzimut = cos(azimut);
    azimutDeg = 180.0/M_PI*azimut;
}
    
//------------------------------------------------------------------------------
void Orthodromie::setStartPoint (float x,float y)
{
    lat0 = y *M_PI/180.0;
    lon0 = x *M_PI/180.0;
    initOrthodromie();
}
//------------------------------------------------------------------------------
void Orthodromie::setEndPoint   (float x,float y)
{
    lat1 = y *M_PI/180.0;
    lon1 = x *M_PI/180.0;
    initOrthodromie();
}
    
//------------------------------------------------------------------------------
void Orthodromie::getMidPoint(float *x, float *y)
{
    float Bx = cosEndLat*cosDeltaLng;
    float By = cosEndLat*sinDeltaLng;
    float Bz = cosStartLat+Bx;
    *y = atan2( sin(lat0)+sin(lat1),  sqrt(Bz*Bz+By*By));
    *x = lon0 + atan2(By, cos(lat0)+Bx);

//    printf("%f %f\n", *x, *y);
}

//------------------------------------------------------------------------------
//// Reduce an angle to (-PI/2, PI/2), for latitudes.
float Orthodromie::reduceLat(float lat)  // old name: fng(x)
{
  return lat - M_PI * floor((lat + M_PI/2.0) / M_PI);
}
//// Reduce and angle to (-PI, PI), for longitudes.
float Orthodromie::reduceLng(float lng)  // old name: fnl(x)
{
  return lng - 2.0*M_PI * floor((lng + M_PI ) / (2.0*M_PI));
}
//// Reduce an angle to (0, 2*PI), for direction and azimuth.
float Orthodromie::reduceAzimut(float azimut)
{
  return azimut - 2.0*M_PI * floor(azimut / (2.0*M_PI));
}

