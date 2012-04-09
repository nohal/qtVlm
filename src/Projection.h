/**********************************************************************
qtVlm: Loup de mer GUI
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

#include <QTimer>

#include <QObject>
#include "dataDef.h"
#include <QDebug>
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616

class Projection : public QObject
{
Q_OBJECT
    public:
        /* init */
        Projection(int w, int h, double cx, double cy);
        void setScreenSize(int w, int h);
        ~Projection() {}

        /* zoom */
        void zoom (double k);
        void zoomKeep (double lon,double lat, double k);
        void zoomAll(void);
        void zoomOnZone(double x0, double y0, double x1, double y1);
        void setScale(double sc);


        /* move */
        void move(double dx, double dy);
        void setCentralPixel(int i, int j);
        void setCenterInMap(double x, double y);
        void setScaleAndCenterInMap(double sc,double x, double y);

        /* get internal data */
        int   getW()        const   {return W;}    // taille de l'ecran
        int   getH()        const   {return H;}
        double getCX()      const   {return CX;}   // centre
        double getCY()      const   {return CY;}
        double getScale()    const   {return scale;}
        double getCoefremp() const   {return coefremp;}
        double getXmin()    const   {return xW;}
        double getXmax()    const   {return xE;}
        double getYmin()    const   {return yS;}
        double getYmax()    const   {return yN;}

        /* coord conversion */
        void screen2map(int i, int j, double *x, double *y) const;
        void screen2mapDouble(double i, double j, double *x, double *y) const;
        void map2screen(double x, double y, int *i, int *j) const;
        void map2screenDouble(double x, double y, double *i, double *j) const;

        /* position / region validation*/
        bool intersect(double w,double e,double s,double n)  const;
        bool isPointVisible (double x,double y) const;
        bool isInBounderies (int x,int y) const;
        bool isInBounderies_strict (int x,int y) const;
        void setFrozen(bool b){this->frozen=b;}
        bool getFrozen(void){return this->frozen;}
        void setUseTempo(bool b){this->useTempo=b;}
    signals:
        void newZoom(double);
        void projectionUpdated(void);

    private:
        int W, H;     // taille de la fenetre (pixels)
        double CX, CY;                  // centre de la vue (longitude/latitude)
        double xW, xE, yN, yS;  // fenetre visible (repere longitude/latitude)
        double PX,PY;       // center in mercator projection
        double scale;       // Echelle courante
        double scalemax;    // Echelle maxi
        double scaleall;    // Echelle pour afficher le monde entier
        double coefremp;	   // Coefficient de remplissage (surface_visible/pixels)

        void updateBoundaries();
        void my_setScreenSize(int w, int h);
        void my_setScale(double sc);
        void my_setCenterInMap(double x, double y);
        QTimer * timer;
        void emit_projectionUpdated(void);
        bool frozen;
        bool useTempo;

};


//===============================================================================
inline void Projection::map2screen(double x, double y, int *i, int *j) const
{
    if(y<=-90) y=-89.9;
    if(y>=90) y=89.9;

//    if(xW>0 && x<0)
//        x=360+x;
//    if(xW<0 && x>0)
//        x=x-360;
    *i = qRound (scale * (x-xW));
    *j = H/2 + qRound (scale * (PY-radToDeg(log(tan(degToRad(y)/2 + M_PI_4)))));
}
inline void Projection::map2screenDouble(double x, double y, double *i, double *j) const
{
    if(y<=-90) y=-89.9999999999999999999999999999999999999999999999999999999;
    if(y>=90) y=89.999999999999999999999999999999999999999999999999999999999;
    double diff=x-xW;
    *i = scale * diff;
    double trick=PY-radToDeg(log(tan(degToRad(y)/(double)2.0 + M_PI_4)));
    *j = ((double)H/(double)2.0 + (scale * trick));
}

//-------------------------------------------------------------------------------
inline void Projection::screen2map(int i, int j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}
inline void Projection::screen2mapDouble(double i, double j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}

//-------------------------------------------------------------------------------
inline bool Projection::intersect (double w,double e,double s,double n) const
{
    return ! (w>xE || e<xW || s>yN || n<yS);
}

//-------------------------------------------------------------------------------
inline bool Projection::isPointVisible (double x,double y) const
{
    return (x<=xE && x>=xW && y<=yN && y>=yS);
}

//-------------------------------------------------------------------------------
inline bool Projection::isInBounderies (int x,int y) const
{
    return (x>=0 && y>=0 && x<=W && y<=H);
}
inline bool Projection::isInBounderies_strict (int x,int y) const
{
    return (x>0 && y>0 && x<W && y<H);
}

#endif




