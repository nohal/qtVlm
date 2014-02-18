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
#include <QDebug>

#include "Orthodromie.h"
#include "Projection.h"
#include "AngleUtil.h"


//------------------------------------------------------------------------------
Orthodromie::Orthodromie(double x0,double y0, double x1,double y1)
{
    setPoints(x0,y0, x1,y1);
}

Orthodromie::Orthodromie(QPointF p1, QPointF p2)
{
    setPoints(p1.x(),p1.y(),p2.x(),p2.y());
}

//------------------------------------------------------------------------------
void Orthodromie::setPoints(double x0,double y0, double x1,double y1)
{
    lon0 = degToRad(AngleUtil::A180(x0));
    lat0 = degToRad(AngleUtil::A180(y0));
    lon1 = degToRad(AngleUtil::A180(x1));
    lat1 = degToRad(AngleUtil::A180(y1));
//    lon0 = x0 *M_PI/180.0;
//    lat0 = y0 *M_PI/180.0;
//    lon1 = x1 *M_PI/180.0;
//    lat1 = y1 *M_PI/180.0;
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
#if 1
    double R = 6371.0/1.852; // nm
    double dLat = lat1 - lat0;
    double dLon = lon1 - lon0;

    double a = sin(dLat/2.0) * sin(dLat/2.0) +
             cos(lat0) * cos(lat1) *
             sin(dLon/2.0) * sin(dLon/2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0-a));
    distanceNM = R * c;
    double y = sin(dLon) * cos(lat1);
    double x = cos(lat0)*sin(lat1) -
               sin(lat0)*cos(lat1)*cos(dLon);
    azimut = reduceAzimut(atan2(y, x));
    sinAzimut = sin(azimut);
    cosAzimut = cos(azimut);
    azimutDeg = 180.0/M_PI*azimut;
    //qWarning()<<"new method"<<azimutDeg<<distanceNM;
#else
    double cosang = sinStartLat*sinEndLat + cosStartLat*cosEndLat*cosDeltaLng;
    cosang = (cosang < -1.0) ? -1.0 : (cosang > 1.0) ? 1.0 : cosang;
    distanceNM = 6378.0/1.852 * acos(cosang);
    azimut = reduceAzimut(atan2(sin(lon1-lon0)*cos(lat1),
                          cos(lat0)*sin(lat1)-sin(lat0)*cos(lat1)*cos(lon1-lon0)));
    sinAzimut = sin(azimut);
    cosAzimut = cos(azimut);
    azimutDeg = 180.0/M_PI*azimut;
    qWarning()<<"old method"<<azimutDeg<<distanceNM;
#endif
}

//------------------------------------------------------------------------------
void Orthodromie::setStartPoint (double x,double y)
{
    lon0 = degToRad(AngleUtil::A180(x));
    lat0 = degToRad(AngleUtil::A180(y));
    initOrthodromie();
}
//------------------------------------------------------------------------------
void Orthodromie::setEndPoint   (double x,double y)
{
    lon1 = degToRad(AngleUtil::A180(x));
    lat1 = degToRad(AngleUtil::A180(y));
    initOrthodromie();
}

//------------------------------------------------------------------------------
void Orthodromie::getMidPoint(double *x, double *y) const
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
double Orthodromie::reduceLat(double lat) const // old name: fng(x)
{
  return lat - M_PI * floor((lat + M_PI/2.0) / M_PI);
}
//// Reduce and angle to (-PI, PI), for longitudes.
double Orthodromie::reduceLng(double lng) const // old name: fnl(x)
{
  return lng - 2.0*M_PI * floor((lng + M_PI ) / (2.0*M_PI));
}
//// Reduce an angle to (0, 2*PI), for direction and azimuth.
double Orthodromie::reduceAzimut(double azimut) const
{
  return azimut - 2.0*M_PI * floor(azimut / (2.0*M_PI));
}

//-------------------------------------------------------
// Trace recursif
//void Orthodromie::draw_OrthodromieSegment(Projection * proj, QPainter * pnt,
//                            double x0,double y0, double x1,double y1,
//                            int recurs
//                            )
//{
//    if (recurs > 10) // this is bugging under win :100)
//        return;
//    int i0,j0, i1,j1, im,jm;
//    double eps = 0.5;
//    if (y0 > 90-eps) y0 = 90-eps;
//    if (y0 <-90+eps) y0 =-90+eps;
//    if (y1 > 90-eps) y1 = 90-eps;
//    if (y1 <-90+eps) y1 =-90+eps;

//    if (fabs(x0-x1)>180)  // il faut faire le tour du monde par derriere
//    {
//        if (x0 < x1) {
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, x1-360,y1, x0,y0, recurs+1);
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0+360,y0, x1,y1, recurs+1);
//        }
//        else {
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0-360,y0, x1,y1, recurs+1);
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, x1+360,y1, x0,y0, recurs+1);
//        }
//    }
//    else
//    {
//        proj->map2screen(x0, y0, &i0, &j0);
//        proj->map2screen(x1, y1, &i1, &j1);
//        if (abs(i0-i1) > 10)
//        {
//            double xm, ym;

//            Orthodromie *ortho = new Orthodromie(x0, y0, x1, y1);
//            ortho->getMidPoint(&xm, &ym);
//            delete ortho;
//            ortho = NULL;

//            xm *= 180.0/M_PI;
//            ym *= 180.0/M_PI;
//            while (ym > 90)
//                ym -= 180;
//            while (ym < -90)
//                ym += 180;
//            proj->map2screen(xm, ym, &im, &jm);
//            //printf("%5d: (%5d %5d) (%5d %5d) (%5d %5d)      %f %f   %f %f\n",recurs,i0,j0, im,jm, i1,j1,x0,y0,x1,y1);
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, x0,y0, xm,ym, recurs+1);
//            Orthodromie::draw_OrthodromieSegment(proj,pnt, xm,ym, x1,y1, recurs+1);
//        }
//        else {
//                pnt->drawLine(i0,j0, i1,j1);
//        }
//    }
//}
double Orthodromie::getLoxoCap() const
{
    double L0=log(tan(M_PI_4+lat0/2));
    double L1=log(tan(M_PI_4+lat1/2));
    double loxo=atan2((lon1-lon0),(L1-L0));
    return 180.0/M_PI*loxo;
}
double Orthodromie::getLoxoDistance() const
{
#if 1
    double L0=log(tan(M_PI_4+lat0/2.0));
    double L1=log(tan(M_PI_4+lat1/2.0));
    double q=0;
    if(qRound(qAbs(L0-L1)*10e4)==0)
        q=cos(lat1);
    else
        q=(lat1-lat0)/(L1-L0);
    double dLon=lon1-lon0;
    if(qAbs(dLon)>M_PI)
        dLon=dLon>0? -(2*M_PI-dLon) : (2*M_PI+dLon);
    return sqrt((lat1-lat0)*(lat1-lat0)+q*q*dLon*dLon)*6378.0/1.852;
#else
    /* routine vlm */
      double angle,distance;
      double ld, la;
      double l, g, rfq;
      double target_long=lon1;
      double longitude=lon0;
      double target_lat=lat1;
      double latitude=lat0;
      if (target_long > longitude) {
            if (target_long - longitude > M_PI) {
              target_long -= TWO_PI;
            }
          } else if (longitude - target_long > M_PI) {
            longitude -= TWO_PI;
          }
          if (fabs(target_lat-latitude) < degToRad(0.001)) {
            /* clamp to horizontal */
            if (fabs(target_long-longitude) < M_PI) {
              angle = ((target_long-longitude)>0) ? M_PI_2 : -M_PI_2;
              distance = fabs(60.0 * cos((latitude+target_lat)/2)
                               * radToDeg(target_long - longitude));
            } else {
              angle = ((target_long-longitude)>0) ? -M_PI_2 : M_PI_2;
              distance = fabs(60.0 * cos((latitude+target_lat)/2)
                               * (360.0 - radToDeg(target_long - longitude)));
            }
            return distance;
          }
          if (fabs(target_long-longitude) < degToRad(0.001)) {
            /* clamp to vertical */
            distance = fabs(60.0 * radToDeg(target_lat-latitude));
            angle = ((target_lat-latitude) > 0) ? 0 : M_PI;
            return distance;
          }
          ld = log(tan(M_PI_4 + (latitude/2.0)));
          la = log(tan(M_PI_4 + (target_lat/2.0)));
          l = target_lat - latitude;
          g = target_long - longitude;
          rfq = atan(fabs(g/(la-ld)));
          if (l>0.0) {
            if (g > 0.0) {
              angle = rfq;
            } else {
              angle = TWO_PI - rfq;
            }
          } else {
            if (g > 0.0) {
              angle = PI - rfq;
            } else {
              angle = PI + rfq;
            }
          }
          if (degToRad(rfq) > 89.0) {
            distance = (60*fabs(radToDeg(g))*cos((latitude+target_lat)/2)) / sin(rfq);
          } else {
            distance = (60*fabs(radToDeg(l))) / cos(rfq);
          }
    return distance;


#endif
}
