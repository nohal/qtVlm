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

#ifndef PROJECTION_H
#define PROJECTION_H

class Projection;

#include "Util.h"

#include <QObject>
class Projection : public QObject
{
Q_OBJECT
    public:
        Projection(int w, int h, float lon, float lat);
        virtual ~Projection() {}

        virtual void screen2map(int i, int j, double *x, double *y) const;
        virtual void map2screen(double x, double y, int *i, int *j) const;

        virtual int   getW()  const   {return W;};    // taille de l'écran
        virtual int   getH()  const   {return H;};
        virtual float getCX() const   {return CX;};   // centre
        virtual float getCY() const   {return CY;};
        virtual float getScale() const      {return scale;};
        virtual float getCoefremp() const   {return coefremp;};

        // zone visible (longitude/latitude)
        virtual float getXmin() const   {return xmin;};
        virtual float getXmax() const   {return xmax;};
        virtual float getYmin() const   {return ymin;};
        virtual float getYmax() const   {return ymax;};

        virtual bool intersect(float w,float e,float s,float n)  const;
        virtual bool isPointVisible (float x,float y) const;
        virtual bool isInBounderies (int x,int y) const;

        virtual void setScale(float sc);
        virtual float getScale();
        virtual void setCentralPixel(int i, int j);
        virtual void setCenterInMap(float x, float y);
        virtual void setScreenSize(int w, int h);
        virtual void updateZoneSelected(float x0, float y0, float x1, float y1);
        virtual void init(int w, int h, float cx, float cy);
        virtual void zoom (float k);
        virtual void move (float dx, float dy);

        void test();

    signals:
        void newZoom(float);
    private:
        int W, H;     // taille de la fenêtre (pixels)

        float CX, CY;                  // centre de la vue (longitude/latitude)
        float xmin, xmax, ymax, ymin;  // fenêtre visible (repère longitude/latitude)
        float scale;       // échelle courante
        float scalemax;    // échelle maxi
        float dscale;	   // rapport scaley/scalex
		float coefremp;		// Coefficient de remplissage (surface_visible/pixels)

        virtual void updateBoundaries();
};


#if 0
//===============================================================================
inline void Projection::map2screen(double x, double y, int *i, int *j) const
{
    //double scaley = scale*dscale;

    //*i =  W/2 + (int) (scale * (x-CX) + 0.5);
    //*j =  H/2 - (int) (scaley * (y-CY) + 0.5);
    *i =  W/2 + (int) (scale * (x-CX) + 0.5);
    *j =  H/2 - (int) (scale * (radToDeg(log(tan(degToRad(y)/2 + M_PI_4))-CY) + 0.5));
}

//-------------------------------------------------------------------------------
inline void Projection::screen2map(int i, int j, double *x, double *y) const
{
    //double scaley = scale*dscale;

    //*x = (double)(i - W/2 + scale*CX)/ scale;
    //*y = (double)(H/2 -j + scaley * CY)/ scaley;
    *x = (double)((i - W/2 + scale*CX)/ scale);
    *y = (double)((H/2 - radToDeg(2*atan(exp(j))+M_PI_2) + scale * CY)/ scale);
}
#endif

//-------------------------------------------------------------------------------
inline bool Projection::intersect (float w,float e,float s,float n) const
{
    return ! (w>xmax || e<xmin || s>ymax || n<ymin);
}

//-------------------------------------------------------------------------------
inline bool Projection::isPointVisible (float x,float y) const
{
    return (x<=xmax && x>=xmin && y<=ymax && y>=ymin);
}

//-------------------------------------------------------------------------------
inline bool Projection::isInBounderies (int x,int y) const
{
    return (x>=0 && y>=0 && x<=W && y<=H);
}

#endif




