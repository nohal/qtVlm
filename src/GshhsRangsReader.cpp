/**********************************************************************
zyGrib: meteorological GRIB file viewer
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

#include "GshhsRangsReader.h"
#include <QDebug>
//#include "dataDef.h"

//------------------------------------------------------------------------
GshhsRangsCell::GshhsRangsCell(FILE *fcat_, FILE *fcel_, FILE *frim_, int x0_, int y0_,Projection *proj_)
{
    proj = proj_;
    fcat = fcat_;
    fcel = fcel_;
    frim = frim_;    
   	x0cell = x0_;
    y0cell = y0_;
   	nbpoints = 0;
	poligonSizeMax = 0;
	
	//x0debug=8;  y0debug=39;
	
    // adresse de la cellule lue dans le fichier .cat
    int offset = 4*((89 - y0cell) * 360 + x0cell);
    
    fseek(fcat, offset, SEEK_SET);
    int adrcel = readInt4(fcat)-1;
    fseek(fcel, adrcel, SEEK_SET);
	//if ((x0cell==x0debug && y0cell==y0debug)) printf("\n===== START CELL =========\n");

    readPolygonList();
}

//------------------------------------------------------------------------
bool GshhsRangsCell::readPolygonList()
{
	// if ((x0cell==x0debug && y0cell==y0debug)) printf("\n--- START POLYGON ---\n");
    
    int PolygonByte;
    int size;
    
    PolygonByte = readInt1(fcel);
    //printf("CEL %ld %d PolygonByte\n", ftell(fcel), PolygonByte);

    if (PolygonByte == 0) {
        // printf("*** End_PolygonList ***\n");
        return false;         // Fin de recursion
    }
    else 
    {
        size = readSegmentLoop();
        if (poligonSizeMax <size)
        	poligonSizeMax = size;
        	
        while (readPolygonList())
            {};        // Appel recursif
        return true;
    }
}
    
//------------------------------------------------------------------------
int GshhsRangsCell::readSegmentLoop()
{
    //printf("============  readSegmentLoop ==========\n");
    int i,x, y, SegmentByte, PolygonId;
    int DataType, Clockwise, Interior;
    int RimAddress, RimLength;
    
    GshhsRangsPolygon * newPolygon;
    GshhsRangsPoint * newPoint;

    newPolygon = new GshhsRangsPolygon();
    assert(newPolygon);
    lsPolygons.push_back(newPolygon);
    
    PolygonId = readInt4(fcel);
    DataType = 1;
    
    // if ((x0cell==x0debug && y0cell==y0debug)) printf("\n");
    while (DataType != 0)
    {
        SegmentByte = readInt1(fcel);
        DataType  =  SegmentByte & 7;
        Clockwise =  (SegmentByte>>3) & 1;
        Interior  =  (SegmentByte>>4) & 7;
        // if ((x0cell==x0debug && y0cell==y0debug)) printf("CEL DataType=%d Clockwise=%d Interior=%d\n", DataType,Clockwise, Interior);
        if (DataType != 0) {
            newPolygon->interior = Interior;
            newPolygon->dataType = DataType;
		}
        if (DataType>=1 && DataType<=6)
        {
            for (i=0; i<DataType; i++) {
                nbpoints ++;
                x = readInt4(fcel);
                y = readInt4(fcel);
                //if ((x0cell==x0debug && y0cell==y0debug)) printf("CEL X vertex %d : (%8.4f %8.4f) %d\n", i+1, x/1e6, y/1e6, nbpoints);
                //if(!proj->isPointVisible(x/1.e6, y/1.e6)) continue;
                newPoint = new GshhsRangsPoint(x/1.e6, y/1.e6, true);
                assert(newPoint);
                newPolygon->lsPoints.push_back(newPoint);
            }
        }
        else if (DataType == 7)
        {
            RimAddress = readInt4(fcel) - 1;
            RimLength = readInt4(fcel);
            //if ((x0cell==x0debug && y0cell==y0debug)) printf("CEL  RimAddress=%d  RimLength=%d\n", RimAddress, RimLength);
            readSegmentRim(RimAddress, RimLength, newPolygon);
        }
    }

	//if ((x0cell==x0debug && y0cell==y0debug)) printf("*** END POLY size=%d\n", newPolygon->lsPoints.size());
	return newPolygon->lsPoints.size();
}

//------------------------------------------------------------------------
void GshhsRangsCell::readSegmentRim(
        int RimAddress, int RimLength, GshhsRangsPolygon *polygon)
{
    int i, x, y;
    GshhsRangsPoint * newPoint;
    
    fseek(frim, RimAddress, SEEK_SET);
    for (i=0; i<RimLength; i++)
    {
        nbpoints ++;
        x = readInt4(frim);
        y = readInt4(frim);         
        //if ((x0cell==x0debug && y0cell==y0debug)) printf("++++ RIM vertex %d : (%8.4f %8.4f) %d\n", i+1, x/1e6, y/1e6, nbpoints);
        newPoint = new GshhsRangsPoint(x/1.e6, y/1.e6, false);
        assert(newPoint);
        polygon->lsPoints.push_back(newPoint);
    }
}


//=======================================================================================
//=======================================================================================
void GshhsRangsCell::drawMapPlain(QPainter &pnt, double dx, QPoint *pts, Projection *proj,
            QColor seaColor, QColor landColor )
{
// if (!(x0cell==x0debug && y0cell==y0debug))
// return;


    std::list<GshhsRangsPolygon *>::iterator iterPolygons;
    std::list<GshhsRangsPoint *>::iterator iterPoints;
    GshhsRangsPolygon *poly;
    int xx, yy, oxx=0, oyy=0, nbpts;
    
    QPen landPen(landColor);
    landPen.setWidthF(1.4);
	
    pnt.setRenderHint(QPainter::Antialiasing, true);

    for (iterPolygons=lsPolygons.begin(); iterPolygons!=lsPolygons.end(); iterPolygons++)
    {
        poly = *iterPolygons;
        if(poly->interior==0) continue;

        std::list<GshhsRangsPoint *> lsPts = poly->lsPoints;
        
        int j = 0;
        for (iterPoints=lsPts.begin(); iterPoints!=lsPts.end(); iterPoints++)
        {
            GshhsRangsPoint *pt = *iterPoints;
            proj->map2screen(pt->x+dx, pt->y, &xx, &yy);
            if (j==0 || (oxx!=xx || oyy!=yy))  // elimine les points trop proches
            {
                oxx = xx;
                oyy = yy;
                pts[j].setX(xx);
                pts[j].setY(yy);
                j ++;
            }
        }
        nbpts = j;
        if (poly->interior==1 || poly->interior==3) {
            pnt.setBrush(landColor);
			pnt.setPen(landPen);
		}
        else {
            pnt.setBrush(seaColor);
			pnt.setPen(Qt::transparent);
        }
        pnt.drawPolygon(pts, nbpts);
    }
}

//------------------------------------------------------------------------
void GshhsRangsCell::drawSeaBorderLines(QPainter &pnt, double dx, Projection *proj)
{
    std::list<GshhsRangsPolygon *>::iterator iterPolygons;
    std::list<GshhsRangsPoint *>::iterator iterPoints;
    GshhsRangsPolygon *poly;
    int xx, yy;
    float xxF,yyF;
    coasts.clear();

    for (iterPolygons=lsPolygons.begin(); iterPolygons!=lsPolygons.end(); iterPolygons++)
    {
        poly = *iterPolygons;
        std::list<GshhsRangsPoint *> lsPts = poly->lsPoints;
		
		GshhsRangsPoint *pt;
		QPoint  pstart, p0, p1;		// points on screen
                QPointF pstartF,p0F,p1F;
		double	xstart, ystart, x0,y0,  x1,y1;    // world coordinate
		bool	p0_isCellBorder=true, p1_isCellBorder=true;
		bool	pstart_isCellBorder;
        
        if (lsPts.size() > 1)
        {
            iterPoints=lsPts.begin();
            pt = *iterPoints;
            proj->map2screenFloat(pt->x+dx, pt->y, &xxF, &yyF);
            xx=qRound(xxF);
            yy=qRound(yyF);
            pstart = QPoint(xx, yy);
            pstartF= QPointF(xxF,yyF);
            xstart = pt->x;
            ystart = pt->y;
            pstart_isCellBorder = pt->isCellBorder;
            p0 = QPoint(xx, yy);
            p0F= QPointF(xxF,yyF);
            x0 = pt->x;
            y0 = pt->y;
            p0_isCellBorder = pt->isCellBorder;
            x1 = x0;
            y1 = y0;
            iterPoints++;
            for ( ; iterPoints!=lsPts.end(); iterPoints++)
            {
                pt = *iterPoints;
                proj->map2screenFloat(pt->x+dx, pt->y, &xxF, &yyF);
                xx=qRound(xxF);
                yy=qRound(yyF);
                p1 = QPoint(xx, yy);
                p1F = QPointF(xxF, yyF);
                x1 = pt->x;
                y1 = pt->y;
                p1_isCellBorder = pt->isCellBorder;
//                if(qRound(p0F.x()*10)!=qRound(p1F.x()*10) ||
//                   qRound(p0F.y()*10)!=qRound(p1F.y()*10))
//                if (p0.x()!=xx || p0.y()!=yy)  // elimine les points trop proches
//                {
                    if (pt->isCellBorder)
                    {
                        if (! p0_isCellBorder)   // ne trace pas les bords des cellules
                        {
                            coasts.append(QLineF(p0F,p1F));
                        }
                        else
                        { // relie les points sur des bords differents
                            if (x1!=x0 && y1!=y0)
                            {
                                coasts.append(QLineF(p0F,p1F));
                            }
                        }
                    }
                    else
                    {	// point interieur : on trace
                        coasts.append(QLineF(p0F,p1F));
                    }
//                }

                if (p0.x()!=xx || p0.y()!=yy)  // elimine les points trop proches
                {
                    if (pt->isCellBorder)
                    {
                        if (! p0_isCellBorder)   // ne trace pas les bords des cellules
                        {
                            pnt.drawLine(p0, p1);
                        }
                        else
                        { // relie les points sur des bords differents
                            if (x1!=x0 && y1!=y0)
                            {
                                pnt.drawLine(p0, p1);
                            }
                        }
                    }
                    else
                    {	// point interieur : on trace
                        pnt.drawLine(p0, p1);
                    }
                }
                p0 = p1;
                p0F= p1F;
                x0 = x1;
                y0 = y1;
                p0_isCellBorder = p1_isCellBorder;
            }

            // closed polygone ?
            if (! p1_isCellBorder)
            {
//                if (!proj->getFrozen())
                    pnt.drawLine(pstart, p1);
//                else
                    coasts.append(QLineF(pstartF,p1F));
            }
            else if (pstart_isCellBorder && xstart!=x1 && ystart!=y1)
            {
//                if (!proj->getFrozen())
                    pnt.drawLine(pstart, p1);
//                else
                    coasts.append(QLineF(pstartF,p1F));
            }
        }
    }
#if 0 //debug
    QPen save=pnt.pen();
    QPen myPen=save;
    myPen.setColor(Qt::red);
    pnt.setPen(myPen);
    foreach(QLineF coast,coasts)
        pnt.drawLine(coast);
    pnt.setPen(save);
#endif
}

//========================================================================
//========================================================================
//========================================================================
//                GshhsRangsReader
//========================================================================
//========================================================================
//========================================================================
GshhsRangsReader::GshhsRangsReader(std::string rangspath)
{
    path = rangspath+"/";
    fcat = NULL;
    fcel = NULL;
    frim = NULL;

	for (int i=0; i<360; i++) {
		for (int j=0; j<180; j++) {
			allCells[i][j] = NULL;
		}
	}
	currentQuality = -1;
    setQuality(1);
}
bool GshhsRangsReader::crossing(QLineF traject,QLineF trajectWorld)
{
    QPointF dummy;
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (qMin(trajectWorld.p1().x(),trajectWorld.p2().x()));
    cxmax = (int) ceil  (qMax(trajectWorld.p1().x(),trajectWorld.p2().x()));
    cymin = (int) floor (qMin(trajectWorld.p1().y(),trajectWorld.p2().y()));
    cymax = (int) ceil  (qMax(trajectWorld.p1().y(),trajectWorld.p2().y()));
    int cx, cxx, cy;
    GshhsRangsCell *cel;

    for (cx=cxmin; cx<cxmax; cx++)
    {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;

        for (cy=cymin; cy<cymax; cy++)
        {
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
                if (allCells[cxx][cy+90] == NULL) continue;
                cel = allCells[cxx][cy+90];
                QList<QLineF> *coasts=cel->getCoasts();
                if (coasts->isEmpty()) continue;
                for(int cs=0;cs<coasts->count();cs++)
                {
                    if(coasts->at(cs).intersect(traject,&dummy)==QLineF::BoundedIntersection)
                        return true;
                }
            }
        }
    }
    return false;
}

//-------------------------------------------------------------------------
GshhsRangsReader::~GshhsRangsReader()
{
	for (int i=0; i<360; i++) {
		for (int j=0; j<180; j++) {
			if (allCells[i][j] != NULL)
			{
				delete allCells[i][j];
				allCells[i][j] = NULL;
			}
		}
	}
}

//-------------------------------------------------------------------------
void GshhsRangsReader::setQuality(int quality)  // 5 levels: 0=low ... 4=full
{    
	if (currentQuality != quality)
	{
		currentQuality = quality;
		
		char txtn[16];
		int qual = 4-quality;   // Fichier .rim : 0=meilleure ... 4=grossière
		if (qual < 0)   qual = 0;
		if (qual > 4)   qual = 4;
		
		snprintf(txtn, 10, "%d", qual);

		if (fcat)
			fclose(fcat);
		if (fcel)
			fclose(fcel);
		if (frim)
			fclose(frim);
		
		fcat = fopen( (path+"rangs_"+txtn+".cat").c_str(), "rb");
		fcel = fopen( (path+"rangs_"+txtn+".cel").c_str(), "rb");
		frim = fopen( (path+"gshhs_"+txtn+".rim").c_str(), "rb");
		
		for (int i=0; i<360; i++) {
			for (int j=0; j<180; j++) {
				if (allCells[i][j] != NULL)
				{
					delete allCells[i][j];
					allCells[i][j] = NULL;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------
void GshhsRangsReader::drawGshhsRangsMapPlain( QPainter &pnt, Projection *proj,
                    QColor seaColor, QColor landColor )
{
    if (!fcat || !fcel || !frim)
        return;
        
    QPoint *pts;
    uint ptsSize = 1000;		// Resolution Max => 9145 pts max
    pts = new QPoint[ptsSize];
    assert(pts);
    
    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (proj->getXmin());
    cxmax = (int) ceil  (proj->getXmax());
    cymin = (int) floor (proj->getYmin());
    cymax = (int) ceil  (proj->getYmax());
    int dx, cx, cxx, cy;
	GshhsRangsCell *cel;

//printf("cxmin=%d cxmax=%d    cymin=%d cymax=%d\n", cxmin,cxmax, cymin,cymax);
    for (cx=cxmin; cx<cxmax; cx++) {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;
        
        for (cy=cymin; cy<cymax; cy++) {
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
            	if (allCells[cxx][cy+90] == NULL) {
                                        cel = new GshhsRangsCell(fcat, fcel, frim, cxx, cy,proj);
					assert(cel);
					allCells[cxx][cy+90] = cel;
				}
				else {
					cel = allCells[cxx][cy+90];
				}
                if (ptsSize <= cel->getPoligonSizeMax()) {
    				delete [] pts;
                	ptsSize = cel->getPoligonSizeMax()+1000;
					pts = new QPoint[ptsSize];
					assert(pts);
                }
                dx = cx-cxx;
                cel -> drawMapPlain(pnt, dx, pts, proj, seaColor, landColor);
            }
        }
    }

    delete [] pts;
}

//-------------------------------------------------------------------------
void GshhsRangsReader::drawGshhsRangsMapSeaBorders( QPainter &pnt, Projection *proj)
{
    if (!fcat || !fcel || !frim)
        return;

    int cxmin, cxmax, cymax, cymin;  // cellules visibles
    cxmin = (int) floor (proj->getXmin());
    cxmax = (int) ceil  (proj->getXmax());
    cymin = (int) floor (proj->getYmin());
    cymax = (int) ceil  (proj->getYmax());
    int dx, cx, cxx, cy;
	GshhsRangsCell *cel;

    for (cx=cxmin; cx<cxmax; cx++) {
        cxx = cx;
        while (cxx < 0)
            cxx += 360;
        while (cxx >= 360)
            cxx -= 360;
        
        for (cy=cymin; cy<cymax; cy++) {
            if (cxx>=0 && cxx<=359 && cy>=-90 && cy<=89)
            {
            	if (allCells[cxx][cy+90] == NULL) {
                                        cel = new GshhsRangsCell(fcat, fcel, frim, cxx, cy,proj);
					assert(cel);
					allCells[cxx][cy+90] = cel;
				}
				else {
					cel = allCells[cxx][cy+90];
				}
                dx = cx-cxx;
                cel->drawSeaBorderLines(pnt, dx, proj);
            }
        }
    }
}











