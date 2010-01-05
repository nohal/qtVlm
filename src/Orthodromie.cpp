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
Orthodromie::Orthodromie(double x0,double y0, double x1,double y1)
{
    setPoints(x0,y0, x1,y1);
}
//------------------------------------------------------------------------------
void Orthodromie::setPoints(double x0,double y0, double x1,double y1)
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
    double deltaLng = reduceLng(lon1 - lon0);
    cosDeltaLng = cos(deltaLng);
    sinDeltaLng = sin(deltaLng);

    double cosang = sinStartLat*sinEndLat + cosStartLat*cosEndLat*cosDeltaLng;
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
void Orthodromie::setStartPoint (double x,double y)
{
    lat0 = y *M_PI/180.0;
    lon0 = x *M_PI/180.0;
    initOrthodromie();
}
//------------------------------------------------------------------------------
void Orthodromie::setEndPoint   (double x,double y)
{
    lat1 = y *M_PI/180.0;
    lon1 = x *M_PI/180.0;
    initOrthodromie();
}
    
//------------------------------------------------------------------------------
void Orthodromie::getMidPoint(double *x, double *y)
{
    double Bx = cosEndLat*cosDeltaLng;
    double By = cosEndLat*sinDeltaLng;
    double Bz = cosStartLat+Bx;
    *y = atan2( sin(lat0)+sin(lat1),  sqrt(Bz*Bz+By*By));
    *x = lon0 + atan2(By, cos(lat0)+Bx);

//    printf("%f %f\n", *x, *y);
}

//------------------------------------------------------------------------------
//// Reduce an angle to (-PI/2, PI/2), for latitudes.
double Orthodromie::reduceLat(double lat)  // old name: fng(x)
{
  return lat - M_PI * floor((lat + M_PI/2.0) / M_PI);
}
//// Reduce and angle to (-PI, PI), for longitudes.
double Orthodromie::reduceLng(double lng)  // old name: fnl(x)
{
  return lng - 2.0*M_PI * floor((lng + M_PI ) / (2.0*M_PI));
}
//// Reduce an angle to (0, 2*PI), for direction and azimuth.
double Orthodromie::reduceAzimut(double azimut)
{
  return azimut - 2.0*M_PI * floor(azimut / (2.0*M_PI));
}

//-------------------------------------------------------
// TracÃ© rÃ©cursif
void Orthodromie::draw_OrthodromieSegment(Projection * proj, QPainter * pnt,
                            double x0,double y0, double x1,double y1,
                            int recurs
                            )
{
    if (recurs > 10) // this is bugging under win :100)
        return;
    Orthodromie *ortho;
    int i0,j0, i1,j1, im,jm;
    double eps = 0.5;
    if (y0 > 90-eps) y0 = 90-eps;
    if (y0 <-90+eps) y0 =-90+eps;
    if (y1 > 90-eps) y1 = 90-eps;
    if (y1 <-90+eps) y1 =-90+eps;

    if (fabs(x0-x1)>180)  // il faut faire le tour du monde par derriÃ¨re
    {
        if (x0 < x1) {
            Orthodromie::draw_OrthodromieSegment(proj,pnt, x1-360,y1, x0,y0, recurs+1);
            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0+360,y0, x1,y1, recurs+1);
        }
        else {
            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0-360,y0, x1,y1, recurs+1);
            Orthodromie::draw_OrthodromieSegment(proj,pnt, x1+360,y1, x0,y0, recurs+1);
        }
    }
    else
    {
        proj->map2screen(x0, y0, &i0, &j0);
        proj->map2screen(x1, y1, &i1, &j1);
        if (abs(i0-i1) > 10)
        {
            double xm, ym;

            ortho = new Orthodromie(x0, y0, x1, y1);
            ortho->getMidPoint(&xm, &ym);
            delete ortho;
            ortho = NULL;

            xm *= 180.0/M_PI;
            ym *= 180.0/M_PI;
            while (ym > 90)
                ym -= 180;
            while (ym < -90)
                ym += 180;
            proj->map2screen(xm, ym, &im, &jm);
            //printf("%5d: (%5d %5d) (%5d %5d) (%5d %5d)      %f %f   %f %f\n",recurs,i0,j0, im,jm, i1,j1,x0,y0,x1,y1);
            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0,y0, xm,ym, recurs+1);
            Orthodromie::draw_OrthodromieSegment(proj,pnt, xm,ym, x1,y1, recurs+1);
        }
        else {
                pnt->drawLine(i0,j0, i1,j1);
        }
    }
}
