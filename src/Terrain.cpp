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
#include <QGraphicsScene>

#include "Terrain.h"
#include "settings.h"
#include "GisReader.h"
#include "Projection.h"
#include "mycentralwidget.h"
#include "GshhsReader.h"
#include "loadImg.h"
#include "Orthodromie.h"
#include <QTimer>
#include "MyView.h"
#include "ToolBar.h"
#include "MapDataDrawer.h"
#include "DataManager.h"

//---------------------------------------------------------
// Constructeur
//---------------------------------------------------------
Terrain::Terrain(myCentralWidget *centralWidget, Projection *proj_) : QGraphicsWidget()
{
    toBeRestarted=false;
    this->centralWidget=centralWidget;
    proj = proj_;
    this->routageGrib=NULL;
    connect(proj,SIGNAL(projectionUpdated()),this,SLOT(redrawAll()));
    connect(centralWidget,SIGNAL(redrawAll()),this,SLOT(redrawAll()));
    connect(centralWidget,SIGNAL(redrawGrib()),this,SLOT(redrawGrib()));
    connect(this,SIGNAL(mousePress(QGraphicsSceneMouseEvent*)),centralWidget,SLOT(slot_mousePress(QGraphicsSceneMouseEvent*)));
    connect(this,SIGNAL(mouseRelease(QGraphicsSceneMouseEvent*)),centralWidget,SLOT(slot_mouseRelease(QGraphicsSceneMouseEvent*)));
    timerUpdated=new QTimer(this);
    timerUpdated->setSingleShot(true);
    timerUpdated->setInterval(200);
    connect(timerUpdated,SIGNAL(timeout()),this,SIGNAL(terrainUpdated()));
    setZValue(Z_VALUE_TERRE);
    setData(0,TERRE_WTYPE);

    width=50;
    height=50;
    setPos(0,0);
    //qWarning() << "Terre is at " << x() << "," << y() << ", size: " << size().width() << "," << size().height();

    //---------------------------------------------------------------------
    showCountriesBorders  = Settings::getSetting("showCountriesBorders", true).toBool();

    showRivers   = Settings::getSetting("showRivers", false).toBool();
    showCitiesNamesLevel = Settings::getSetting("showCitiesNamesLevel", 0).toInt();
    showCountriesNames = Settings::getSetting("showCountriesNames", false).toBool();
    showWindColorMap  = Settings::getSetting("showWindColorMap", true).toBool();

    colorMapSmooth = Settings::getSetting("colorMapSmooth", true).toBool();
    showWindArrows  = Settings::getSetting("showWindArrows", true).toBool();
    showBarbules = Settings::getSetting("showBarbules", true).toBool();

    showIsobars  = Settings::getSetting("showIsobars", true).toBool();
    showIsobarsLabels = Settings::getSetting("showIsobarsLabels", false).toBool();
    isobarsStep = Settings::getSetting("isobarsStep", 2).toDouble();
    showPressureMinMax = Settings::getSetting("showPressureMinMax", false).toBool();

    showIsotherms0  = Settings::getSetting("showIsotherms0", false).toBool();
    showIsotherms0Labels  = Settings::getSetting("showIsotherms0Labels", false).toBool();
    isotherms0Step = Settings::getSetting("isotherms0Step", 50).toDouble();

    colorMapMode = Settings::getSetting("colorMapMode", MapDataDrawer::drawWind).toInt();

    showTemperatureLabels = Settings::getSetting("showTemperatureLabels", false).toBool();
    //showGribGrid = Settings::getSetting("showGribGrid", false).toBool();
    //----------------------------------------------------------------------------

    imgEarth = NULL;
    imgWind  = NULL;
    imgAll   = NULL;
    isEarthMapValid = false;
    isWindMapValid  = false;
    mustRedraw = true;

    gshhsReader = NULL;
    gisReader = NULL;

    setPalette(QPalette(backgroundColor));
    int sX=Settings::getSetting("scalePosX",5).toInt();
    int sY=Settings::getSetting("scalePosY",height-5).toInt();
    scalePos=QPoint(sX,sY);
    updateGraphicsParameters();    
}

//-------------------------------------------
void Terrain::updateGraphicsParameters()
{
    backgroundColor  = Settings::getSetting("backgroundColor", QColor(0,0,45)).value<QColor>();
    seaColor  = Settings::getSetting("seaColor", QColor(50,50,150)).value<QColor>();
    landColor = Settings::getSetting("landColor", QColor(200,200,120)).value<QColor>();
    landColor.setAlpha(Settings::getSetting("landOpacity","180").toInt());
    transparentColor=QColor(0,0,0,0);

    seaBordersPen.setColor(Settings::getSetting("seaBordersLineColor", QColor(40,45,30)).value<QColor>());
    seaBordersPen.setWidthF(Settings::getSetting("seaBordersLineWidth", 1.8).toDouble());

    boundariesPen.setColor(Settings::getSetting("boundariesLineColor", QColor(40,40,40)).value<QColor>());
    boundariesPen.setWidthF(Settings::getSetting("boundariesLineWidth", 1.4).toDouble());

    riversPen.setColor(Settings::getSetting("riversLineColor", QColor(50,50,150)).value<QColor>());
    riversPen.setWidthF(Settings::getSetting("riversLineWidth", 1.0).toDouble());

    isobarsPen.setColor(Settings::getSetting("isobarsLineColor", QColor(80,80,80)).value<QColor>());
    isobarsPen.setWidthF(Settings::getSetting("isobarsLineWidth", 2.0).toDouble());

    isotherms0Pen.setColor(Settings::getSetting("isotherms0LineColor", QColor(200,120,100)).value<QColor>());
    isotherms0Pen.setWidthF(Settings::getSetting("isotherms0LineWidth", 1.6).toDouble());

    int v = 180;
    selectColor     = QColor(v,v,v);


    isEarthMapValid = false;
    mustRedraw = true;
    centralWidget->getScene()->setBackgroundBrush(seaColor);
    indicateWaitingMap();
}

//---------------------------------------------------------
// Cartes GSHHS
//---------------------------------------------------------
void Terrain::setGSHHS_map(GshhsReader *map)
{
    gshhsReader = map;
    /* new gshhs => reload gis */
    if(gisReader)
    {
        delete gisReader;
        gisReader=NULL;
    }
    if(!gshhsReader)
        return;
    gisReader=new GisReader();
    isEarthMapValid = false;
    redrawAll();
}

//-------------------------------------------------------
void Terrain::draw_GSHHSandGRIB()
{
//    if(proj->getFrozen()) //routage
//    {
//        QPainter pnt(imgAll);
//        gshhsReader->drawSeaBorders(pnt, proj);
//        return;
//    }
    if(centralWidget->getKap())
        centralWidget->getKap()->slot_updateProjection();
    QCursor oldcursor = cursor();
    setCursor(Qt::WaitCursor);
    if (imgAll != NULL) {
        delete imgAll;
        imgAll = NULL;
    }
    imgAll = new QPixmap(width,height);
    assert(imgAll);
    imgAll->fill(Qt::transparent);
    QPainter pnt(imgAll);
    pnt.setRenderHint(QPainter::Antialiasing, true);
    pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);

    transparentColor=Qt::transparent;

    //===================================================
    // Dessin du fond de carte et des donnees GRIB
    //===================================================
    if (!isEarthMapValid || mustRedraw)
    {
        if (imgEarth != NULL) {
            delete imgEarth;
            imgEarth = NULL;
        }


#ifdef __TERRAIN_QIMAGE
        imgEarth = new QImage(width,height,QImage::Format_ARGB32_Premultiplied);
#else
        imgEarth= new QPixmap(width,height);
#endif
        assert(imgEarth);
        imgEarth->fill(Qt::transparent);

        if (gshhsReader != NULL)
        {
            QPainter pnt1(imgEarth);
            pnt1.setRenderHint(QPainter::Antialiasing, true);
            pnt1.setRenderHint(QPainter::SmoothPixmapTransform, true);
            pnt1.setCompositionMode(QPainter::CompositionMode_Source);
            gshhsReader->drawContinents(pnt1, proj, transparentColor, landColor);
        }
    }

    //===================================================
    // Dessin des donnees GRIB
    //===================================================

    if(centralWidget->get_dataManager()->isOk())
    {
        drawGrib(pnt);
        //imgAll->save("testGrib_terrain1.png");
        if(centralWidget->getKap()!=NULL /*&& centralWidget->getKap()->getDrawGribOverKap()*/)
        {
            QPolygon bordersXY;
            QPolygonF borders=centralWidget->getKap()->getBorders();
            for(int i=0;i<borders.count();++i)
            {
                int X,Y;
                proj->map2screen(borders.at(i).x(),borders.at(i).y(),&X,&Y);
                bordersXY.append(QPoint(X,Y));
            }
            QRectF br=bordersXY.boundingRect();
            if(!br.isNull() && !br.isEmpty())
                centralWidget->getKap()->setImgGribKap(imgAll->copy(br.toRect()));
            else
                centralWidget->getKap()->setImgGribKap(QPixmap(0,0));
        }
        if(routageGrib!=NULL)
        {
            mutex.lock();
            QPen penRoutage;
            penRoutage.setWidth(1);
            QList<vlmLine*> isochrones=routageGrib->getIsochrones();
            for(int i=isochrones.count()-1;i>0;--i)
            {
                QList<vlmPoint> * iso=isochrones.at(i)->getPoints();
                for(int p=0;p<iso->count()-1;++p)
                {
                    double x,y;
                    double windAverage=0;
                    QPolygonF poly;

                    vlmPoint ip=*(iso->at(p).origin);
                    windAverage+=ip.wind_speed;
                    proj->map2screenDouble(ip.lon,ip.lat,&x,&y);
                    poly.append(QPointF(x,y));

                    ip=iso->at(p);
                    windAverage+=ip.wind_speed;
                    proj->map2screenDouble(ip.lon,ip.lat,&x,&y);
                    poly.append(QPointF(x,y));

                    ip=iso->at(p+1);
                    windAverage+=ip.wind_speed;
                    proj->map2screenDouble(ip.lon,ip.lat,&x,&y);
                    poly.append(QPointF(x,y));

                    ip=*(iso->at(p+1).origin);
                    windAverage+=ip.wind_speed;
                    proj->map2screenDouble(ip.lon,ip.lat,&x,&y);
                    poly.append(QPointF(x,y));


                    vlmPoint O1=*(iso->at(p).origin);
                    vlmPoint O2=*(iso->at(p+1).origin);
                    QList<vlmPoint> * previousIso=isochrones.at(i-1)->getPoints();
                    int o1=previousIso->indexOf(O1,0);
                    int o2=previousIso->indexOf(O2,0);
                    while(o2>o1)
                    {
                        --o2;
                        ip=previousIso->at(o2);
                        windAverage+=ip.wind_speed;
                        proj->map2screenDouble(ip.lon,ip.lat,&x,&y);
                        poly.append(QPointF(x,y));
                    }

                    QColor color_r= MapDataDrawer::getWindColorStatic(windAverage/poly.count(),true);
                    color_r.setAlpha(255);
                    penRoutage.setColor(color_r);
                    penRoutage.setBrush(QBrush(color_r));
                    pnt.setPen(penRoutage);
                    pnt.setBrush(penRoutage.brush());
                    pnt.drawPolygon(poly,Qt::WindingFill);
                }
            }
            mutex.unlock();
        }
    }
    else if(centralWidget->getKap()!=NULL)
        centralWidget->getKap()->setImgGribKap(QPixmap(0,0));


    //imgAll->save("testGrib_terrain2.png");
#ifdef __TERRAIN_QIMAGE
    pnt.drawImage(0,0, *imgEarth);
#else
    pnt.drawPixmap(0,0, *imgEarth);
#endif
#if 0
    if(centralWidget->getFax()!=NULL)
    {
        QPixmap * fax=centralWidget->getFax();
//        double scale=Settings::getSetting("faxScale",0).toDouble();
        double faxLatN=65;
        double faxLatS=10;
        double faxLon=-101;

        double distFax=qAbs(faxLatN-faxLatS);

        double distProj=qAbs(proj->getYmax()-proj->getYmin());

        double newHeight=height*distFax/distProj;
        if(newHeight<2048)
        {
            QPixmap faxResized=fax->scaledToHeight(newHeight,Qt::SmoothTransformation);
            int xFax,yFax;
            proj->map2screen(faxLon,faxLatN,&xFax,&yFax);
            qWarning()<<proj->getYmax()<<proj->getYmin();
            qWarning()<<distFax<<distProj<<"fax height="<<fax->height()<<"new height="<<newHeight;
            qWarning()<<"xFax="<<xFax<<"yFax="<<yFax;
            pnt.setBackgroundMode(Qt::transparentMode);
            pnt.setCompositionMode(QPainter::CompositionMode_Source);
            pnt.drawPixmap(xFax,yFax,faxResized);
            pnt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
#endif
    //===================================================
    // Dessin des bordures et frontieres
    //===================================================

    if (gshhsReader != NULL)
    {

        if (showCountriesBorders) {
            pnt.setPen(boundariesPen);
            gshhsReader->drawBoundaries(pnt, proj);
        }
        if (showRivers) {
            pnt.setPen(riversPen);
            gshhsReader->drawRivers(pnt, proj);
        }
    }

    if (gisReader && showCountriesNames)
        gisReader->drawCountriesNames(pnt, proj);
    if (gisReader && showCitiesNamesLevel > 0)
        gisReader->drawCitiesNames(pnt, proj, showCitiesNamesLevel);
    //===================================================

    if (gshhsReader != NULL)
    {
        pnt.setPen(seaBordersPen);
        gshhsReader->drawSeaBorders(pnt, proj);
    }
    //===================================================

    /*int save=0;
    if(save==1) imgEarth->save("test.jpg","JPG",100);*/
    QString cartouche="";
    DataManager * dataManager=centralWidget->get_dataManager();
    if(dataManager) cartouche=dataManager->get_cartoucheData()+". ";
    if(this->gshhsReader)
        cartouche=cartouche+tr("Niveau de detail des cotes: ")+QString().setNum(this->gshhsReader->getQuality()+1);
    QFont fontbig("TypeWriter", 12, QFont::Bold, false);
    fontbig.setPointSizeF(12.0+Settings::getSetting("defaultFontSizeInc",0).toDouble());
    fontbig.setStyleHint(QFont::TypeWriter);
    fontbig.setStretch(QFont::Condensed);
    QColor   transpcolor(255,255,255,120);
    QColor   textcolor(20,20,20,255);
    pnt.setBrush(transpcolor);
    pnt.setFont(fontbig);
    pnt.setPen(transpcolor);
    QFontMetrics fm(fontbig);
    QSize Fsize=fm.size(Qt::TextSingleLine,cartouche);
    pnt.drawRect(3,3,Fsize.width()+2,Fsize.height());
    pnt.setPen(textcolor);

    pnt.drawText(5, 8+Fsize.height()/2, cartouche);// forecast validity date

    /*echelle*/
    double w=width/8.0;
    double lon1,lat1,lon2,lat2;
    proj->screen2map(scalePos.x(),scalePos.y(),&lon1,&lat1);
    proj->screen2map(w+scalePos.x(),scalePos.y(),&lon2,&lat2);
    Orthodromie oo(lon1,lat1,lon2,lat2);
    double distance=oo.getLoxoDistance();
    bool meters=false;
    bool centimeters=false;
    double distanceMeters=distance*1852.0;
    if(distanceMeters<1)
    {
        centimeters=true;
        distanceMeters=distanceMeters*100.0;
        if(distanceMeters>10)
        {
            distanceMeters=qRound(distanceMeters);
            distanceMeters=qRound(distanceMeters/10.0)*10;
        }
        else if(distanceMeters>1)
        {
            distanceMeters=qRound(distanceMeters);
        }
        distance=distanceMeters/(100*1852);
    }
    else if(distanceMeters<10)
    {
        meters=true;
        distanceMeters=qRound(distanceMeters);
        distance=distanceMeters/1852;
    }
    else if(distanceMeters<100)
    {
        meters=true;
        distanceMeters=qRound(distanceMeters);
        distanceMeters=qRound(distanceMeters/10.0)*10;
        distance=distanceMeters/1852;
    }
    else if(distanceMeters<1000)
    {
        meters=true;
        distanceMeters=qRound(distanceMeters);
        distanceMeters=qRound(distanceMeters/100.0)*100;
        distance=distanceMeters/1852;
    }
    else if(distance<10) distance=qRound(distance);
    else if(distance<25) distance=20;
    else if(distance<50) distance=25;
    else if(distance<75) distance=50;
    else if(distance<100) distance=75;
    else if(distance<150) distance=100;
    else if(distance<1100) distance=qRound(distance/100.0)*100;
    else distance=qRound(distance/1000.0)*1000;
    Util::getCoordFromDistanceLoxo(lat1,lon1,distance,90.0,&lat2,&lon2);
    int a,b;
    proj->map2screen(lon2,lat2,&a,&b);
    int sX=scalePos.x();
    int sY=scalePos.y();
    if(a<sX)
    {
        a += proj->getScale()*360.0;
    }
    pnt.setBackgroundMode(Qt::OpaqueMode);
    pnt.setBackground(QBrush(QColor(255,255,255,100)));
    QString scaleText;
    if(meters)
        scaleText=QString().setNum(distanceMeters)+tr("m");
    else if(centimeters)
        scaleText=QString().setNum(distanceMeters)+tr("cm");
    else
        scaleText=QString().setNum(distance)+tr("NM");
    if(proj->getScale()>3.99e8)
        scaleText+=" "+tr("(Max zoom reached)");
    QSize Ssize=fm.size(Qt::TextSingleLine,scaleText);
    int screenDist=qAbs(a-sX)+Ssize.width()+10;
    if(sX+screenDist>width)
        sX=width-screenDist;
    if(sY-(7+Ssize.height()+10)<0)
        sY=17+Ssize.height();
    screenDist-=(Ssize.width()+10);
    QPoint correctedScalePos(sX,sY);
    pnt.drawText(sX+screenDist,correctedScalePos.y()-7,scaleText);
    QPen p(Qt::black);
    p.setWidth(3);
    pnt.setPen(p);
    pnt.drawLine(correctedScalePos,QPoint(sX+screenDist,correctedScalePos.y()));
    pnt.drawLine(correctedScalePos,QPoint(correctedScalePos.x(),correctedScalePos.y()-4));
    pnt.drawLine(QPoint(sX+screenDist,correctedScalePos.y()),QPoint(sX+screenDist,correctedScalePos.y()-4));
    setCursor(oldcursor);
    daylight(&pnt,vlmPoint(0,0));
    centralWidget->getView()->resetTransform();
    centralWidget->getView()->hideViewPix();
    centralWidget->getScene()->setPinching(false);
}

void Terrain::drawGrib(QPainter &pnt)
{
    MapDataDrawer * mapDataDrawer=centralWidget->get_mapDataDrawer();
        //QTime t1 = QTime::currentTime();
        //qWarning() << "Grib mode: " << colorMapMode << " (grib=" << MapDataDrawer::drawWind << ")";
        // grib->draw_WIND_Color(pnt, proj, colorMapSmooth,showWindColorMap,showWindArrows,showBarbules);
        switch (colorMapMode)
        {
                case MapDataDrawer::drawWind :
                        windArrowsColor.setRgb(255, 255, 255);                        
                        mapDataDrawer->draw_WIND_Color(pnt, proj, colorMapSmooth,showWindArrows,showBarbules);
                        break;
                case MapDataDrawer::drawCurrent :
                        windArrowsColor.setRgb(255, 255, 255);
                        mapDataDrawer->draw_CURRENT_Color(pnt, proj, colorMapSmooth,showWindArrows,false);
                        break;
                case MapDataDrawer::drawRain :
                        windArrowsColor.setRgb(140, 120, 100);
                        mapDataDrawer->draw_RAIN_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawCloud :
                        windArrowsColor.setRgb(180, 180, 80);
                        mapDataDrawer->draw_CLOUD_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawHumid :
                        windArrowsColor.setRgb(180, 180, 80);
                        mapDataDrawer->draw_HUMID_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawTemp :
                        windArrowsColor.setRgb(255, 255, 255);
                        mapDataDrawer->draw_Temp_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawTempPot :
                        windArrowsColor.setRgb(255, 255, 255);
                        mapDataDrawer->draw_TempPot_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawDewpoint :
                        windArrowsColor.setRgb(255, 255, 255);
                        mapDataDrawer->draw_Dewpoint_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawDeltaDewpoint :
                        windArrowsColor.setRgb(180, 180, 80);
                        mapDataDrawer->draw_DeltaDewpoint_Color(pnt, proj, colorMapSmooth);
                        break;
                /*case MapDataDrawer::drawSnowDepth :
                        windArrowsColor.setRgb(140, 120, 100);
                        mapDataDrawer->draw_SNOW_DEPTH_Color(pnt, proj, colorMapSmooth);
                        break;*/
                case MapDataDrawer::drawSnowCateg :
                        windArrowsColor.setRgb(140, 120, 100);
                        mapDataDrawer->draw_SNOW_CATEG_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawFrzRainCateg :
                        windArrowsColor.setRgb(140, 120, 100);
                        mapDataDrawer->draw_FRZRAIN_CATEG_Color(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawCAPEsfc :
                        windArrowsColor.setRgb(100, 80, 80);
                        mapDataDrawer->draw_CAPEsfc(pnt, proj, colorMapSmooth);
                        break;
                case MapDataDrawer::drawCINsfc :
                        windArrowsColor.setRgb(100, 80, 80);
                        mapDataDrawer->draw_CINsfc(pnt, proj, colorMapSmooth);
                        break;
        }
        //printf("time show ColorMap = %d ms\n", t1.elapsed());

        //send gfs:40N,60N,140W,120W|2,2|24,48,72|PRESS,WIND,SEATMP,AIRTMP,WAVES

        if (showIsobars) {
            pnt.setPen(isobarsPen);
            mapDataDrawer->draw_Isobars(pnt, proj);
            if (showIsobarsLabels) {
                mapDataDrawer->draw_IsobarsLabels(pnt, proj);
            }
        }

        if (showIsotherms0) {
            pnt.setPen(isotherms0Pen);
            mapDataDrawer->draw_Isotherms0(pnt, proj);
            if (showIsotherms0Labels) {
                mapDataDrawer->draw_Isotherms0Labels(pnt, proj);
            }
        }

        if (showPressureMinMax) {
                mapDataDrawer->draw_PRESSURE_MinMax (pnt, proj);
        }
        if (showTemperatureLabels) {
                mapDataDrawer->draw_TEMPERATURE_Labels (pnt, proj);
        }
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

//-------------------------------------------------------
void Terrain::switchGribDisplay(bool windArrowOnly)
{
    if(windArrowOnly)
    {
        colorMapMode=MapDataDrawer::drawWind;
        colorMapSmooth=false;
        showWindArrows=true;
        showBarbules=true;
        showWindColorMap=false;
    }
    else
    {
        colorMapMode = Settings::getSetting("colorMapMode", 1).toInt();
        colorMapSmooth = Settings::getSetting("colorMapSmooth", true).toBool();
        showWindArrows  = Settings::getSetting("showWindArrows", true).toBool();
        showBarbules = Settings::getSetting("showBarbules", true).toBool();
        showWindColorMap = Settings::getSetting("showWindColorMap", true).toBool();
    }
}

void Terrain::slot_setDrawWindColors (bool b) {
    if (showWindColorMap != b) {
        showWindColorMap = b;
        Settings::setSetting("showWindColorMap", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
void Terrain::slotTemperatureLabels(bool b) {
    if (showTemperatureLabels != b) {
        showTemperatureLabels = b;
        Settings::setSetting("showTemperatureLabels", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setColorMapMode(int mode)
{    
    if (colorMapMode != mode)
    {
        colorMapMode=mode;
        Settings::setSetting("colorMapMode", mode);
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

//-------------------------------------------------------
void Terrain::setPressureMinMax (bool b) {
    if (showPressureMinMax != b) {
        showPressureMinMax = b;
        Settings::setSetting("showPressureMinMax", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawIsobars(bool b) {
    if (showIsobars != b) {
        showIsobars = b;
        Settings::setSetting("showIsobars", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setIsobarsStep(double step)
{
    if (isobarsStep != step) {

        DataManager * dataManager=centralWidget->get_dataManager();
        if(dataManager)
            dataManager->set_isoBarsStep(step);
        else
            qWarning() << "No grib present";
        Settings::setSetting("isobarsStep", step);
        isobarsStep = step;
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setDrawIsobarsLabels(bool b) {
    if (showIsobarsLabels != b) {
        showIsobarsLabels = b;
        Settings::setSetting("showIsobarsLabels", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawIsotherms0(bool b) {
    if (showIsotherms0 != b) {
        showIsotherms0 = b;
        Settings::setSetting("showIsotherms0", b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setIsotherms0Step(double step)
{
    if (isotherms0Step!=step) {
        DataManager * dataManager=centralWidget->get_dataManager();
        if(dataManager)
            dataManager->set_isoTherms0Step(step);
        Settings::setSetting("isotherms0Step", step);
        isotherms0Step = step;
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setDrawIsotherms0Labels(bool b) {
    if (showIsotherms0Labels != b) {
        showIsotherms0Labels = b;
        Settings::setSetting("showIsotherms0Labels", b);
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
    update();
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
    pnt->setRenderHint(QPainter::Antialiasing,true);
    pnt->drawPixmap(0,0, *imgAll);
    timerUpdated->start();
}
void Terrain::indicateWaitingMap()
{
    if(centralWidget->getMagnifier()!=NULL)
    {
        delete centralWidget->getMagnifier();
        centralWidget->setMagnifier(NULL);
        centralWidget->get_toolBar()->magnify->setChecked(false);
    }
    if(imgAll!=NULL && imgAll->paintingActive())
    {
        qWarning()<<"painting active in indicateWaitingMap()(1)";
        toBeRestarted=true;
        return;
    }
    if(imgAll!=NULL)
    {
        QPainter pnt_1;
        if(!pnt_1.begin(imgAll))
        {
            qWarning()<<"painting active in indicateWaitingMap()(2)";
            toBeRestarted=true;
            return;
        }
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
        pnt_1.end();
   }
   if (mustRedraw  ||  !isEarthMapValid  || !isWindMapValid)
   {
        draw_GSHHSandGRIB();
        isEarthMapValid = true;
        isWindMapValid = true;
        mustRedraw = false;
    }
    updateRoutine();
    mustRedraw = false;
    if(toBeRestarted)
    {
        toBeRestarted=false;
        isEarthMapValid = true;
        isWindMapValid = true;
        mustRedraw = false;
        this->indicateWaitingMap();
    }
}
void Terrain::updateRoutine()
{
    update();
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}
void Terrain::setRoutageGrib(ROUTAGE * routage)
{
    mutex.lock();
    this->routageGrib=routage;
    mutex.unlock();
    redrawGrib();
}
ROUTAGE * Terrain::getRoutageGrib()
{
    return routageGrib;
}

bool Terrain::daylight(QPainter *pnt, const vlmPoint &coords) //called with pnt!=NULL will draw night zone, called with coords!=NULL will return false if point if not under sun at point's eta
{
    if(pnt!=NULL && Settings::getSetting("showNight",1).toInt()!=1) return false;
    QDateTime date=QDateTime().currentDateTimeUtc();
     DataManager * dataManager=centralWidget->get_dataManager();
    if(pnt==NULL)
        date=QDateTime().fromTime_t(coords.eta).toUTC();
    else if(dataManager)
        date=QDateTime().fromTime_t(dataManager->get_currentDate()).toUTC();
    int nbDays=date.date().dayOfYear();
    double hour=date.time().hour()+date.time().minute()/60.0+date.time().second()/3600.0;
    double M=-3.6 + 0.9856*nbDays;
    double v= M+1.9*sin(degToRad(M));
    double L= v+102.9;
    double sinL=sin(degToRad(L));
    double d= 22.8*sinL+0.6*sinL*sinL*sinL;
    double latSun=d;
    double lonSun=-15.0*hour;
    double b=degToRad(latSun);
    double l=degToRad(lonSun);
    QPolygonF terminator1;
    QPolygonF terminator2;
    double X=0;
    double Y=0;
    double XS=0;
    double YS=0;
    proj->map2screenDouble(lonSun,latSun,&XS,&YS);
    for (int dist=360;dist>=0;--dist)
    {
        double f=degToRad(dist);
        double lat=radToDeg(asin(cos(b)*sin(f)));
        double x= -cos(l)*sin(b)*sin(f) - sin(l)*cos(f);
        double y= -sin(l)*sin(b)*sin(f) + cos(l)*cos(f);
        double lon=radToDeg(atan2(y,x));
        proj->map2screenByReference(lonSun,XS,lon,lat,&X,&Y);
        terminator1.append(QPointF(X,Y));
    }
    double reflect=terminator1.boundingRect().center().x();
    if(qRound(reflect)!=qRound(width/2.0))
    {
        double project=proj->getScale()*360.0;
        if(reflect>width/2.0)
            project=-project;
        terminator2=terminator1.translated(project,0.0);
    }
    if(pnt==NULL)
    {
        proj->map2screenDouble(coords.lon,coords.lat,&X,&Y);
        return !terminator1.containsPoint(QPointF(X,Y),Qt::OddEvenFill) && !terminator2.containsPoint(QPointF(X,Y),Qt::OddEvenFill);
    }
#ifdef __TERRAIN_QIMAGE
    QImage mask = QImage(width,height,QImage::Format_ARGB32_Premultiplied);
#else
    QPixmap mask = QPixmap(width,height);
#endif
    QColor night=Qt::black;
    night.setAlpha(Settings::getSetting("nightOpacity",120).toInt());
    QColor day=Qt::transparent;
    proj->map2screenDouble(lonSun,latSun,&X,&Y);
    mask.fill(day);
    QBrush brush(night);
    QPainter p(&mask);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setBrush(brush);
    p.setPen(Qt::NoPen);
    p.drawPolygon(terminator1.toPolygon());
    if(!terminator2.isEmpty())
        p.drawPolygon(terminator2.toPolygon());
    p.end();
    pnt->setCompositionMode(QPainter::CompositionMode_SourceOver);
#ifdef __TERRAIN_QIMAGE
    pnt->drawImage(QPoint(0,0),mask);
#else
    pnt->drawPixmap(0,0,mask);
#endif
    return false;
}
