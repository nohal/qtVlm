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

#include <iostream>
#include <cassert>

#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

#include "Terrain.h"
#include "settings.h"

//---------------------------------------------------------
// Constructeur
//---------------------------------------------------------
Terrain::Terrain(myCentralWidget *parent, Projection *proj_) : QGraphicsWidget()
{
    this->parent=parent;
    proj = proj_;
    connect(proj,SIGNAL(projectionUpdated()),this,SLOT(redrawAll()));
    connect(parent,SIGNAL(redrawAll()),this,SLOT(redrawAll()));
    connect(parent,SIGNAL(redrawGrib()),this,SLOT(redrawGrib()));
    connect(this,SIGNAL(mousePress(QGraphicsSceneMouseEvent*)),parent,SLOT(slot_mousePress(QGraphicsSceneMouseEvent*)));
    connect(this,SIGNAL(mouseRelease(QGraphicsSceneMouseEvent*)),parent,SLOT(slot_mouseRelease(QGraphicsSceneMouseEvent*)));

    setZValue(Z_VALUE_TERRE);
    setData(0,TERRE_WTYPE);

    width=50;
    height=50;
    setPos(0,0);
    //qWarning() << "Terre is at " << x() << "," << y() << ", size: " << size().width() << "," << size().height();

    quality = 0;

    //---------------------------------------------------------------------
    showCountriesBorders  = Settings::getSetting("showCountriesBorders", true).toBool();

    showRivers   = Settings::getSetting("showRivers", false).toBool();
    showCitiesNamesLevel = Settings::getSetting("showCitiesNamesLevel", 0).toInt();
    showCountriesNames = Settings::getSetting("showCountriesNames", false).toBool();
    showWindColorMap  = Settings::getSetting("showWindColorMap", true).toBool();

    colorMapSmooth = Settings::getSetting("colorMapSmooth", true).toBool();
    showWindArrows  = Settings::getSetting("showWindArrows", true).toBool();
    showBarbules = Settings::getSetting("showBarbules", true).toBool();
    //----------------------------------------------------------------------------

    imgEarth = NULL;
    imgSea = NULL;
    imgWind  = NULL;
    imgAll   = NULL;
    isEarthMapValid = false;
    isWindMapValid  = false;
    mustRedraw = true;
    isWaiting=false;

    gshhsReader = NULL;
    gisReader = new GisReader();
    assert(gisReader);

    setPalette(QPalette(backgroundColor));

    updateGraphicsParameters();    
}

//-------------------------------------------
void Terrain::updateGraphicsParameters()
{
    backgroundColor  = Settings::getSetting("backgroundColor", QColor(0,0,45)).value<QColor>();
    seaColor  = Settings::getSetting("seaColor", QColor(50,50,150)).value<QColor>();
    landColor = Settings::getSetting("landColor", QColor(200,200,120)).value<QColor>();
    landColor.setAlpha(Settings::getSetting("landOpacity","180").toInt());
    tranparent=QColor(0,0,0,0);

    seaBordersPen.setColor(Settings::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>());
    seaBordersPen.setWidthF(Settings::getSetting("seaBordersLineWidth", 1.8).toDouble());

    boundariesPen.setColor(Settings::getSetting("boundariesLineColor", QColor(40,40,40)).value<QColor>());
    boundariesPen.setWidthF(Settings::getSetting("boundariesLineWidth", 1.4).toDouble());

    riversPen.setColor(Settings::getSetting("riversLineColor", QColor(50,50,150)).value<QColor>());
    riversPen.setWidthF(Settings::getSetting("riversLineWidth", 1.0).toDouble());

    int v = 180;
    selectColor     = QColor(v,v,v);


    isEarthMapValid = false;
    mustRedraw = true;
    indicateWaitingMap();
}

//---------------------------------------------------------
// Cartes GSHHS
//---------------------------------------------------------
void Terrain::setGSHHS_map(GshhsReader *map)
{
    gshhsReader = map;
    isEarthMapValid = false;
}

//-------------------------------------------------------
void Terrain::draw_GSHHSandGRIB()
{
    QCursor oldcursor = cursor();
    setCursor(Qt::WaitCursor);
    if (imgAll != NULL) {
        delete imgAll;
        imgAll = NULL;
    }
    imgAll = new QPixmap(width,height);
    assert(imgAll);
    QPainter pnt(imgAll);
    pnt.setRenderHint(QPainter::Antialiasing, true);

    tranparent=Qt::transparent;

    //===================================================
    // Dessin du fond de carte et des données GRIB
    //===================================================
    if (!isEarthMapValid)
    {
        if (imgEarth != NULL) {
            delete imgEarth;
            imgEarth = NULL;
        }
        if (imgSea != NULL) {
            delete imgSea;
            imgSea = NULL;
        }

        imgEarth = new QImage(width,height,QImage::Format_ARGB32_Premultiplied);
        assert(imgEarth);
        imgEarth->fill(Qt::transparent);
        imgSea = new QImage(width,height,QImage::Format_ARGB32_Premultiplied);
        assert(imgSea);

        if (gshhsReader != NULL)
        {
            QPainter pnt0(imgSea);
            QPainter pnt1(imgEarth);
            pnt0.setRenderHint(QPainter::Antialiasing, true);
            pnt1.setRenderHint(QPainter::Antialiasing, true);
            pnt1.setCompositionMode(QPainter::CompositionMode_Source);
            gshhsReader->drawBackground(pnt0, proj, seaColor, backgroundColor);
            gshhsReader->drawContinents(pnt1, proj, tranparent, landColor);
        }
    }
    pnt.drawImage(0,0, *imgSea);

    //===================================================
    // Dessin des données GRIB
    //===================================================

    Grib * grib=parent->getGrib();

    if(grib)
        grib->draw_WIND_Color(pnt, proj, colorMapSmooth,showWindColorMap,showWindArrows,showBarbules);

    //===================================================
    // Dessin des bordures et frontières
    //===================================================

    if (gshhsReader != NULL)
    {
        pnt.setPen(seaBordersPen);
        gshhsReader->drawSeaBorders(pnt, proj);

        if (showCountriesBorders) {
            pnt.setPen(boundariesPen);
            gshhsReader->drawBoundaries(pnt, proj);
        }
        if (showRivers) {
            pnt.setPen(riversPen);
            gshhsReader->drawRivers(pnt, proj);
        }
    }

    if (showCountriesNames) {

        gisReader->drawCountriesNames(pnt, proj);
    }
    if (showCitiesNamesLevel > 0) {
        gisReader->drawCitiesNames(pnt, proj, showCitiesNamesLevel);
    }
    //===================================================


    //===================================================

    pnt.drawImage(0,0, *imgEarth);
    /*int save=0;
    if(save==1) imgEarth->save("test.jpg","JPG",100);*/
    if(grib) grib->drawCartouche(pnt);
    setCursor(oldcursor);
}

//=========================================================
void Terrain::setDrawRivers(bool b) {
    if (showRivers != b) {
        showRivers = b;
        Settings::setSetting("showRivers", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawCountriesBorders(bool b) {
    if (showCountriesBorders != b) {
        showCountriesBorders = b;
        Settings::setSetting("showCountriesBorders", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------

//-------------------------------------------------------
void Terrain::setCountriesNames(bool b) {
    if (showCountriesNames != b) {
        showCountriesNames = b;
        Settings::setSetting("showCountriesNames", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setCitiesNamesLevel  (int level) {
    if (showCitiesNamesLevel != level) {
        showCitiesNamesLevel = level;
        Settings::setSetting("showCitiesNamesLevel", level);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::slot_setMapQuality(int q) {
    if (quality != q) {
        if (gshhsReader == NULL)
            return;
        quality = q;        
        gshhsReader->setUserPreferredQuality(q);
        isEarthMapValid = false;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::slot_setDrawWindColors (bool b) {
    if (showWindColorMap != b) {
        showWindColorMap = b;
        Settings::setSetting("showWindColorMap", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setColorMapSmooth (bool b) {
    if (colorMapSmooth != b) {
        colorMapSmooth = b;
        Settings::setSetting("colorMapSmooth", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setDrawWindArrows (bool b) {
    if (showWindArrows != b) {
        showWindArrows = b;
        Settings::setSetting("showWindArrows", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setBarbules (bool b) {
    if (showBarbules != b) {
        showBarbules = b;
        Settings::setSetting("showBarbules", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

void Terrain::redrawAll()
{
    isEarthMapValid = false;
    isWindMapValid = false;
    indicateWaitingMap();
}

void Terrain::redrawGrib()
{
    isWindMapValid = false;
    indicateWaitingMap();
}


//---------------------------------------------------------




//---------------------------------------------------------
// Events
//---------------------------------------------------------


void Terrain::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{    
    emit showContextualMenu(event);
}

void Terrain::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::LeftButton || e->button()==Qt::MidButton )
    {
        emit mousePress(e);
    }
}

void Terrain::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::LeftButton)
    {
        emit mouseRelease(e);
    }
}

void Terrain::updateSize(int width, int height)
{
    prepareGeometryChange();
    isEarthMapValid = false;
    isWindMapValid = false;
    this->width=width;
    this->height=height;
}

QRectF Terrain::boundingRect() const
{
    return QRectF(0,0,width,height);
}

QPainterPath Terrain::shape() const
 {
     QPainterPath path;
     path.addRect(0,0,width,height);
     return path;
 }

//---------------------------------------------------------
// paintEvent
//---------------------------------------------------------
void Terrain::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    pnt->drawPixmap(0,0, *imgAll);
}

void Terrain::indicateWaitingMap()
{
    if(isWaiting) return;
    isWaiting=true;
    if(imgAll!=NULL)
    {
        QPainter pnt_1(imgAll);
        pnt_1.setRenderHint(QPainter::Antialiasing, true);
        QFont fontWait("Helvetica", 12, QFont::Bold, true);
        QFontMetrics fmet(fontWait);
        pnt_1.setPen(QColor(Qt::white));
        int r = 80;
        QColor transp = QColor(r,r,r, 80);
        pnt_1.setFont(fontWait);
        pnt_1.setBrush(transp);
        QString txt = tr("Calculs en cours...");
        QRect rect = fmet.boundingRect(txt);
        rect.moveTo(20,20);
        pnt_1.drawRect(rect);
        pnt_1.drawText(rect, Qt::AlignHCenter|Qt::AlignVCenter , txt);
        updateRoutine();
   }
   if (mustRedraw  ||  !isEarthMapValid  || !isWindMapValid)
   {
        draw_GSHHSandGRIB();

        isEarthMapValid = true;
        isWindMapValid = true;
        mustRedraw = false;
    }
    updateRoutine();
    isWaiting=false;
}
void Terrain::updateRoutine()
{
    update();
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}
