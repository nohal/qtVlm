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

#include "GribPlot.h"
#include <QMessageBox>
#include <QObject>
 
//----------------------------------------------------
GribPlot::GribPlot()
{
    gribReader = NULL;
    
    mapColorTransp = 210;
    isobarsStep = 4;
    windArrowSpace = 30;      // distance mini entre flèches du vent (pixels)
    windArrowSize = 14;       // longueur des flèches
    windBarbuleSpace = 40;    // distance mini entre flèches du vent (pixels)
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
    // Color scale for rain in mm/h
    rainColor[ 0].setRgba(qRgba(255,255,255,  mapColorTransp));
    rainColor[ 1].setRgba(qRgba(200,255,255,  mapColorTransp));
    rainColor[ 2].setRgba(qRgba(150,255,255,  mapColorTransp));
    rainColor[ 3].setRgba(qRgba(100,200,255,  mapColorTransp));
    rainColor[ 4].setRgba(qRgba( 50,200,255,  mapColorTransp));
    rainColor[ 5].setRgba(qRgba(  0,150,255,  mapColorTransp));
    rainColor[ 6].setRgba(qRgba(  0,100,255,  mapColorTransp));
    rainColor[ 7].setRgba(qRgba(  0, 50,255,  mapColorTransp));
    rainColor[ 8].setRgba(qRgba( 50,  0,255,  mapColorTransp));
    rainColor[ 9].setRgba(qRgba(100,  0,255,  mapColorTransp));
    rainColor[10].setRgba(qRgba(150,  0,255,  mapColorTransp));
    rainColor[11].setRgba(qRgba(200,  0,255,  mapColorTransp));
    rainColor[12].setRgba(qRgba(250,  0,255,  mapColorTransp));
    rainColor[13].setRgba(qRgba(200,  0,200,  mapColorTransp));
    rainColor[14].setRgba(qRgba(150,  0,150,  mapColorTransp));
    rainColor[15].setRgba(qRgba(100,  0,100,  mapColorTransp));
    rainColor[16].setRgba(qRgba( 50,  0,50,  mapColorTransp));
}
//----------------------------------------------------
GribPlot::~GribPlot() {
    Util::cleanListPointers(listIsobars);
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getRainColor(float mm, bool smooth) {
    QRgb rgb = 0;
    int  ind;
    float v = mm/5;  
    float indf = cbrt(337.5*v);        // TODO better color map!!!!
    
    ind = (int) floor(Util::inRange(indf, 0.0f, 15.0f));
    
    if (smooth && ind<16) {
        // Interpolation de couleur
        QColor c1 = rainColor[ind];
        QColor c2 = rainColor[ind+1];
        float dcol = indf-ind;
        rgb = qRgba(
            (int)( (float) c1.red()  *(1.0-dcol) + dcol*c2.red()   +0.5),
            (int)( (float) c1.green()*(1.0-dcol) + dcol*c2.green() +0.5),
            (int)( (float) c1.blue() *(1.0-dcol) + dcol*c2.blue()  +0.5),
            mapColorTransp
            );
    }
    else {
    
    ind = (int) (indf + 0.5);
    ind = Util::inRange(ind, 0, 15);
        rgb = rainColor[ind].rgba();
    }
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getCloudColor(float v, bool smooth) {
    QRgb rgb = 0;
    if (!smooth) {
        v = 10.0*floor(v/10.0);
    }
    int r = 255 - (int)(1.6*v);
    int g = 255 - (int)(1.6*v);
    int b = 255 - (int)(2.0*v);
    rgb = qRgba(r,g,b,  mapColorTransp);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getHumidColor(float v, bool smooth) {
    QRgb rgb = 0;
    if (!smooth) {
        v = 10.0*floor(v/10.0);
    }
    
    //v = v*v*v/10000;
    v = v*v/100;
    
    int r = 255 - (int)(2.4*v);
    int g = 255 - (int)(2.4*v);
    int b = 255 - (int)(1.2*v);
    rgb = qRgba(r,g,b,  mapColorTransp);
    return rgb;
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getTemperatureColor(float v, bool smooth)
{
	// Même échelle colorée que pour le vent
	float x = v-273.15;
	float t0 = -30;  // valeur mini de l'échelle
	float t1 =  40;  // valeur maxi de l'échelle
	float b0 = 0;    // min beauforts
	float b1 = 12;   // max beauforts
	float eqbeauf = b0 + (x-t0)*(b1-b0)/(t1-t0);
	if (eqbeauf < 0)
		eqbeauf = 0;
	else if (eqbeauf > 12)
		eqbeauf = 12;
	return getWindColor(Util::BeaufortToKmh_F(eqbeauf), smooth);
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getPressureColor(float v, bool smooth)
{
	// Même échelle colorée que pour le vent
	float x = v;
	float t0 = 960;  // valeur mini de l'échelle
	float t1 = 1050;  // valeur maxi de l'échelle
	float b0 = 0;    // min beauforts
	float b1 = 12;   // max beauforts
	float eqbeauf = b0 + (x-t0)*(b1-b0)/(t1-t0);
	if (eqbeauf < 0)
		eqbeauf = 0;
	else if (eqbeauf > 12)
		eqbeauf = 12;
	return getWindColor(Util::BeaufortToKmh_F(eqbeauf), smooth);
}
//--------------------------------------------------------------------------
QRgb  GribPlot::getWindColor(float v, bool smooth) {
    QRgb rgb = 0;
    int beauf;
    if (! smooth) {
        beauf = Util::kmhToBeaufort(v);
        rgb = windColor[beauf].rgba();
    }
    else {
        // Interpolation de couleur
        float fbeauf = Util::kmhToBeaufort_F(v);
        QColor c1 = windColor[ (int) fbeauf ];
        QColor c2 = windColor[ (int) fbeauf +1 ];
        float dcol = fbeauf-floor(fbeauf);
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
    if (gribReader != NULL) {
        listDates.clear();
        gribReader->openFile(qPrintable(fileName));
    }
    else {        
        gribReader = new GribReader(qPrintable(fileName));
    }
    
    listDates = gribReader->getListDates();
    setCurrentDate ( listDates.size()>0 ? *(listDates.begin()) : 0);

}

//----------------------------------------------------
void GribPlot::setCurrentDate(time_t t)
{
    currentDate = t;
    initIsobars();
}
//----------------------------------------------------
void GribPlot::initIsobars()
{
    if (gribReader == NULL)
        return;
    GribRecord *rec = gribReader->getGribRecord(GRB_PRESS_MSL, currentDate);
    if (rec == NULL)
        return;
    
    Util::cleanListPointers(listIsobars);    
    if (listIsobars.size() == 0) {
        Isobar *isob;
        for (float press=840; press<1120; press += isobarsStep)
        {
            isob = new Isobar(press*100, rec);
            listIsobars.push_back(isob);
        }
    }
}

//==========================================================================
GribPointInfo  GribPlot::getGribPointInfo(float x, float y)
{
    GribPointInfo pinfo(x, y, currentDate);
    GribRecord *rec;
    if (gribReader != NULL) {
        if ( (rec = gribReader->getGribRecord(GRB_WIND_VX, currentDate)) != NULL)
            pinfo.vx = rec->getInterpolatedValue(x,y);
        if ( (rec = gribReader->getGribRecord(GRB_WIND_VY, currentDate)) != NULL)
            pinfo.vy = rec->getInterpolatedValue(x,y);
        if ( (rec = gribReader->getGribRecord(GRB_PRESS_MSL, currentDate)) != NULL)
            pinfo.pressure = rec->getInterpolatedValue(x,y);
        if ( (rec = gribReader->getGribRecord(GRB_TEMP, currentDate)) != NULL)
            pinfo.temp = rec->getInterpolatedValue(x,y);
        if ( (rec = gribReader->getGribRecord(GRB_PRECIP_TOT, currentDate)) != NULL)
        {
            float pluie = rec->getInterpolatedValue(x,y);
            if (pluie != GRIB_NOTDEF) {
                int duree = rec->getPeriodP2()-rec->getPeriodP1();
                if (duree<=0)
                        duree=1; // TODO !!!!!!!!!
                if (duree > 0)
                    pinfo.rain = pluie / duree;
            }
        }
        if ( (rec = gribReader->getGribRecord(GRB_CLOUD_TOT, currentDate)) != NULL)
            pinfo.cloud = rec->getInterpolatedValue(x,y);
        if ( (rec = gribReader->getGribRecord(GRB_HUMID_REL, currentDate)) != NULL)
            pinfo.humid = rec->getInterpolatedValue(x,y);
    }
    return pinfo;
}

//==========================================================================
// Rectangle translucide sur la zone couverte par les données
void GribPlot::show_GRIB_CoverZone(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    
    float x0,y0, x1,y1;
    int i, j, k,l;
    if (gribReader->getGribExtension(&x0,&y0, &x1,&y1))
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
// Dates de la prévision courante
void GribPlot::draw_ForecastDates(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    
    if (gribReader->getNumberOfDates() > 0)
    {
        QString  tref = "Ref: "+Util::formatDateTimeLong(gribReader->getRefDate());
        QString  tval = "Val: "+Util::formatDateTimeLong(this->getCurrentDate());
        int dx = 0;
        int dy = proj->getH() - 28;
        pnt.setPen(QColor(255,255,255,100));
        pnt.setBrush(QColor(255,255,255,100));
        pnt.drawRect(dx,dy,186,28);

        QFont font("TypeWriter", 11, QFont::Normal, false);
        font.setStyleHint(QFont::TypeWriter);
        font.setStretch(QFont::Condensed);
        pnt.setFont(font);
        pnt.setPen(QColor(30,30,30,255));
        pnt.drawText(dx+3,dy+12, tref);
        pnt.drawText(dx+3,dy+27, tval);
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
// Carte de couleurs des précipitations
//--------------------------------------------------------------------------
void GribPlot::draw_RAIN_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (gribReader == NULL)
        return;
    GribRecord *rec;
    rec = gribReader->getGribRecord(GRB_PRECIP_TOT, currentDate);
    if (rec == NULL)
        return;
        
    
    int i, j;
    float x, y, v;
    int W = proj->getW();
    int H = proj->getH();

    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));

    // Zones colorées selon la force
    for (i=0; i<W; i++)
    {
        for (j=0; j<H; j++)
        {
            proj->screen2map(i,j, &x, &y);
            
            if (! rec->isXInMap(x))
                x += 360.0;    // tour complet ?
                
            if (rec->isPointInMap(x, y))
            {
                v = rec->getInterpolatedValue(x, y);
                if (v != GRIB_NOTDEF)
                {
                    int duree = rec->getPeriodP2()-rec->getPeriodP1();
                    if (duree<=0)
                            duree=1; // TODO !!!!!!!!!
                    if (duree > 0) {
                        v = v / duree;
                        rgb = getRainColor(v, smooth);
                        image->setPixel(i,j, rgb);
                    }
                }
            }
        }
    }

    pnt.drawImage(0,0,*image);
    delete image;
}

//--------------------------------------------------------------------------
// Carte de couleurs de la nébulosité
//--------------------------------------------------------------------------
void GribPlot::draw_CLOUD_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (gribReader == NULL)
        return;
    GribRecord *rec = gribReader->getGribRecord(GRB_CLOUD_TOT, currentDate);
    if (rec == NULL)
        return;
    
    int i, j;
    float x, y, v;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W; i++) {
        for (j=0; j<H; j++) {
            proj->screen2map(i,j, &x, &y);
            if (! rec->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (rec->isPointInMap(x, y)) {
                v = rec->getInterpolatedValue(x, y);
                if (v != GRIB_NOTDEF) {
                    rgb = getCloudColor(v, smooth);
                    image->setPixel(i,j, rgb);
                }
            }
        }
    }
    pnt.drawImage(0,0,*image);
    delete image;
}
//--------------------------------------------------------------------------
// Carte de couleurs de l'humidité relative
//--------------------------------------------------------------------------
void GribPlot::draw_HUMID_Color(QPainter &pnt, const Projection *proj, bool smooth)
{
    if (gribReader == NULL)
        return;
    GribRecord *rec = gribReader->getGribRecord(GRB_HUMID_REL, currentDate);
    if (rec == NULL)
        return;
    
    int i, j;
    float x, y, v;
    int W = proj->getW();
    int H = proj->getH();
    QRgb   rgb;
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));
    for (i=0; i<W; i++) {
        for (j=0; j<H; j++) {
            proj->screen2map(i,j, &x, &y);
            if (! rec->isXInMap(x))
                x += 360.0;    // tour complet ?
            if (rec->isPointInMap(x, y)) {
                v = rec->getInterpolatedValue(x, y);
                if (v != GRIB_NOTDEF) {
                    rgb = getHumidColor(v, smooth);
                    image->setPixel(i,j, rgb);
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
    recx = gribReader->getGribRecord(GRB_WIND_VX, currentDate);
    recy = gribReader->getGribRecord(GRB_WIND_VY, currentDate);
    if (recx == NULL || recy == NULL)
        return;
        
    
    int i, j;
    float x, y, vx, vy, v;
    int W = proj->getW();
    int H = proj->getH();
    
    QImage *image = new QImage(W,H,QImage::Format_ARGB32);
    image->fill( qRgba(0,0,0,0));

    // Zones colorées selon la force
    for (i=0; i<W; i++) {
        for (j=0; j<H; j++)
        {
            proj->screen2map(i,j, &x, &y);
            
            if (! recx->isXInMap(x))
                x += 360.0;    // tour complet ?
                
            if (recx->isPointInMap(x, y))
            {
                vx = recx->getInterpolatedValue(x, y);
                vy = recy->getInterpolatedValue(x, y);

                if (vx != GRIB_NOTDEF && vx != GRIB_NOTDEF)
                {
                    v = sqrt(vx*vx+vy*vy)*3.6;
                    image->setPixel(i,j, getWindColor(v, smooth));
                }
            }
        }
    }
    pnt.drawImage(0,0,*image);
    delete image;
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

    GribRecord *recx = gribReader->getGribRecord(GRB_WIND_VX, currentDate);
    GribRecord *recy = gribReader->getGribRecord(GRB_WIND_VY, currentDate);
    if (recx == NULL || recy == NULL)
        return;        
    int i, j;
    float x, y, vx, vy;
    int W = proj->getW();
    int H = proj->getH();
    int space =  barbules ? windBarbuleSpace : windArrowSpace;
    
    for (i=0; i<W; i+=space)
    {
        for (j=0; j<H; j+=space)
        {
            proj->screen2map(i,j, &x,&y);
            if (! recx->isXInMap(x))
                x += 360.0;   // tour du monde ?
            
            if (recx->isPointInMap(x,y)) {
                vx = recx->getInterpolatedValue(x, y);
                vy = recy->getInterpolatedValue(x, y);
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

//-----------------------------------------------------------------------------
void GribPlot::drawTransformedLine( QPainter &pnt,
        float si, float co,int di, int dj, int i,int j, int k,int l)
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
void GribPlot::drawWindArrow(QPainter &pnt, int i, int j, float vx, float vy)
{
    float ang = atan2(vy, -vx);
    float si=sin(ang),  co=cos(ang);
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
			int i, int j, float vx, float vy,
			bool south,
			QColor arrowColor
	)
{
    float  vkn = sqrt(vx*vx+vy*vy)*3.6/1.852;
    float ang = atan2(vy, -vx);
    float si=sin(ang),  co=cos(ang);
    
    QPen pen( arrowColor);
    pen.setWidth(2);
    pnt.setPen(pen);
    pnt.setBrush(Qt::transparent);
    
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
                    float si, float co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, -5);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+2, 5);
}
//---------------------------------------------------------------
void GribPlot::drawGrandeBarbule(QPainter &pnt, bool south,
                    float si, float co, int di, int dj, int b)
{
    if (south)
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,-10);
    else
        drawTransformedLine(pnt, si,co, di,dj,  b,0,  b+4,10);
}
//---------------------------------------------------------------
void GribPlot::drawTriangle(QPainter &pnt, bool south,
                    float si, float co, int di, int dj, int b)
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


//==========================================================================
// Pression
//==========================================================================
void GribPlot::draw_PRESSURE_Isobars(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    GribRecord *rec = gribReader->getGribRecord(GRB_PRESS_MSL, currentDate);
    if (rec == NULL)
        return;    
    
    std::list<Isobar *>::iterator it;
    for(it=listIsobars.begin(); it!=listIsobars.end(); it++)
    {
        (*it)->drawIsobar(pnt, proj);
    }
}
//--------------------------------------------------------------------------
void GribPlot::setIsobarsStep(int step)
{
    isobarsStep = step;
    initIsobars();
}
//--------------------------------------------------------------------------
void GribPlot::draw_PRESSURE_IsobarsLabels(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    GribRecord *rec = gribReader->getGribRecord(GRB_PRESS_MSL, currentDate);
    if (rec == NULL)
        return;
    
    std::list<Isobar *>::iterator it;
    
    int nbseg = 0;
    for(it=listIsobars.begin(); it!=listIsobars.end(); it++)
    {
        nbseg += (*it)->getNbSegments();
    }

    int nbpix, density, first;
    nbpix = proj->getW()*proj->getH();
    if (nbpix == 0)
        return;
    
    float r = (float)nbseg/nbpix *1000;
    
    float dens = 10;
    
    density =  (int) (r*dens +0.5);
    if (density < 20)
        density = 20;

    first = 0;
//printf("nbpix=%d nbseg=%d density=%d\n", nbpix,nbseg,density);
    
    for(it=listIsobars.begin(); it!=listIsobars.end(); it++)
    {
        first += 20;
        (*it)->drawIsobarLabels(pnt, proj, density, first);
    }
}

//--------------------------------------------------------------------------
void GribPlot::draw_PRESSURE_MinMax(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    GribRecord *rec = gribReader->getGribRecord(GRB_PRESS_MSL, currentDate);
    if (rec == NULL)
        return;    
    
    int i, j, W, H, pi,pj;
    float x, y, v;
         
    QFont fontPressureMinMax("Times", 18, QFont::Bold, true);
    QFontMetrics fmet(fontPressureMinMax);
    pnt.setPen(QColor(0,0,0));
    pnt.setFont(fontPressureMinMax);
    W = rec->getNi();
    H = rec->getNj();

    for (j=1; j<H-1; j++) {     // !!!! 1 to end-1
        for (i=1; i<W-1; i++) {
            v = rec->getValue( i, j );
            if ( v < 101200
                   && v < rec->getValue( i-1, j-1 )  // Minima local ?
                   && v < rec->getValue( i-1, j   )
                   && v < rec->getValue( i-1, j+1 )
                   && v < rec->getValue( i  , j-1 )
                   && v < rec->getValue( i  , j+1 )
                   && v < rec->getValue( i+1, j-1 )
                   && v < rec->getValue( i+1, j   )
                   && v < rec->getValue( i+1, j+1 )
            ) {
                x = rec->getX(i);
                y = rec->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('L')/2, pj+fmet.ascent()/2, "L");
            }
            if ( v > 101200
                   && v >= rec->getValue( i-1, j-1 )  // Maxima local ?
                   && v >= rec->getValue( i-1, j   )
                   && v >= rec->getValue( i-1, j+1 )
                   && v >= rec->getValue( i  , j-1 )
                   && v >= rec->getValue( i  , j+1 )
                   && v >= rec->getValue( i+1, j-1 )
                   && v >= rec->getValue( i+1, j   )
                   && v >= rec->getValue( i+1, j+1 )
            ) {
                x = rec->getX(i);
                y = rec->getY(j);
                proj->map2screen(x,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
                proj->map2screen(x-360.0,y, &pi, &pj);
                pnt.drawText(pi-fmet.width('H')/2, pj+fmet.ascent()/2, "H");
            }
        }
    }
}


//--------------------------------------------------------------------------
void GribPlot::draw_TEMPERATURE_Labels(QPainter &pnt, const Projection *proj)
{
    if (gribReader == NULL) {
        return;
    }
    GribRecord *rec = gribReader->getGribRecord(GRB_TEMP, currentDate);
    if (rec == NULL)
        return;    
    
    QFont fontTemperatureLabels("Times", 9, QFont::Bold, true);
    QFontMetrics fmet(fontTemperatureLabels);
    pnt.setFont(fontTemperatureLabels);
    pnt.setPen(QColor(0,0,0));
    
    float x, y, v;
    int i, j, dimin, djmin;
    dimin = 50;
    djmin = 30;

    for (j=0; j<proj->getH(); j+= djmin) {
        for (i=0; i<proj->getW(); i+= dimin) {
            proj->screen2map(i,j, &x,&y);
                
            v = rec->getInterpolatedValue(x, y);
            if (v!= GRIB_NOTDEF) {
                QString strtemp = Util::formatTemperature_short(v);
                pnt.drawText(i-fmet.width("XXX")/2, j+fmet.ascent()/2, strtemp);
            }

        }
    }
}



