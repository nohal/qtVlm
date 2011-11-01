/**********************************************************************
zUGrib: meteorologic GRIB file data viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

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
***********************************************************************/

#ifndef GSHHSPOLYREADER_H
#define GSHHSPOLYREADER_H

#include <cassert>
#include <iostream>
#include <list>

#include <QImage>
#include <QPainter>

#include "class_list.h"

#include "zuFile.h"
#include "Util.h"
#include "Projection.h"

#define GSHHS_SCL    1.0e-6    /* Convert micro-degrees to degrees */
#  define INTER_MAX_LIMIT 1.0000001
#  define INTER_MIN_LIMIT -0.0000001

struct PolygonFileHeader
     {
        int version;
        int pasx;
        int pasy;
        int xmin;
        int ymin;
        int xmax;
        int ymax;
        int p1;
        int p2;
        int p3;
        int p4;
        int p5;
    } ;

typedef QList<QPointF> contour;
typedef QList<contour> contour_list;

//==========================================================================
class GshhsPolyCell
{
    public:

         GshhsPolyCell(FILE *fpoly, int x0, int y0, Projection *proj, PolygonFileHeader *header);
        ~GshhsPolyCell();

        void  drawMapPlain(QPainter &pnt, double dx,Projection *proj,
                    QColor seaColor, QColor landColor );

        void  drawSeaBorderLines(QPainter &pnt, double dx, Projection *proj);
        QList<QLineF> * getCoasts(){return & coasts;}
    private:
        int nbpoints;
        int x0cell, y0cell;

        FILE *fpoly;

        QList<QLineF> coasts;
        Projection *proj;
        PolygonFileHeader *header;
        contour_list poly1,poly2,poly3,poly4,poly5;

        void DrawPolygonFilled(QPainter &pnt,contour_list * poly,double dx,Projection *proj,QColor color);
        void DrawPolygonContour(QPainter &pnt,contour_list * poly, double dx, Projection *proj);

        void ReadPolygonFile (FILE *polyfile,
                                int x, int y,
                                int pas_x, int pas_y,
                                contour_list *p1, contour_list *p2, contour_list *p3, contour_list *p4, contour_list *p5);

};

class GshhsPolyReader
{
    public:
        GshhsPolyReader(std::string path_);
        ~GshhsPolyReader();

        void drawGshhsPolyMapPlain( QPainter &pnt, Projection *proj,
                    QColor seaColor, QColor landColor );

        void drawGshhsPolyMapSeaBorders( QPainter &pnt, Projection *proj);

        void setQuality(int quality); // 5 levels: 0=low ... 4=full
        bool crossing(QLineF traject, QLineF trajectWorld) const;
        int currentQuality;
        void setProj(Projection * p){this->proj=p;}
    private:
        std::string path;
        FILE *fpoly;
        GshhsPolyCell * allCells[360][180];

        PolygonFileHeader polyHeader;
#if 0
        bool vlm_intersects(QLineF line1,QLineF line2) const;
#endif
        bool my_intersects(QLineF line1,QLineF line2) const;
        void readPolygonFileHeader(FILE *polyfile, PolygonFileHeader *header);
        bool abortRequested;
        Projection * proj;
};
inline bool GshhsPolyReader::crossing(QLineF traject, QLineF trajectWorld) const
{
    if(!proj || proj==NULL) return false;
    if(!proj->isInBounderies(traject.p1().x(),traject.p1().y()) &&
       !proj->isInBounderies(traject.p2().x(),traject.p2().y()))
        return false;
    //QPointF dummy;
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (qMin(trajectWorld.p1().x(),trajectWorld.p2().x()));
    cxmax = (int) ceil  (qMax(trajectWorld.p1().x(),trajectWorld.p2().x()));
    cymin = (int) floor (qMin(trajectWorld.p1().y(),trajectWorld.p2().y()));
    cymax = (int) ceil  (qMax(trajectWorld.p1().y(),trajectWorld.p2().y()));
    int cx, cxx, cy;
    GshhsPolyCell *cel;

    for (cx=cxmin; cx<cxmax; cx++)
    {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;

        for (cy=cymin; cy<cymax; cy++)
        {
            if(this->abortRequested) return false;
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
                if(this->abortRequested) return false;
                if (allCells[cxx][cy+90] == NULL) continue;
                cel = allCells[cxx][cy+90];
                QList<QLineF> *coasts=cel->getCoasts();
                if (coasts->isEmpty()) continue;
                for(int cs=0;cs<coasts->count();cs++)
                {
#if 1
                    if(this->abortRequested)
                    {
                        return false;
                    }
                    if (my_intersects(traject,coasts->at(cs)))
#else
                    QPointF dummy;
                    if(coasts->at(cs).intersect(traject,&dummy)==QLineF::BoundedIntersection)
#endif
                        return true;
                }
            }
        }
    }
    return false;
}
inline bool GshhsPolyReader::my_intersects(QLineF line1,QLineF line2) const
{

    // implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
    const QPointF a = line1.p2() - line1.p1();
    const QPointF b = line2.p1() - line2.p2();
    const QPointF c = line1.p1() - line2.p1();

    const qreal denominator = a.y() * b.x() - a.x() * b.y();
    if (denominator == 0)
        return false;

    const qreal reciprocal = 1 / denominator;
    const qreal na = (b.y() * c.x() - b.x() * c.y()) * reciprocal;

    if (na < INTER_MIN_LIMIT || na > INTER_MAX_LIMIT)
        return false;

    const qreal nb = (a.x() * c.y() - a.y() * c.x()) * reciprocal;
    if (nb < INTER_MIN_LIMIT || nb > INTER_MAX_LIMIT)
        return false;

    return true;
}
#if false
inline bool GshhsPolyReader::vlm_intersects(QLineF line1,QLineF line2) const
{

#if 0
    double longitude=degToRad(line1.x1());
    double latitude=degToRad(line1.y1());
    double new_longitude=degToRad(line1.x2());
    double new_latitude=degToRad(line1.y2());
    double seg_a_longitude=degToRad(line2.x1());
    double seg_a_latitude=degToRad(line2.y1());
    double seg_b_longitude=degToRad(line2.x2());
    double seg_b_latitude=degToRad(line2.y2());

    if (longitude <0)
        longitude += TWO_PI;
    if (new_longitude <0)
        new_longitude += TWO_PI;
    if (seg_a_longitude <0)
        seg_a_longitude += TWO_PI;
    if (seg_b_longitude <0)
        seg_b_longitude += TWO_PI;
    /* now check if the segments are crossing the '0' line */
    if (qAbs(longitude - new_longitude) > PI)
    {
        if (longitude > new_longitude)
            longitude -= TWO_PI;
        else
            new_longitude -= TWO_PI;
    }
    if (qAbs(seg_a_longitude - seg_b_longitude) > PI)
    {
        if (seg_a_longitude > seg_b_longitude)
            seg_a_longitude -= TWO_PI;
        else
            seg_b_longitude -= TWO_PI;
    }
    /* and the last check, is one segment on the negative side, but
       in the TWO-PI range, while the other one is across the 0 line in the 0
       range */
    if (qAbs(longitude - seg_a_longitude) > PI)
    {
        if (longitude > seg_a_longitude)
        {
            longitude -= TWO_PI;
            new_longitude -= TWO_PI;
        }
        else
        {
            seg_a_longitude -= TWO_PI;
            seg_b_longitude -= TWO_PI;
        }

    }
#else
    double longitude=(line1.x1());
    double latitude=(line1.y1());
    double new_longitude=(line1.x2());
    double new_latitude=(line1.y2());
    double seg_a_longitude=(line2.x1());
    double seg_a_latitude=(line2.y1());
    double seg_b_longitude=(line2.x2());
    double seg_b_latitude=(line2.y2());
#endif
    double x, y, x1, x2, t, t_seg,d;

    x1 = (new_longitude - longitude);
    x2 = (seg_b_longitude - seg_a_longitude);
    d = ((seg_b_latitude - seg_a_latitude)*x1 - x2 * (new_latitude - latitude));
    if (d == 0.0)
        return false;
    x = (longitude - seg_a_longitude);
    y = (latitude - seg_a_latitude);
    t = (x2*y - (seg_b_latitude - seg_a_latitude)*x) / d;
    /* out of the first segment... return ASAP */
    if (t < INTER_MIN_LIMIT || t > INTER_MAX_LIMIT)
        return false;
    t_seg = (x1*y - (new_latitude - latitude)*x) / d;
    if (t_seg>=INTER_MIN_LIMIT && t_seg <=INTER_MAX_LIMIT)
        return true;
    return false;
}
#endif
#endif // GSHHSPOLYREADER_H
