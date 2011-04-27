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

#define GSHHS_SCL    1.0e-6    /* Convert micro-degrees to degrees */

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
        bool crossing(QLineF traject,QLineF trajectWorld);
        int currentQuality;
    private:
        std::string path;
        FILE *fpoly;
        GshhsPolyCell * allCells[360][180];

        PolygonFileHeader polyHeader;

        void readPolygonFileHeader(FILE *polyfile, PolygonFileHeader *header);
};

#endif // GSHHSPOLYREADER_H
