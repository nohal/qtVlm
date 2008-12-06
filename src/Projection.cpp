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
//printf("scale %f",scale);
    scale = (sx<sy) ? sx : sy;
//printf(" -> %f\n",scale);
    updateBoundaries();
}

//--------------------------------------------------------------
// Ajustements
//--------------------------------------------------------------
void Projection::setCentralPixel(int i, int j)
{
    float x, y;
    screen2map(i, j, &x, &y);
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
    //updateBoundaries();
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
    updateBoundaries();
}


//--------------------------------------------------------------
// Intersection avec une zone de la carte
//--------------------------------------------------------------
void Projection::updateBoundaries() {
    // Extrémités de la zone
    float x0,y0, x1,y1;
    screen2map(-1, -1, &x0, &y0);
    screen2map(getW()+1, getH()+1, &x1, &y1);

    xmax = x1;
    xmin = x0;
    ymax = y0;
    ymin = y1;
    
	emit projectionUpdated(this);
	//emit showMessage("[Proj] update proj (" + QString().setNum((long)this)+")");
    //printf("Projection::updateBoundaries %g %g\n", xmin, xmax);
	
	coefremp = 10000.0*fabs( ((xmax-xmin)*(ymax-ymin)) / (getW()*getH()) );
}


