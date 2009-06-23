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

#include <iostream>
#include <cmath>
#include <QDebug>

#include "Projection.h"

//-----------------------------
// Constructeur
//-----------------------------
Projection::Projection(int w, int h, double cx, double cy) {
    scalemax = 10000;
    init(w, h, cx, cy);
}

//--------------------------------------------------------------
void Projection::init(int w, int h, double cx, double cy)
{
    W = w;
    H = h;

    // Echelle (garde la plus petite en largeur ou hauteur)
    computeScalleAll();

    scale = scaleall;

    setCenterInMap(cx,cy);
}

void Projection::computeScalleAll(void)
{
    double sx, sy,sy1,sy2;
    sx = W/360.0;
    sy1=log(tan(degToRad(89.9)/2 + M_PI_4));
    sy2=log(tan(degToRad(-89.9)/2 + M_PI_4));
    sy = H/fabs(radToDeg(sy1-sy2));;
    //sy=H/180.0;
    scaleall = (sx<sy) ? sy : sx;
}

//--------------------------------------------------------------
// Ajustements


//--------------------------------------------------------------
void Projection::setScreenSize(int w, int h)
{
    W = w;
    H = h;

    /* update scale all */
    computeScalleAll();

    if (scale < scaleall)
        scale = scaleall;

    emit newZoom(scale);

    updateBoundaries();
}

//--------------------------------------------------------------
void Projection::setCentralPixel(int i, int j)
{
    double x, y;
    screen2map(i, j, &x, &y);
    setCenterInMap(x,y);
}
//--------------------------------------------------------------
void Projection::setCenterInMap(double x, double y)
{
    while (x > 180.0) {
        x -= 360.0;
    }
    while (x < -180.0) {
        x += 360.0;
    }
    CX = x;
    CY = y;

    /* compute projection */
    PX=CX;
    PY=radToDeg(log(tan(degToRad(CY)/2 + M_PI_4)));

    updateBoundaries();
}

//--------------------------------------------------------------
void Projection::updateZoneSelected(double x0, double y0, double x1, double y1)
{
    // security
    if (x1 == x0)
        x1 = x0+0.1;
    if (y1 == y0)
        y1 = y0+0.1;

    //qWarning() << "Zoom select: " << x0 << " " << x1 << " " << y0 << " " << y1;
    //qWarning() << "Scale= " << scale ;

    xW=x0;
    xE=x1;
    yN=y0;
    yS=y1;

    // compute scale;
    double sX,sY,sYN,sYS;
    sX=W/fabs(xE-xW);
    sYN=log(tan(degToRad(yN)/2 + M_PI_4));
    sYS=log(tan(degToRad(yS)/2 + M_PI_4));
    sY=H/fabs(radToDeg(sYN-sYS));
    scale=sX<sY?sY:sX;

    if (scale > scalemax)
        scale = scalemax;

    emit newZoom(scale);

    // Nouvelle position du centre
    //screen2map(W/2,H/2,&CX,&CY);
    CX=(xE+xW)/2;
    CY=(yN+yS)/2;
    PX=CX;
    PY=radToDeg(log(tan(degToRad(CY)/2 + M_PI_4)));

    if((getW()*getH())!=0)
        coefremp = 10000.0*fabs( ((xE-xW)*(yN-yS)) / (getW()*getH()) );
    else
        coefremp = 10000.0;
}

//--------------------------------------------------------------
void Projection::move(double dx, double dy)
{
    // Nouvelle position du centre
    setCenterInMap(CX - dx*(xE-xW),CY - dy*(yN-yS));
}

//--------------------------------------------------------------
void Projection::zoom(float k)
{
    setScale(scale*k);
}

//--------------------------------------------------------------
void Projection::setScale(float sc)
{
    scale = sc;
    if (scale < scaleall)
        scale = scaleall;
    if (scale > scalemax)
        scale = scalemax;
    emit newZoom(scale);
    updateBoundaries();
}

float Projection::getScale()
{
    return scale;
}

//--------------------------------------------------------------
// Intersection avec une zone de la carte
//--------------------------------------------------------------
void Projection::updateBoundaries() {
    /* lat */
    yS=radToDeg(2*atan(exp((double)(degToRad(PY-H/(2*scale)))))-M_PI_2);
    yN=radToDeg(2*atan(exp((double)(degToRad(PY+H/(2*scale)))))-M_PI_2);

    /* lon */
    xW=PX-W/(2*scale);
    xE=PX+W/(2*scale);

    //qWarning() << "Px=" << PX << " Py="  << PY;
    //qWarning() << "Scale= " << scale ;

    /* xW and yN => upper corner */

    if((getW()*getH())!=0)
        coefremp = 10000.0*fabs( ((xE-xW)*(yN-yS)) / (getW()*getH()) );
    else
        coefremp = 10000.0;
}

/*
//===============================================================================
void Projection::map2screen(double x, double y, int *i, int *j) const
{
    if(y<=-90) y=-89.9;
    if(y>=90) y=89.9;

    *i = (int) (scale * (x-xW));
    *j = H/2 + (int) (scale * (PY-radToDeg(log(tan(degToRad(y)/2 + M_PI_4)))));
}

//-------------------------------------------------------------------------------
void Projection::screen2map(int i, int j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}*/
