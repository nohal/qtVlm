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

//#include "Util.h"
#include "dataDef.h"
//struct vlmPoint
//{
//#warning change vlmPoint from struct to class
//    float       lat;
//    float       lon;
//    VlmPoint    origin;
//    bool        isStart;
//    float       cap;
//    bool        isSplitting;
//    float       etaBcmg;
//    float       capBcmg;
//};

class Projection : public QObject
{
Q_OBJECT
    public:
        /* init */
        Projection(int w, int h, double cx, double cy);
        void setScreenSize(int w, int h);
        ~Projection() {}

        /* zoom */
        void zoom (float k);
        void zoomKeep (double lon,double lat, float k);
        void zoomAll(void);
        void zoomOnZone(double x0, double y0, double x1, double y1);
        void setScale(float sc);


        /* move */
        void move(double dx, double dy);
        void setCentralPixel(int i, int j);
        void setCenterInMap(double x, double y);
        void setScaleAndCenterInMap(float sc,double x, double y);

        /* get internal data */
        int   getW()        const   {return W;}    // taille de l'écran
        int   getH()        const   {return H;}
        double getCX()      const   {return CX;}   // centre
        double getCY()      const   {return CY;}
        float getScale()    const   {return scale;}
        float getCoefremp() const   {return coefremp;}
        double getXmin()    const   {return xW;}
        double getXmax()    const   {return xE;}
        double getYmin()    const   {return yS;}
        double getYmax()    const   {return yN;}

        /* coord conversion */
        void screen2map(int i, int j, double *x, double *y) const;
        void map2screen(double x, double y, int *i, int *j) const;
        void map2screenFloat(double x, double y, float *i, float *j) const;

        /* position / region validation*/
        bool intersect(double w,double e,double s,double n)  const;
        bool isPointVisible (double x,double y) const;
        bool isInBounderies (int x,int y) const;
        void setFrozen(bool b){this->frozen=b;/*emit_projectionUpdated();*/}
        bool getFrozen(void){return this->frozen;}
    signals:
        void newZoom(float);
        void projectionUpdated(void);

    private:
        int W, H;     // taille de la fenêtre (pixels)
        double CX, CY;                  // centre de la vue (longitude/latitude)
        double xW, xE, yN, yS;  // fenetre visible (repere longitude/latitude)
        double PX,PY;       // center in mercator projection
        float scale;       // Echelle courante
        float scalemax;    // Echelle maxi
        float scaleall;    // Echelle pour afficher le monde entier
        float coefremp;	   // Coefficient de remplissage (surface_visible/pixels)

        void updateBoundaries();
        void my_setScreenSize(int w, int h);
        void my_setScale(float sc);
        void my_setCenterInMap(double x, double y);
        QTimer * timer;
        void emit_projectionUpdated(void);
        bool frozen;

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
    *i = (int) (scale * (x-xW));
    *j = H/2 + (int) (scale * (PY-radToDeg(log(tan(degToRad(y)/2 + M_PI_4)))));
}
inline void Projection::map2screenFloat(double x, double y, float *i, float *j) const
{
    if(y<=-90) y=-89.9999999999999999999999999999999999999999999999999999999;
    if(y>=90) y=89.999999999999999999999999999999999999999999999999999999999;
    float diff=(float)x-(float)xW;
    *i = ((float)scale * diff);
    *j = (float)H/2.0 + ((float)scale * ((float)PY-radToDeg(log(tan((float)degToRad(y)/2 + (float)M_PI_4)))));
}

//-------------------------------------------------------------------------------
inline void Projection::screen2map(int i, int j, double *x, double *y) const
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

#endif




