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

#include "GribPlot.h"
#include <QMessageBox>
#include <QObject>

//----------------------------------------------------
GribPlot::GribPlot(const GribPlot &model)
{
	initNewGribPlot (model.mustInterpolateValues, model.drawWindArrowsOnGribGrid);	
        loadGribFile (model.fileName);
	duplicateFirstCumulativeRecord (model.mustDuplicateFirstCumulativeRecord);
}

//----------------------------------------------------
GribPlot::GribPlot(bool interpolateValues_, bool windArrowsOnGribGrid_)
{
	initNewGribPlot(interpolateValues_, windArrowsOnGribGrid_);
}

//----------------------------------------------------
void GribPlot::initNewGribPlot(bool interpolateValues_, bool windArrowsOnGribGrid_)
{
    gribReader = NULL;
    
    mustInterpolateValues = interpolateValues_;
    drawWindArrowsOnGribGrid = windArrowsOnGribGrid_;
    
    mapColorTransp = 210;
    
    windArrowSpace = 28;      // distance mini entre flèches
    windBarbuleSpace = 34;    // distance mini entre flèches
    
    windArrowSpaceOnGrid = 20;      // distance mini entre flèches
    windBarbuleSpaceOnGrid = 28;    // distance mini entre flèches
    
    windArrowSize = 14;       // longueur des flèches
    windBarbuleSize = 26;     // longueur des flèches avec barbules
    
    // Color scale for wind in beaufort
    windColor[ 0].setRgba(qRgba(   0,  80, 255,  mapColorTransp));
    windColor[ 1].setRgba(qRgba(   0, 150, 255,  mapColorTransp));
    windColor[ 2].setRgba(qRgba(   0, 200, 255,  mapColorTransp));
    windColor[ 3].setRgba(qRgba(   0, 250, 180,  mapColorTransp));
    windColor[ 4].setRgba(qRgba(   0, 230, 150,  mapColorTransp));
    windColor[ 5].setRgba(qRgba( 255, 255,   0,  mapColorTransp));
    windColor[ 6].setRgba(qRgba( 255, 220,   0,  mapColorTransp));
    windColor[ 7].setRgba(qRgba( 255, 180,   0,  mapColorTransp));
    windColor[ 8].setRgba(qRgba( 255, 120,   0,  mapColorTransp));
    windColor[ 9].setRgba(qRgba( 230, 120,   0,  mapColorTransp));
    windColor[10].setRgba(qRgba( 220,  80,   0,  mapColorTransp));
    windColor[11].setRgba(qRgba( 200,  50,  30,  mapColorTransp));
    windColor[12].setRgba(qRgba( 170,   0,  50,  mapColorTransp));
    windColor[13].setRgba(qRgba( 150,   0,  30,  mapColorTransp));
}
//----------------------------------------------------
GribPlot::~GribPlot() {
}

//--------------------------------------------------------------------------
QRgb  GribPlot::getWindColor(double v, bool smooth) {
    QRgb rgb = 0;
    int beauf;
    if (! smooth) {
        beauf = Util::kmhToBeaufort(v);
        rgb = windColor[beauf].rgba();
    }
    else {
        // Interpolation de couleur
        double fbeauf = Util::kmhToBeaufort_F(v);
        QColor c1 = windColor[ (int) fbeauf ];
        QColor c2 = windColor[ (int) fbeauf +1 ];
        double dcol = fbeauf-floor(fbeauf);
        rgb = qRgba(
            (int)( c1.red()  *(1.0-dcol) + dcol*c2.red()   +0.5),
            (int)( c1.green()*(1.0-dcol) + dcol*c2.green() +0.5),
            (int)( c1.blue() *(1.0-dcol) + dcol*c2.blue()  +0.5),
            mapColorTransp
            );
    }
    return rgb;
}

//----------------------------------------------------
void GribPlot::loadGribFile(QString fileName)
{
	this->fileName = fileName;
	listDates.clear();
    
    if (gribReader != NULL) {
    	delete gribReader;
        gribReader = NULL;
    }
	
	gribReader = new GribReader(qPrintable(fileName));
    if (gribReader != NULL  &&  gribReader->isOk())
    {
		listDates = gribReader->getListDates();
		setCurrentDate ( listDates.size()>0 ? *(listDates.begin()) : 0);
	}
}

//----------------------------------------------------
void GribPlot::duplicateFirstCumulativeRecord( bool mustDuplicate )
{
	mustDuplicateFirstCumulativeRecord = mustDuplicate;
    if (gribReader != NULL  &&  gribReader->isOk())
    {
		if (mustDuplicate) {
			gribReader->copyFirstCumulativeRecord();
		}
		else {
			gribReader->removeFirstCumulativeRecord();
		}
	}
}

//----------------------------------------------------
void GribPlot::interpolateValues( bool b )
{
	mustInterpolateValues = b;
}
//----------------------------------------------------
void GribPlot::windArrowsOnGribGrid( bool b )
{
	drawWindArrowsOnGribGrid = b;
}

//----------------------------------------------------
void GribPlot::setCurrentDate(time_t t)
{
    currentDate = t;
}

//==========================================================================
// Rectangle translucide sur la zone couverte par les données
void GribPlot::show_CoverZone(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    
    double x0,y0, x1,y1;
    int i, j, k,l;
    if (gribReader->getZoneExtension(&x0,&y0, &x1,&y1))
    {
        pnt.setPen(QColor(120, 120,120));
        pnt.setBrush(QColor(255,255,255,40));
        proj->map2screen(x0,y0, &i, &j);
        proj->map2screen(x1,y1, &k, &l);
        pnt.drawRect(i, j, k-i, l-j);
        proj->map2screen(x0-360.0,y0, &i, &j);
        proj->map2screen(x1-360.0,y1, &k, &l);
        pnt.drawRect(i, j, k-i, l-j);
    }
}

//-------------------------------------------------------------
// Grille GRIB
void GribPlot::draw_GribGrid(QPainter &pnt, const Projection *proj)
{
    
    pnt.setPen(QColor(100,100,100));
    
    if (gribReader == NULL) {
        return;
    }
    GribRecord *rec = gribReader->getFirstGribRecord();
    if (rec == NULL)
        return;
    int px,py, i,j, dl=2;
    for (i=0; i<rec->getNi(); i++)
        for (j=0; j<rec->getNj(); j++)
        {
            if (rec->hasValue(i,j))
            {
                proj->map2screen(rec->getX(i), rec->getY(j), &px,&py);
                pnt.drawLine(px-dl,py, px+dl,py);
                pnt.drawLine(px,py-dl, px,py+dl);
                proj->map2screen(rec->getX(i)-360.0, rec->getY(j), &px,&py);
                pnt.drawLine(px-dl,py, px+dl,py);
                pnt.drawLine(px,py-dl, px,py+dl);
            }
        }
}


//--------------------------------------------------------------------------
// Carte de couleurs générique en dimension 1
//--------------------------------------------------------------------------
void GribPlot::drawColorMapGeneric_1D (
		QPainter &pnt, const Projection *proj, bool smooth,
		GribRecord *rec,
		QRgb (GribPlot::*function_getColor) (double v, bool smooth)
	)
{
    if (rec == NULL)
        return;
    int i, j;
    double x, y, v;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
/*    for (i=0; i<W; i++) {
        for (j=0; j<H; j++) {
            proj->screen2map(i,j, &x, &y);
            if (! rec->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (rec->isPointInMap(x, y)) {
                v = rec->getInterpolatedValue(x, y, mustInterpolateValues);
                if (v != GRIB_NOTDEF) {
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,j, rgb);
                }
            }
        }
    }*/
    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2) {
            proj->screen2map(i,j, &x, &y);
            if (! rec->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (rec->isPointInMap(x, y)) {
                v = rec->getInterpolatedValue(x, y, mustInterpolateValues);
                if (v != GRIB_NOTDEF) {
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }
    
    pnt.drawImage(0,0,*image);
    delete image;
}

//--------------------------------------------------------------------------
// Carte de couleurs générique en dimension 2
//--------------------------------------------------------------------------
void  GribPlot::drawColorMapGeneric_2D (
		QPainter &pnt, const Projection *proj, bool smooth,
		GribRecord *recx, GribRecord *recy,
		QRgb (GribPlot::*function_getColor) (double v, bool smooth)
	)
{
    if (recx == NULL || recy == NULL)
        return;
    int i, j;
    double x, y, vx, vy, v;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            
            if (! recx->isXInMap(x))
                x += 360.0;    // tour complet ?
                
            if (recx->isPointInMap(x, y))
            {
                vx = recx->getInterpolatedValue(x, y, mustInterpolateValues);
                vy = recy->getInterpolatedValue(x, y, mustInterpolateValues);

                if (vx != GRIB_NOTDEF && vx != GRIB_NOTDEF)
                {
                    v = sqrt(vx*vx+vy*vy)*3.6;		// m/s => km/h
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }
	pnt.drawImage(0,0,*image);
    delete image;
}

//--------------------------------------------------------------------------
// Carte de couleurs générique de la différence entre 2 champs
//--------------------------------------------------------------------------
void  GribPlot::drawColorMapGeneric_Abs_Delta_Data (
		QPainter &pnt, const Projection *proj, bool smooth,
		GribRecord *recx,
		GribRecord *recy,
		QRgb (GribPlot::*function_getColor) (double v, bool smooth)
	)
{
    if (recx == NULL || recy == NULL)
        return;
    int i, j;
    double x, y, vx, vy, v;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W-1; i+=2) {
        for (j=0; j<H-1; j+=2)
        {
            proj->screen2map(i,j, &x, &y);
            
            if (! recx->isXInMap(x))
                x += 360.0;    // tour complet ?
                
            if (recx->isPointInMap(x, y))
            {
                vx = recx->getInterpolatedValue(x, y, mustInterpolateValues);
                vy = recy->getInterpolatedValue(x, y, mustInterpolateValues);

                if (vx != GRIB_NOTDEF && vx != GRIB_NOTDEF)
                {
                    v = fabs(vx-vy);
                    rgb = (this->*function_getColor) (v, smooth);
                    image->setPixel(i,  j, rgb);
                    image->setPixel(i+1,j, rgb);
                    image->setPixel(i,  j+1, rgb);
                    image->setPixel(i+1,j+1, rgb);
                }
            }
        }
    }
	pnt.drawImage(0,0,*image);
    delete image;
}


//--------------------------------------------------------------------------
// Carte de couleurs du vent
//--------------------------------------------------------------------------
void GribPlot::draw_WIND_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (gribReader == NULL) {
        return;
    }
    GribRecord *recx, *recy;
    recx = gribReader->getGribRecord(GRB_WIND_VX,LV_ABOV_GND,10,currentDate);
    recy = gribReader->getGribRecord(GRB_WIND_VY,LV_ABOV_GND,10,currentDate);
    drawColorMapGeneric_2D(pnt,proj,smooth, recx,recy, &GribPlot::getWindColor );
}

//--------------------------------------------------------------------------
// Flèches de direction du vent espacées régulièrement
void GribPlot::draw_WIND_Arrows
        (QPainter &pnt, const Projection *proj, bool barbules, QColor arrowsColor)
{
    if (gribReader == NULL) {
        return;
    }
    windArrowColor = arrowsColor;

    GribRecord *recx = gribReader->getGribRecord(GRB_WIND_VX,LV_ABOV_GND,10,currentDate);
    GribRecord *recy = gribReader->getGribRecord(GRB_WIND_VY,LV_ABOV_GND,10,currentDate);
    if (recx == NULL || recy == NULL)
        return;        
    int i, j;
    double x, y, vx, vy;
    int W = proj->getW();
    int H = proj->getH();
    
	int space;    
    if (barbules)
	    space =  drawWindArrowsOnGribGrid ? windBarbuleSpaceOnGrid : windBarbuleSpace;
    else
	    space =  drawWindArrowsOnGribGrid ? windArrowSpaceOnGrid : windArrowSpace;
    
    if (drawWindArrowsOnGribGrid)
    {	// Flèches uniquement sur les points de la grille
    	int oldi=-1000, oldj=-1000;
    	for (int gi=0; gi<recx->getNi(); gi++)
    	{
			x = recx->getX(gi);
			y = recx->getY(0);
			proj->map2screen(x,y, &i,&j);
			if (abs(i-oldi)>=space)
			{
				oldi = i;
				for (int gj=0; gj<recx->getNj(); gj++)
				{
					x = recx->getX(gi);
					y = recx->getY(gj);
					proj->map2screen(x,y, &i,&j);
					
						//----------------------------------------------------------------------
						if (! recx->isXInMap(x))
							x += 360.0;   // tour du monde ?

						if (recx->isPointInMap(x,y)) {
							if (abs(j-oldj)>=space)
							{
								oldj = j;
								vx = recx->getInterpolatedValue(x, y, mustInterpolateValues);
								vy = recy->getInterpolatedValue(x, y, mustInterpolateValues);
								if (vx != GRIB_NOTDEF && vy != GRIB_NOTDEF)
								{
									if (barbules)
										drawWindArrowWithBarbs(pnt, i,j, vx,vy, (y<0), arrowsColor);
									else
										drawWindArrow(pnt, i,j, vx,vy);
								}
							}
						}
				}
			}
    	}
    }
    else
    {	// Flèches uniformément réparties sur l'écran
		for (i=0; i<W; i+=space)
		{
			for (j=0; j<H; j+=space)
			{
				proj->screen2map(i,j, &x,&y);
				//----------------------------------------------------------------------    			
				if (! recx->isXInMap(x))
					x += 360.0;   // tour du monde ?
				if (recx->isPointInMap(x,y)) {
					vx = recx->getInterpolatedValue(x, y, mustInterpolateValues);
					vy = recy->getInterpolatedValue(x, y, mustInterpolateValues);
					if (vx != GRIB_NOTDEF && vy != GRIB_NOTDEF)
					{
						if (barbules)
							drawWindArrowWithBarbs(pnt, i,j, vx,vy, (y<0), arrowsColor);
						else
							drawWindArrow(pnt, i,j, vx,vy);
					}
				}
				//----------------------------------------------------------------------    			
			}
		}
	}
}

//-----------------------------------------------------------------------------
void GribPlot::drawTransformedLine( QPainter &pnt,
        double si, double co,int di, int dj, int i,int j, int k,int l)
{
    int ii, jj, kk, ll;
    ii = (int) (i*co-j*si +0.5) + di;
    jj = (int) (i*si+j*co +0.5) + dj;
    kk = (int) (k*co-l*si +0.5) + di;
    ll = (int) (k*si+l*co +0.5) + dj;
    // Clip forcé à cause d'un bug qpixmap sous windows
    int w = pnt.device()->width();
    int h = pnt.device()->height();
    if (       Util::isInRange(ii, 0, w)
            && Util::isInRange(kk, 0, w)
            && Util::isInRange(jj, 0, h)
            && Util::isInRange(ll, 0, h) )
        pnt.drawLine(ii, jj, kk, ll);
}
//-----------------------------------------------------------------------------
void GribPlot::drawWindArrow(QPainter &pnt, int i, int j, double vx, double vy)
{
    double ang = atan2(vy, -vx);
    double si=sin(ang),  co=cos(ang);
    QPen pen( windArrowColor);
    pen.setWidth(2);
    pnt.setPen(pen);
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0, windArrowSize, 0);   // hampe
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5, 2);   // flèche
    drawTransformedLine(pnt, si,co, i-windArrowSize/2,j,  0,0,  5,-2);   // flèche
}

//-----------------------------------------------------------------------------
void GribPlot::drawWindArrowWithBarbs(
			QPainter &pnt,
			int i, int j, double vx, double vy,
			bool south,
			QColor arrowColor
	)
{
    double  vkn = sqrt(vx*vx+vy*vy)*3.6/1.852;
    double ang = atan2(vy, -vx);
    double si=sin(ang),  co=cos(ang);
    
    QPen pen( arrowColor);
    pen.setWidth(2);
    pnt.setPen(pen);
    pnt.setBrush(Qt::NoBrush);
    
    if (vkn < 1)
    {
        int r = 5;     // vent très faible, dessine un cercle
        pnt.drawEllipse(i-r,j-r,2*r,2*r);
    }
    else {
        // Flèche centrée sur l'origine
        int dec = -windBarbuleSize/2;
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+windBarbuleSize, 0);   // hampe
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, 2);    // flèche
        drawTransformedLine(pnt, si,co, i,j,  dec,0,  dec+5, -2);   // flèche
        
	 	int b1 = dec+windBarbuleSize -4;  // position de la 1ère barbule
		if (vkn >= 7.5  &&  vkn < 45 ) {
			b1 = dec+windBarbuleSize;  // position de la 1ère barbule si >= 10 noeuds
		}
        
        if (vkn < 7.5) {  // 5 ktn
            drawPetiteBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 12.5) { // 10 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
        }
        else if (vkn < 17.5) { // 15 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 22.5) { // 20 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 27.5) { // 25 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 32.5) { // 30 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 37.5) { // 35 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawPetiteBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 45) { // 40 ktn
            drawGrandeBarbule(pnt,south, si,co, i,j, b1);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 55) { // 50 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
        }
        else if (vkn < 65) { // 60 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
        }
        else if (vkn < 75) { // 70 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
        }
        else if (vkn < 85) { // 80 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-8);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-12);
            drawGrandeBarbule(pnt,south, si,co, i,j, b1-16);
        }
        else { // > 90 ktn
            drawTriangle(pnt,south, si,co, i,j, b1-4);
            drawTriangle(pnt,south, si,co, i,j, b1-12);
        }
    }
}
//---------------------------------------------------------------
void GribPlot::drawPetiteBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, -5);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, 5);
}
//---------------------------------------------------------------
void GribPlot::drawGrandeBarbule(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
}
//---------------------------------------------------------------
void GribPlot::drawTriangle(QPainter &pnt, bool south,
                    double si, double co, int di, int dj, int b)
{
    if (south) {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,-10);
    }
    else {
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
        drawTransformedLine(pnt, si,co, di,dj,  b+8,0,  b+4,10);
    }
}
