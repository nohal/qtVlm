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

#ifndef GshhsRangsREADER_H
#define GshhsRangsREADER_H

#include <stdio.h>
#include <string>
#include <math.h>
#include <assert.h>

#include <QPainter>

#include "class_list.h"

#include "Util.h"
#include "Projection.h"

//-------------------------------------------------------------------------
class GshhsRangsPoint 
{
    public :
        GshhsRangsPoint(double x=0, double y=0, bool isCellBorder=false) {
            this->x = x;
            this->y = y;
            this->isCellBorder = isCellBorder;
        }
        
        double x, y;
        bool  isCellBorder;
};

//-------------------------------------------------------------------------
class GshhsRangsPolygon 
{
    public :
        ~GshhsRangsPolygon() {
                Util::cleanListPointers(lsPoints);
            }
   
        std::list<GshhsRangsPoint *> lsPoints;
        int interior;
        int dataType;
/*            Interior =  0 inside is ocean
                        1 inside is land
                        2 inside is lake on land
                        3 inside is island in lake
                        4 inside is pond on island*/
};

//==========================================================================
class GshhsRangsCell
{
    public:

         GshhsRangsCell(FILE *fcat, FILE *fcel, FILE *frim, int x0, int y0, Projection *proj);
        ~GshhsRangsCell() {
                Util::cleanListPointers(lsPolygons);
            }
        
        void  drawMapPlain(QPainter &pnt, double dx, QPoint *pts, Projection *proj,
                    QColor seaColor, QColor landColor );

        void  drawSeaBorderLines(QPainter &pnt, double dx, Projection *proj);
		
        uint  getPoligonSizeMax() {return poligonSizeMax;}
        QList<QLineF> * getCoasts(){return & this->coasts;}
    
    private:
        int nbpoints;
        int x0cell, y0cell;
        // int x0debug, y0debug;
        int poligonSizeMax;
        
        FILE *fcat, *fcel, *frim;
        std::list<GshhsRangsPolygon *> lsPolygons;

        inline int readInt1(FILE *f);
        inline int readInt2(FILE *f);
        inline int readInt4(FILE *f);

        bool readPolygonList();
        int  readSegmentLoop();
        void readSegmentRim(int RimAddress, int RimLength, GshhsRangsPolygon *polygon);
        QList<QLineF> coasts;
        Projection *proj;

};

//==========================================================================
class GshhsRangsReader
{
    public:
        GshhsRangsReader(std::string path_);
        ~GshhsRangsReader();

        void drawGshhsRangsMapPlain( QPainter &pnt, Projection *proj,
                    QColor seaColor, QColor landColor );
        
        void drawGshhsRangsMapSeaBorders( QPainter &pnt, Projection *proj);
        
        void setQuality(int quality); // 5 levels: 0=low ... 4=full
        bool crossing(QLineF traject,QLineF trajectWorld);
        int currentQuality;
    private:
        std::string path;
        FILE *fcat, *fcel, *frim;
        GshhsRangsCell * allCells[360][180];
};




//======================================================================
//======================================================================
inline int GshhsRangsCell::readInt1(FILE *f)
{
    unsigned char  buf;
    if (fread(&buf, 1, 1, f) == 1)
    	return buf;
	else
		return 0;
}
//--------------------------------------------------------
inline int GshhsRangsCell::readInt2(FILE *f)
{
    unsigned char buf[2];
    if (fread(buf, 1, 2, f) == 2)
    	return (buf[1]<<8) + (buf[0]);
	else
		return 0;
}
//--------------------------------------------------------
inline int GshhsRangsCell::readInt4(FILE *f)
{
    unsigned char buf[4];
    if (fread(buf, 1, 4, f) == 4)
    	return (buf[3]<<24) + (buf[2]<<16) + (buf[1]<<8) + (buf[0]);
	else
		return 0;
}



#endif
