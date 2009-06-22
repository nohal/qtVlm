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
Projection::Projection(int w, int h, float cx, float cy) {
    scalemax = 50000;
    dscale = 1.2;
    init(w, h, cx, cy);
}

//--------------------------------------------------------------
void Projection::init(int w, int h, float cx, float cy) {
    CX = cx;
    CY = cy;
    W = w;
    H = h;

    // Echelle (garde la plus petite en largeur ou hauteur)
    float sx, sy;
    sx = W/360.0;
    sy = H/180.0;
    scale = (sx<sy) ? sx : sy;
    updateBoundaries();
}


//--------------------------------------------------------------
// Ajustements
//--------------------------------------------------------------
void Projection::setCentralPixel(int i, int j)
{
    double x, y;
    screen2map(i, j, &x, &y);
    while (x > 180.0) {
        x -= 360.0;
    }
    while (x < -180.0) {
        x += 360.0;
    }
    CX = (float)x;
    CY = (float)y;
    updateBoundaries();
}
//--------------------------------------------------------------
void Projection::setCenterInMap(float x, float y)
{
    while (x > 180.0) {
        x -= 360.0;
    }
    while (x < -180.0) {
        x += 360.0;
    }
    CX = x;
    CY = y;
    updateBoundaries();
}
//--------------------------------------------------------------
void Projection::setScreenSize(int w, int h) {
    W = w;
    H = h;
    updateBoundaries();
}

//--------------------------------------------------------------
void Projection::updateZoneSelected(float x0, float y0, float x1, float y1)
{
    // Nouvelle position du centre
    CX = (x0+x1)/2.0;
    CY = (y0+y1)/2.0;

    // sécurité
    if (x1 == x0) {
    	x1 = x0+0.1;
	}
    if (y1 == y0) {
    	y1 = y0+0.1;
	}
    // Echelle (garde la plus petite en largeur ou hauteur)
    float sx, sy;
    sx = fabs(W/(x1-x0));
    sy = fabs(H/(y1-y0)) / dscale;
    scale = (sx<sy) ? sx : sy;
    if (scale > scalemax)
        scale = scalemax;
    updateBoundaries();
}

//--------------------------------------------------------------
void Projection::move(float dx, float dy)
{
    // Nouvelle position du centre
    CX = CX - dx*(xmax-xmin);
    while (CX > 180.0)
        CX -= 360.0;
    while (CX < -180.0)
        CX += 360.0;

    CY = CY - dy*(ymax-ymin);
    if (CY > 90.0)
        CY = 90.0;
    if (CY < -90.0)
        CY = -90.0;

    updateBoundaries();
}

//--------------------------------------------------------------
void Projection::zoom(float k)
{
    setScale(scale*k);
}

//--------------------------------------------------------------
void Projection::setScale(float sc)
{
    float sx, sy, scaleall;
    // scaleall = Zoom sur la terre entière
    sx = W/360.0;
    sy = H/180.0;
    scaleall = (sx<sy) ? sx : sy;

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
    // Extrémités de la zone
    double x0,y0, x1,y1;
    screen2map(-1, -1, &x0, &y0);
    screen2map(getW()+1, getH()+1, &x1, &y1);

    xmax = (float)x1;
    xmin = (float)x0;
    ymax = (float)y0;
    ymin = (float)y1;
    if((getW()*getH())!=0)
        coefremp = 10000.0*fabs( ((xmax-xmin)*(ymax-ymin)) / (getW()*getH()) );
    else
        coefremp = 10000.0;
}

//===============================================================================
void Projection::map2screen(double x, double y, int *i, int *j) const
{
    double scaley = scale*dscale;

    *i =  W/2 + (int) (scale * (x-CX) + 0.5);
    *j =  H/2 - (int) (scaley * (y-CY) + 0.5);
/*    *i =  W/2 + (int) (scale * (x-CX)+0.5);
    *j =  H/2 - (int) (scale * (radToDeg(log(tan(degToRad(y-CY)/2 + M_PI_4))))+0.5);

    if(*i<0 || *i> W)
    {
        qWarning()<< "i out of bound: " << *i;
        *i=-1;
    }

    if(*j<0 || *j>H)
    {
        qWarning()<< "i out of bound: " << *j;
        *j=-1;
    }*/
}

//-------------------------------------------------------------------------------
void Projection::screen2map(int i, int j, double *x, double *y) const
{
    double scaley = scale*dscale;

    *x = (double)(i - W/2 + scale*CX)/ scale;
    *y = (double)(H/2 -j + scaley * CY)/ scaley;
    //*x = (double)((i - W/2 + scale*CX)/ scale);
    //*y = radToDeg(2*atan(exp(degToRad((double)((H/2 -j)/scale))))-M_PI_2)+CY;
}

void Projection::test(void)
{
    float x= 0;
    float y = 45;
    int i,j;
    qWarning() << "W=" << W << " H=" << H;
    qWarning() << "Cx=" << CX << " Cy=" << CY;

    qWarning() << "Scale= " << scale ;

    qWarning() << "x-CX=" << x-CX << " prod=" << scale * (x-CX) + 0.5;

    double scaley = scale*dscale;

    //*j =  H/2 - (int) (scaley * (y-CY) + 0.5);
    //*y = (double)(H/2 -j + scaley * CY)/ scaley;

    i =  W/2 + (int) (scale * (x-CX) + 0.5);
    j =  H/2 - (int) (scale * (radToDeg(log(tan(degToRad(y-CY)/2 + M_PI_4)))));

    y = radToDeg(2*atan(exp(degToRad((double)((H/2 -j)/scale))))-M_PI_2)+CY;

    qWarning() << i << "," << j << " => " << y;

}

