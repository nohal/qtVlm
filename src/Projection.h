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
#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif
class Projection : public QObject
{
Q_OBJECT
    public:
        /* init */
        Projection(int w, int h, double cx, double cy);
        void setScreenSize(int w, int h);
        ~Projection() {}

        /* zoom */
        void zoom (const double &k);
        void zoomKeep (const double &lon, const double &lat, const double &k);
        void zoomAll(void);
        void zoomOnZone(const double &x0, const double &y0, const double &x1, const double &y1);
        void setScale(const double &sc);


        /* move */
        void move(const double &dx, const double &dy);
        void setCentralPixel(const int &i, const int &j);
        void setCenterInMap(const double &x, const double &y);
        void setScaleAndCenterInMap(const double &sc,const double &x, const double &y);

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
        void screen2map(const int &i, const int &j, double *x, double *y) const;
        void screen2mapDouble(const double &i, const double &j, double *x, double *y) const;
        void map2screen(const double &x, const double &y, int *i, int *j) const;
        void map2screenDouble(const double &x, const double &y, double *i, double *j) const;

        /* position / region validation*/
        bool intersect(const double &w,const double &e,const double &s,const double &n)  const;
        bool isPointVisible (const double &x,const double &y) const;
        bool isInBounderies (const int &x,const int &y) const;
        bool isInBounderies_strict (const int &x,const int &y) const;
        void setFrozen(const bool &b){this->frozen=b;}
        bool getFrozen(void) const {return this->frozen;}
        void setUseTempo(const bool &b){this->useTempo=b;}
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

Q_DECLARE_TYPEINFO(Projection,Q_MOVABLE_TYPE);

//===============================================================================
inline void Projection::map2screen(const double &x, const double &y, int *i, int *j) const
{
    double y1=y;
    if(y1<=-90) y1=-89.9;
    if(y1>=90) y1=89.9;

//    if(xW>0 && x<0)
//        x=360+x;
//    if(xW<0 && x>0)
//        x=x-360;
    *i = qRound (scale * (x-xW));
    *j = H/2 + qRound (scale * (PY-radToDeg(log(tan(degToRad(y1)/2 + M_PI_4)))));
}
inline void Projection::map2screenDouble(const double &x, const double &y, double *i, double *j) const
{
    double y1=y;
    if(y1<=-90) y1=-89.9999999999999999999999999999999999999999999999999999999;
    if(y1>=90) y1=89.999999999999999999999999999999999999999999999999999999999;
    const double diff=x-xW;
    *i = scale * diff;
    const double trick=PY-radToDeg(log(tan(degToRad(y1)/(double)2.0 + M_PI_4)));
    *j = ((double)H/2.0 + (scale * trick));
}

//-------------------------------------------------------------------------------
inline void Projection::screen2map(const int &i, const int &j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}
inline void Projection::screen2mapDouble(const double &i, const double &j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}

//-------------------------------------------------------------------------------
inline bool Projection::intersect (const double &w, const double &e, const double &s, const double &n) const
{
    return ! (w>xE || e<xW || s>yN || n<yS);
}

//-------------------------------------------------------------------------------
inline bool Projection::isPointVisible (const double &x, const double &y) const
{
    return (x<=xE && x>=xW && y<=yN && y>=yS);
}

//-------------------------------------------------------------------------------
inline bool Projection::isInBounderies (const int &x, const int &y) const
{
    return (x>=0 && y>=0 && x<=W && y<=H);
}
inline bool Projection::isInBounderies_strict (const int &x, const int &y) const
{
    return (x>0 && y>0 && x<W && y<H);
}

#endif




