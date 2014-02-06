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
#include "Util.h"
#include "AngleUtil.h"
#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif
#define scalemax 4e8
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
        void setCentralPixel(const QPointF &c);
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
        void screen2map(const QPoint & screenCoord,QPointF * position) const;
        void screen2mapDouble(const double &i, const double &j, double *x, double *y) const;
        void screen2mapDouble(const QPointF & screenCoord,QPointF * position) const;
        QPointF screen2mapDouble(const QPointF & screenCoord) const;
        void map2screen(const double &x, const double &y, int *i, int *j) const;
        void map2screenDouble(const double &x, const double &y, double *i, double *j) const;
        void map2screenByReference(const double &ref, const double &refX, const double &x, const double &y, double *i, double *j) const;
        void map2screenByReference(const double &ref, const double &refX, const double &x, const double &y, int *i, int *j) const;

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
        double mySignedDiffAngle(const double &a1, const double &a2) const;
        int W, H;     // taille de la fenetre (pixels)
        double CX, CY;                  // centre de la vue (longitude/latitude)
        double xW, xE, yN, yS;  // fenetre visible (repere longitude/latitude)
        double PX,PY;       // center in mercator projection
        double scale;       // Echelle courante
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
#if 0
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
#endif
//-------------------------------------------------------------------------------
inline void Projection::screen2map(const int &i, const int &j, double *x, double *y) const {
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}

inline void Projection::screen2map(const QPoint & screenCoord,QPointF * position) const {
    position->setX((double)(screenCoord.x()/scale+xW));
    position->setY(radToDeg((2*atan(exp((double)(degToRad(PY-(screenCoord.y()-H/2)/scale)))))-M_PI_2));
}

inline void Projection::screen2mapDouble(const double &i, const double &j, double *x, double *y) const
{
    *x = (double)(i/scale+xW);
    *y = radToDeg((2*atan(exp((double)(degToRad(PY-(j-H/2)/scale)))))-M_PI_2);
}

inline void Projection::screen2mapDouble(const QPointF & screenCoord, QPointF * position) const
{
    position->setX((double)(screenCoord.x()/scale+xW));
    position->setY(radToDeg((2*atan(exp((double)(degToRad(PY-(screenCoord.y()-H/2)/scale)))))-M_PI_2));
}

inline QPointF Projection::screen2mapDouble(const QPointF & screenCoord) const
{
    QPointF position;
    position.setX((double)(screenCoord.x()/scale+xW));
    position.setY(radToDeg((2*atan(exp((double)(degToRad(PY-(screenCoord.y()-H/2)/scale)))))-M_PI_2));
    return position;
}

//-------------------------------------------------------------------------------
inline bool Projection::intersect (const double &w, const double &e, const double &s, const double &n) const
{
    return ! (w>xE || e<xW || s>yN || n<yS);
}

//-------------------------------------------------------------------------------
inline bool Projection::isPointVisible (const double &x, const double &y) const
{
//    if(y>yN || y<yS) return false;
//    if(x<=xE && x>=xW) return true;
//    if(xW<=x && xE>=180 && x>=0) return true;
//    if(xW<=-180 && xE>=0 && x<=0) return true;
    double A,B;
    this->map2screenDouble(x,y,&A,&B);
    return this->isInBounderies(A,B);
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
inline void Projection::map2screen(const double &x, const double &y, int *i, int *j) const
{
    double x1=x;
    double y1=y;
    double i1,j1;
    map2screenDouble(x1,y1,&i1,&j1);
    *i=qRound(i1);
    *j=qRound(j1);
}
inline void Projection::map2screenDouble(const double &x, const double &y, double *i, double *j) const
{
    double y1=y;
    if(y1<=-90) y1=-89.9999999999999999999999999999999999999999999999999999999;
    if(y1>=90) y1=89.999999999999999999999999999999999999999999999999999999999;

    double diff=mySignedDiffAngle(AngleUtil::A360(x),AngleUtil::A360(CX));
    if(diff>0)
    {
        if(360.0-diff<diff) diff-=360.0;
    }
    else if (diff<0)
    {
        if(360.0+diff<qAbs(diff)) diff+=360.0;
    }
    *i = W/2+ scale * diff;
    const double trick=PY-radToDeg(log(tan(degToRad(y1)/(double)2.0 + M_PI_4)));
    *j = ((double)H/2.0 + (scale * trick));
}
inline void Projection::map2screenByReference(const double &ref, const double &refX, const double &x, const double &y, int *i, int *j) const
{
    double I,J;
    map2screenByReference(ref,refX,x,y,&I,&J);
    *i=qRound(I);
    *j=qRound(J);
}
inline void Projection::map2screenByReference(const double &ref, const double &refX, const double &x, const double &y, double *i, double *j) const
{
    double y1=y;
    if(y1<=-90) y1=-89.9999999999999999999999999999999999999999999999999999999;
    if(y1>=90) y1=89.999999999999999999999999999999999999999999999999999999999;
    double diff=mySignedDiffAngle(AngleUtil::A360(x),AngleUtil::A360(ref));
    if(diff>0)
    {
        if(360.0-diff<diff) diff-=360.0;
    }
    else if (diff<0)
    {
        if(360.0+diff<qAbs(diff)) diff+=360.0;
    }
    *i = refX + scale * diff;
    const double trick=PY-radToDeg(log(tan(degToRad(y1)/(double)2.0 + M_PI_4)));
    *j = ((double)H/2.0 + (scale * trick));
}
inline double Projection::mySignedDiffAngle(const double &a1,const double &a2) const
{
    return (AngleUtil::A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
}

#endif




