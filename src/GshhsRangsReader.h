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

#ifndef GshhsRangsREADER_H
#define GshhsRangsREADER_H

#include <stdio.h>
#include <string>
#include <math.h>
#include <assert.h>

#include <QPainter>

#include "Projection.h"
#include "Util.h"

//-------------------------------------------------------------------------
class GshhsRangsPoint 
{
    public :
        GshhsRangsPoint(float x=0, float y=0, bool isCellBorder=false) {
            this->x = x;
            this->y = y;
            this->isCellBorder = isCellBorder;
        }
        
        float x, y;
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

         GshhsRangsCell(FILE *fcat, FILE *fcel, FILE *frim, int x0, int y0);
        ~GshhsRangsCell() {
                Util::cleanListPointers(lsPolygons);
            }
        
        void  drawMapPlain(QPainter &pnt, float dx, QPoint *pts, Projection *proj,
                    QColor seaColor, QColor landColor );

        void  drawSeaBorderLines(QPainter &pnt, float dx, Projection *proj);
		
		uint  getPoligonSizeMax() {return poligonSizeMax;}
    
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

    private:
    	int currentQuality;
        std::string path;
        FILE *fcat, *fcel, *frim;
        GshhsRangsCell * allCells[360][180];
};




//======================================================================
//======================================================================
inline int GshhsRangsCell::readInt1(FILE *f)
{
    unsigned char  buf;
    fread(&buf, 1, 1, f);
    return buf;
}
//--------------------------------------------------------
inline int GshhsRangsCell::readInt2(FILE *f)
{
    unsigned char buf[2];
    fread(buf, 1, 2, f);
    return (buf[1]<<8) + (buf[0]);
}
//--------------------------------------------------------
inline int GshhsRangsCell::readInt4(FILE *f)
{
    unsigned char buf[4];
    fread(buf, 1, 4, f);
    return (buf[3]<<24) + (buf[2]<<16) + (buf[1]<<8) + (buf[0]);
}



#endif
