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
#include <QTimer>

#include "Terrain.h"
#include "settings.h"
#include "GisReader.h"
#include "Projection.h"
#include "mycentralwidget.h"
#include "GshhsReader.h"
#include "loadImg.h"
#include "Orthodromie.h"
#include "MyView.h"
#include "ToolBar.h"
#include "MapDataDrawer.h"
#include "DataManager.h"
#include "Grib.h"
#include "GribRecord.h"
#include "routage.h"
#include <QGestureEvent>
#include "StatusBar.h"
#include <QToolTip>
#include <QScreen>
//#define traceTime


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
    showCountriesBorders  = Settings::getSetting(show_countriesBorders).toBool();

    showRivers   = Settings::getSetting(show_rivers).toBool();
    showCitiesNamesLevel = Settings::getSetting(show_citiesNamesLevel).toInt();
    showCountriesNames = Settings::getSetting(show_countriesNames).toBool();

    showIsobars  = Settings::getSetting(showIsobarsSet).toBool();
    showIsobarsLabels = Settings::getSetting(showIsobarsLabelsSet).toBool();
    isobarsStep = Settings::getSetting(isobarsStepSet).toDouble();
    isoBarLevelType = Settings::getSetting(isoBar_levelType).toInt();
    isoBarLevelValue = Settings::getSetting(isoBar_levelValue).toInt();

    showPressureMinMax = Settings::getSetting(showPressureMinMaxSet).toBool();

    showIsotherms0  = Settings::getSetting(showIsotherms0Set).toBool();
    showIsotherms0Labels  = Settings::getSetting(showIsotherms0LabelsSet).toBool();
    isotherms0Step = Settings::getSetting(isotherms0StepSet).toDouble();

    colorMapMode = Settings::getSetting(colorMap_mode).toInt();
    colorMapLevelType = Settings::getSetting(colorMap_levelType).toInt();
    colorMapLevelValue = Settings::getSetting(colorMap_levelValue).toInt();
    colorMapSmooth = Settings::getSetting(colorMapSmoothSet).toBool();

    frstArwMode = Settings::getSetting(frstArw_mode).toInt();
    frstArwLevelType = Settings::getSetting(frstArw_levelType).toInt();
    frstArwLevelValue = Settings::getSetting(frstArw_levelValue).toInt();
    showBarbules=Settings::getSetting(showBarbulesSet).toBool();;

    secArwMode = Settings::getSetting(secArw_mode).toInt();
    secArwLevelType = Settings::getSetting(secArw_levelType).toInt();
    secArwLevelValue = Settings::getSetting(secArw_levelValue).toInt();

    labelMode = Settings::getSetting(label_mode).toInt();
    labelLevelType = Settings::getSetting(label_levelType).toInt();
    labelLevelValue = Settings::getSetting(label_levelValue).toInt();

    qWarning() << "GrbDrawing param at start: " << colorMapMode << " / " << colorMapLevelType << " / " << colorMapLevelValue
                << " frstArw: " << frstArwMode << " / " << frstArwLevelType << " / " << frstArwLevelValue
                << " secArw: " << secArwMode << " / " << secArwLevelType << " / " << secArwLevelValue
                << " label: " << labelMode << " / " << labelLevelType << " / " << labelLevelValue;

    //----------------------------------------------------------------------------

    imgEarth = NULL;
    imgWind  = NULL;
    imgAll   = NULL;
    isEarthMapValid = false;
    isWindMapValid  = false;
    mustRedraw = true;

    gshhsReader = NULL;
    gisReader = NULL;


    int sX=Settings::getSetting(scalePosX).toInt();
    int sY=Settings::getSetting(scalePosY).toInt();
    scalePos=QPoint(sX,sY);
    updateGraphicsParameters();
    if(Settings::getSetting(enable_Gesture).toString()=="1")
    {
        this->setAcceptTouchEvents(true);
        this->grabGesture(Qt::TapGesture);
        this->grabGesture(Qt::TapAndHoldGesture);
        this->grabGesture(Qt::PinchGesture);
    }
    QTapAndHoldGesture::setTimeout(2000);
    QScreen * screen=QGuiApplication::primaryScreen();
    fingerSize=10*screen->physicalDotsPerInch()*0.0393700787; //10mm approx size of a finger

    debugGesture=new QGraphicsEllipseItem(-fingerSize/2.0,-fingerSize/2.0,fingerSize,fingerSize);
    debugGesture->setZValue(150);
    QPen pen(Qt::red);
    QBrush brush(Qt::red);
    debugGesture->setPen(pen);
    debugGesture->setFlag(QGraphicsWidget::ItemIsMovable,true);
    debugGesture->setBrush(brush);
    //centralWidget->getScene()->addItem(debugGesture);
}
//---------------------------------------------------------
// Events
//---------------------------------------------------------

void Terrain::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
    emit showContextualMenu(event);
}

void Terrain::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsWidget::mousePressEvent(e);
    e->accept();
    if(e->button()==Qt::LeftButton || e->button()==Qt::MidButton )
    {
        emit mousePress(e);
    }
    //centralWidget->getScene()->addItem(debugGesture);
}

void Terrain::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    QGraphicsWidget::mouseReleaseEvent(e);
    e->accept();
    if(e->button()==Qt::LeftButton)
    {
        emit mouseRelease(e);
    }
    //centralWidget->getScene()->removeItem(debugGesture);
}
bool Terrain::sceneEvent(QEvent *event)
{
#if 1
    if (event->type() == QEvent::Gesture)
    {
        QGestureEvent * gestureEvent=static_cast<QGestureEvent*>(event);
        if (QGesture * p1 = gestureEvent->gesture(Qt::PinchGesture))
        {
            qWarning()<<"PinchGesture detected in terrain"<<p1->state();
            QPinchGesture *pinch = static_cast<QPinchGesture *>(p1);
            QPointF pinchCenter=centralWidget->mapFromGlobal(pinch->centerPoint().toPoint());
            if(pinch->state()==Qt::GestureStarted)
            {
                centralWidget->slot_resetGestures();
                centralWidget->getView()->myScale(pinch->totalScaleFactor(),pinchCenter.x(),pinchCenter.y(),true);
            }
            else if(pinch->state()==Qt::GestureUpdated)
                centralWidget->getView()->myScale(pinch->totalScaleFactor(),pinchCenter.x(),pinchCenter.y(),true);
            else if (pinch->state()==Qt::GestureFinished)
            {
                double lat,lon;
                proj->screen2mapDouble(pinchCenter.x(),pinchCenter.y(),&lon,&lat);
                proj->zoomKeep(lon,lat,pinch->totalScaleFactor());
            }
            else if (pinch->state()==Qt::GestureCanceled)
                centralWidget->slot_resetGestures();
            event->accept();
        }
        if (QGesture * p2=gestureEvent->gesture(Qt::TapGesture))
        {
            QTapGesture *tap = static_cast<QTapGesture *>(p2);
            qWarning()<<"TapGesture detected in terrain"<<tap->state();
            QPointF tapCenter=tap->position();
            //debugGesture->setPos(tapCenter);
            if(tap->state()==Qt::GestureFinished)
            {
                myClearSelectedItems();
                double lat,lon;
                proj->screen2mapDouble(tapCenter.x(),tapCenter.y(),&lon,&lat);
                centralWidget->getMainWindow()->get_statusBar()->showGribData(lon, lat);
                event->accept();
            }
            else
                event->ignore();
        }
        if (QGesture *p3=gestureEvent->gesture(Qt::TapAndHoldGesture))
        {
            qWarning()<<"TapAndHoldGesture detected in terrain"<<p3->state();
            if(p3->state()==Qt::GestureFinished)
            {
                //QTapAndHoldGesture *p=static_cast<QTapAndHoldGesture*>(p3);
                QPointF tapCenter=p3->hotSpot();
                QPoint screenPos=tapCenter.toPoint();
                QPointF scenePos=centralWidget->mapFromGlobal(screenPos);
                myClearSelectedItems();
                //debugGesture->setPos(scenePos);
                int X=qRound(scenePos.x());
                int Y=qRound(scenePos.y());

                if(centralWidget->getScene()->selectedItems().isEmpty())
                {
                    centralWidget->getMainWindow()->showContextualMenu(X,Y,screenPos);
                }
                event->accept();
            }
            else
                event->ignore();
        }
        if (/*QGesture *p4=*/gestureEvent->gesture(Qt::PanGesture))
        {
            qWarning()<<"PanGesture detected in terrain";
            event->accept();
        }
        if (/*QGesture *p5=*/gestureEvent->gesture(Qt::SwipeGesture))
        {
            qWarning()<<"SwipeGesture detected in terrain";
            event->accept();
        }
        if (/*QGesture *p6=*/gestureEvent->gesture(Qt::CustomGesture))
        {
            qWarning()<<"CustomGesture detected in terrain ??";
            event->accept();
        }
    }
#endif
    return QGraphicsWidget::sceneEvent(event);
}
void Terrain::myClearSelectedItems()
{
    foreach (QGraphicsItem * i,centralWidget->getScene()->selectedItems())
    {
        i->setSelected(false);
        i->update();
    }
}

//-------------------------------------------
void Terrain::updateGraphicsParameters()
{
    setPalette(QPalette(Settings::getSetting(backgroundColor).value<QColor>()));

    centralWidget->getScene()->setBackgroundBrush(Settings::getSetting(seaColor).value<QColor>());

    landColorVal = Settings::getSetting(landColor).value<QColor>();
    landColorVal.setAlpha(Settings::getSetting(landOpacity).toInt());

    seaBordersPen.setColor(Settings::getSetting(seaBordersLineColor).value<QColor>());
    seaBordersPen.setWidthF(Settings::getSetting(seaBordersLineWidth).toDouble());

    boundariesPen.setColor(Settings::getSetting(boundariesLineColor).value<QColor>());
    boundariesPen.setWidthF(Settings::getSetting(boundariesLineWidth).toDouble());

    riversPen.setColor(Settings::getSetting(riversLineColor).value<QColor>());
    riversPen.setWidthF(Settings::getSetting(riversLineWidth).toDouble());

    isobarsPen.setColor(Settings::getSetting(isobarsLineColor).value<QColor>());
    isobarsPen.setWidthF(Settings::getSetting(isobarsLineWidth).toDouble());

    isotherms0Pen.setColor(Settings::getSetting(isotherms0LineColor).value<QColor>());
    isotherms0Pen.setWidthF(Settings::getSetting(isotherms0LineWidth).toDouble());

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

    //qWarning() << "[draw_GSHHSandGRIB]";
    centralWidget->getView()->setInteractive(false);
    if(centralWidget->getKap())
        centralWidget->getKap()->slot_updateProjection();
    QCursor oldcursor = cursor();
    setCursor(Qt::WaitCursor);
    if(imgAll==NULL || imgAll->width()!=width || imgAll->height()!=height)
    {
        if (imgAll != NULL) {
            delete imgAll;
            imgAll = NULL;
        }
        imgAll = new QPixmap(width,height);
    }
    imgAll->fill(Qt::transparent);
    QPainter pnt(imgAll);
    pnt.setRenderHint(QPainter::Antialiasing, true);
    pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);

    //===================================================
    // Dessin du fond de carte et des donnees GRIB
    //===================================================
    if (!isEarthMapValid || mustRedraw)
    {
        if(imgEarth==NULL || imgEarth->width()!=width || imgEarth->height()!=height)
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
        }
        imgEarth->fill(Qt::transparent);

        if (gshhsReader != NULL)
        {
            QPainter pnt1(imgEarth);
            pnt1.setRenderHint(QPainter::Antialiasing, true);
            pnt1.setRenderHint(QPainter::SmoothPixmapTransform, true);
            pnt1.setCompositionMode(QPainter::CompositionMode_Source);
#ifdef traceTime
            QTime t;
            t.start();
#endif
            gshhsReader->drawContinents(pnt1, proj, Qt::transparent, landColorVal);
#ifdef traceTime
        qWarning()<<"time to draw continents"<<t.elapsed();
#endif
        }
    }

    //===================================================
    // Dessin des donnees GRIB
    //===================================================

    if(centralWidget->get_dataManager()->isOk())
    {
#ifdef traceTime
            QTime t;
            t.start();
#endif
        drawGrib(pnt);
#ifdef traceTime
        qWarning()<<"time to draw grib"<<t.elapsed();
#endif
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
            QList<vlmLine*> * isochrones=routageGrib->getIsochrones();
            for(int i=isochrones->size()-1;i>0;--i)
            {
                QList<vlmPoint> * iso=isochrones->at(i)->getPoints();
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
                    QList<vlmPoint> * previousIso=isochrones->at(i-1)->getPoints();
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
#ifdef traceTime
            QTime t;
            t.start();
#endif
            gshhsReader->drawBoundaries(pnt, proj);
#ifdef traceTime
            qWarning()<<"time to draw boundaries"<<t.elapsed();
#endif
        }
        if (showRivers) {
            pnt.setPen(riversPen);
#ifdef traceTime
            QTime t;
            t.start();
#endif
            gshhsReader->drawRivers(pnt, proj);
#ifdef traceTime
            qWarning()<<"time to draw rivers"<<t.elapsed();
#endif
        }
    }

    if (gshhsReader && gshhsReader->getQuality()>0 && gisReader && showCountriesNames){
#ifdef traceTime
        QTime t;
        t.start();
#endif
        gisReader->drawCountriesNames(pnt, proj);
#ifdef traceTime
        qWarning()<<"time to draw countries"<<t.elapsed();
#endif
    }
    if (gshhsReader && gshhsReader->getQuality()>1 && gisReader && showCitiesNamesLevel > 0){
#ifdef traceTime
        QTime t;
        t.start();
#endif
        gisReader->drawCitiesNames(pnt, proj, showCitiesNamesLevel);
#ifdef traceTime
        qWarning()<<"time to draw cities"<<t.elapsed();
#endif
    }
    //===================================================

    if (gshhsReader != NULL)
    {
        pnt.setPen(seaBordersPen);
#ifdef traceTime
        QTime t;
        t.start();
#endif
        gshhsReader->drawSeaBorders(pnt, proj);
#ifdef traceTime
        qWarning()<<"time to draw sea boarder"<<t.elapsed();
#endif
    }
    //===================================================

    /*int save=0;
    if(save==1) imgEarth->save("test.jpg","JPG",100);*/

    drawCartouche(pnt);

    /*echelle*/
    drawScale(pnt);

    setCursor(oldcursor);
    daylight(&pnt,vlmPoint(0,0));
    centralWidget->getView()->resetTransform();
    centralWidget->getView()->hideViewPix();
    centralWidget->getView()->setInteractive(true);
#ifdef traceTime
        qWarning()<<"--------------------------------------";
#endif
//    if(gshhsReader)
//        gshhsReader->clearCells();
}

void Terrain::drawCartouche(QPainter &pnt) {
    QString cartouche="";
    QString cartoucheL2="";
    DataManager * dataManager=centralWidget->get_dataManager();
    if(dataManager) cartouche=dataManager->get_cartoucheData();
    if(this->gshhsReader) {
        if(!cartouche.isEmpty()) cartouche += " . ";
        cartouche=cartouche+tr("Niveau de detail des cotes: ")+QString().setNum(this->gshhsReader->getQuality()+1);
    }
    if(dataManager->isOk()) {
        cartoucheL2=dataManager->format_dataType(colorMapMode,colorMapLevelType,colorMapLevelValue);
        if(colorMapMode == frstArwMode && colorMapLevelType==frstArwLevelType && colorMapLevelValue==frstArwLevelValue)
            cartoucheL2 += " - *";
        else
            cartoucheL2 += " - " + dataManager->format_dataType(frstArwMode,frstArwLevelType,frstArwLevelValue);
        if(colorMapMode == secArwMode && colorMapLevelType==secArwLevelType && colorMapLevelValue==secArwLevelValue)
            cartoucheL2 += " - *";
        else
            cartoucheL2 += " - " + dataManager->format_dataType(secArwMode,secArwLevelType,secArwLevelValue);
    }
    QFont fontbig("TypeWriter", 12, QFont::Bold, false);
    fontbig.setPointSizeF(12.0+Settings::getSetting(defaultFontSizeInc).toDouble());
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

    pnt.drawText(5, 8+Fsize.height()/2, cartouche);
    if(!cartoucheL2.isEmpty()) {
        int pos = 8+Fsize.height()+1;
        Fsize=fm.size(Qt::TextSingleLine,cartoucheL2);
        pnt.setPen(transpcolor);
        pnt.drawRect(3,pos-5,Fsize.width()+2,Fsize.height());
        pnt.setPen(textcolor);
        pnt.drawText(5, pos+Fsize.height()/2, cartoucheL2);
    }

}

void Terrain::drawScale(QPainter &pnt) {
    if(Settings::getSetting(showScale).toInt()!=1) {
        //qWarning() << "Not drawing scale";
        return;
    }

    double w=width/8.0;
    double lon1,lat1,lon2,lat2;
    proj->screen2map(scalePos.x(),scalePos.y(),&lon1,&lat1);
    proj->screen2map(w+scalePos.x(),scalePos.y(),&lon2,&lat2);
    Orthodromie oo(lon1,lat1,lon2,lat2);
    double distance=oo.getLoxoDistance();
    bool meters=false;
    bool centimeters=false;
    double distanceMeters=distance*1852.0;
    QFont fontbig("TypeWriter", 12, QFont::Bold, false);
    fontbig.setPointSizeF(12.0+Settings::getSetting(defaultFontSizeInc).toDouble());
    fontbig.setStyleHint(QFont::TypeWriter);
    fontbig.setStretch(QFont::Condensed);
    QColor   transpcolor(255,255,255,120);
    QColor   textcolor(20,20,20,255);
    pnt.setBrush(transpcolor);
    pnt.setFont(fontbig);
    //pnt.setPen(transpcolor);
    QFontMetrics fm(fontbig);
    pnt.setPen(textcolor);


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
}


void Terrain::drawGrib(QPainter &pnt)
{
    MapDataDrawer * mapDataDrawer=centralWidget->get_mapDataDrawer();
        //QTime t1 = QTime::currentTime();
    /*qWarning() << "Grb drawing colorMap: " << colorMapMode << " / " << colorMapLevelType << " / " << colorMapLevelValue
                << " frstArw: " << frstArwMode << " / " << frstArwLevelType << " / " << frstArwLevelValue
                << " secArw: " << secArwMode << " / " << secArwLevelType << " / " << secArwLevelValue
                << " label: " << labelMode << " / " << labelLevelType << " / " << labelLevelValue;
*/
    if(colorMapMode!=DATA_NOTDEF)
        mapDataDrawer->drawColorMapGeneric_DTC(pnt,proj,colorMapMode,colorMapLevelType,colorMapLevelValue,colorMapSmooth);

        //printf("time show ColorMap = %d ms\n", t1.elapsed());

    if(frstArwMode!=DATA_NOTDEF) {
        QColor color=Settings::getSetting(frstArrowColor).value<QColor>();
        mapDataDrawer->drawArrowGeneric_DTC(pnt,proj,color,frstArwMode,frstArwLevelType,frstArwLevelValue,showBarbules);
    }

    if(secArwMode!=DATA_NOTDEF) {
        QColor color=Settings::getSetting(secArrowColor).value<QColor>();
        mapDataDrawer->drawArrowGeneric_DTC(pnt,proj,color,secArwMode,secArwLevelType,secArwLevelValue,false);
    }

    if(labelMode!=DATA_NOTDEF) {
        QColor color=Settings::getSetting(labelColor).value<QColor>();
        mapDataDrawer->draw_labelGeneric(pnt,proj,labelMode,labelLevelType,labelLevelValue,color);
    }

    if (showIsobars) {
        pnt.setPen(isobarsPen);
        mapDataDrawer->draw_Isobars(pnt, proj,isoBarLevelType,isoBarLevelValue);
        if (showIsobarsLabels) {
            mapDataDrawer->draw_IsobarsLabels(pnt, proj,isoBarLevelType,isoBarLevelValue);
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
}

//=========================================================
void Terrain::setDrawRivers(bool b) {
    if (showRivers != b) {
        showRivers = b;
        Settings::setSetting(show_rivers, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawCountriesBorders(bool b) {
    if (showCountriesBorders != b) {
        showCountriesBorders = b;
        Settings::setSetting(show_countriesBorders, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------

//-------------------------------------------------------
void Terrain::setCountriesNames(bool b) {
    if(gisReader)
    {
        gisReader->clearLists();
    }
    if (showCountriesNames != b) {
        showCountriesNames = b;
        Settings::setSetting(show_countriesNames, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setCitiesNamesLevel  (int level) {
    if(gisReader)
    {
        gisReader->clearLists();
    }
    if (showCitiesNamesLevel != level) {
        showCitiesNamesLevel = level;
        Settings::setSetting(show_citiesNamesLevel, level);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

int Terrain::compute_dataType(DataManager * dataManager,
                     int currentMode, int defaultMode1, int defaultMode2,
                     QMap<int,QStringList> * allowedMode) {
    int newType=DATA_NOTDEF;
    /* is current mode valid: ie in dataRange [0..DATA_MAX[, != NOT_DEF, allowed for this type of drawing*/
    if(currentMode >= 0 && currentMode < DATA_MAX && currentMode != DATA_NOTDEF &&
            allowedMode->contains(currentMode) && dataManager->hasDataType(currentMode)) {
        // using current mode
        newType=currentMode;
    }
    else {
        /* try first default value*/
        if(defaultMode1!= DATA_NOTDEF && dataManager->hasDataType(defaultMode1)) newType=defaultMode1;
        else {
            /* try second default value*/
            if(defaultMode2!= DATA_NOTDEF && dataManager->hasDataType(defaultMode2)) newType=defaultMode2;
            else {
                /* take first data */
                newType=dataManager->get_firstDataType();
            }
        }
    }
    return newType;
}

Couple Terrain::compute_level(DataManager * dataManager,int newType,int curLevelType, int curLevelValue,
                     QMap<int,QStringList> * allowedLevel) {
    //qWarning() << "[compute_level] ";
    QMap<int,QList<int>*> * levelList=NULL;
    levelList = dataManager->get_levelList(newType);
    bool found=false;

    Couple res;
    res.init(DATA_LV_NOTDEF,0);

    if(!levelList || !allowedLevel || allowedLevel->count()==0) {
        qWarning() << "levelList or allowedList bad";
        return res;
    }

    if(curLevelType>=0 && curLevelType<DATA_LV_MAX && curLevelType!=DATA_LV_NOTDEF &&
            levelList->contains(curLevelType) && allowedLevel->contains(curLevelType)) {
        res.a=curLevelType;
        if(levelList->value(curLevelType)) {
            found=true;
            if(levelList->value(curLevelType)->contains(curLevelValue))
                res.b=curLevelValue;
            else
                res.b=levelList->value(curLevelType)->first();
        }
    }

    if(!found) {
        /* try to use default value */
        Couple c=dataManager->get_defaultLevel(newType);
        if(levelList->contains(c.a) && allowedLevel->contains(c.a)) {
            res.a=c.a;
            if(levelList->value(c.a)) {
                found=true;
                if(levelList->value(c.a)->contains(c.b))
                    res.b=c.b;
                else
                    res.b=levelList->value(c.a)->first();
            }
        }
    }

    if(!found) {
        /* try to use first allowed level */
        QMapIterator<int,QList<int>*> it(*levelList);
        while(it.hasNext()) {
            it.next();
            if(allowedLevel->contains(it.key())) {
                res.a=it.key();
                res.b=it.value()->first(); // use first levelValue
                break;
            }
        }
    }


    return res;
}

void Terrain::update_mapDataAndLevel(void) {

    /*qWarning() << "[update_mapDataAndLevel] starting";
    qWarning() << "[update_mapDataAndLevel] before " << colorMapMode << " / " << colorMapLevelType << " / " << colorMapLevelValue
                << " frstArw: " << frstArwMode << " / " << frstArwLevelType << " / " << frstArwLevelValue
                << " secArw: " << secArwMode << " / " << secArwLevelType << " / " << secArwLevelValue
                << " label: " << labelMode << " / " << labelLevelType << " / " << labelLevelValue;
*/

    DataManager * dataManager=centralWidget->get_dataManager();
    if(!dataManager) return;

    int newType=-1;
    Couple lv;
    lv.init(DATA_LV_NOTDEF,0);

    /************/
    /* colorMap */
    // dataType
    newType=compute_dataType(dataManager,colorMapMode,DATA_WIND_VX,DATA_CURRENT_VX,
                             dataManager->get_dataTypes());

    //qWarning() << "mapData: newType=" << newType;
    if(newType==DATA_NOTDEF) {
        /* no data to display ==> set everything to NOTDEF & out */
        colorMapMode=DATA_NOTDEF;
        colorMapLevelType=DATA_LV_NOTDEF;
        colorMapLevelValue=0;
        frstArwMode=DATA_NOTDEF;
        frstArwLevelType=DATA_LV_NOTDEF;
        frstArwLevelValue=0;
        secArwMode=DATA_NOTDEF;
        secArwLevelType=DATA_LV_NOTDEF;
        secArwLevelValue=0;
    }
    else {
        // level
        lv = compute_level(dataManager,newType,colorMapLevelType,colorMapLevelValue,
                           dataManager->get_levelTypes());

        if(lv.a==DATA_LV_NOTDEF) {
            newType=DATA_NOTDEF;
            lv.b=0;
        }

        colorMapMode=newType;
        colorMapLevelType=lv.a;
        colorMapLevelValue=lv.b;
        //qWarning() << "mapData: level=" << lv.a <<" / " << lv.b;

        /**************/
        /* Frst Arrow */
        if(frstArwMode != DATA_NOTDEF) {
            //data type
            newType=compute_dataType(dataManager,frstArwMode,DATA_WIND_VX,DATA_CURRENT_VX,
                                     dataManager->get_arrowTypesFst());
            if(newType==DATA_NOTDEF) {
                frstArwMode=DATA_NOTDEF;
                frstArwLevelType=DATA_LV_NOTDEF;
                frstArwLevelValue=0;
            }
            else {
                // level
                lv = compute_level(dataManager,newType,frstArwLevelType,frstArwLevelValue,
                                   dataManager->get_levelTypes());
                if(lv.a==DATA_LV_NOTDEF) {
                    newType=DATA_NOTDEF;
                    lv.b=0;
                }
                frstArwMode=newType;
                frstArwLevelType=lv.a;
                frstArwLevelValue=lv.b;
            }
        }

        /**************/
        /* Sec Arrow */
        if(secArwMode != DATA_NOTDEF) {
            //data type
            newType=compute_dataType(dataManager,secArwMode,DATA_CURRENT_VX,DATA_NOTDEF,
                                     dataManager->get_arrowTypesSec());
            if(newType==DATA_NOTDEF) {
                secArwMode=DATA_NOTDEF;
                secArwLevelType=DATA_LV_NOTDEF;
                secArwLevelValue=0;
            }
            else {
                // level
                lv = compute_level(dataManager,newType,secArwLevelType,secArwLevelValue,
                                   dataManager->get_levelTypes());
                if(lv.a==DATA_LV_NOTDEF) {
                    newType=DATA_NOTDEF;
                    lv.b=0;
                }
                secArwMode=newType;
                secArwLevelType=lv.a;
                secArwLevelValue=lv.b;
            }
        }

        /**************/
        /* Labels */
        if(labelMode != DATA_NOTDEF) {
            //data type

            newType=compute_dataType(dataManager,labelMode,DATA_WIND_VX,DATA_NOTDEF,
                                     dataManager->get_dataTypes());
            if(newType==DATA_NOTDEF) {
                labelMode=DATA_NOTDEF;
                labelLevelType=DATA_LV_NOTDEF;
                labelLevelValue=0;
            }
            else {
                // level
                lv = compute_level(dataManager,newType,labelLevelType,labelLevelValue,
                                   dataManager->get_levelTypes());
                if(lv.a==DATA_LV_NOTDEF) {
                    newType=DATA_NOTDEF;
                    lv.b=0;
                }
                labelMode=newType;
                labelLevelType=lv.a;
                labelLevelValue=lv.b;
            }
        }
    }

    // everything is updated
    Settings::setSetting(colorMap_mode, colorMapMode);
    Settings::setSetting(colorMap_levelType, colorMapLevelType);
    Settings::setSetting(colorMap_levelValue, colorMapLevelValue);
    Settings::setSetting(frstArw_mode, frstArwMode);
    Settings::setSetting(frstArw_levelType, frstArwLevelType);
    Settings::setSetting(frstArw_levelValue, frstArwLevelValue);
    Settings::setSetting(secArw_mode, secArwMode);
    Settings::setSetting(secArw_levelType, secArwLevelType);
    Settings::setSetting(secArw_levelValue, secArwLevelValue);
    Settings::setSetting(label_mode, labelMode);
    Settings::setSetting(label_levelType, labelLevelType);
    Settings::setSetting(label_levelValue, labelLevelValue);

    /*qWarning() << "[update_mapDataAndLevel] after " << colorMapMode << " / " << colorMapLevelType << " / " << colorMapLevelValue
                << " frstArw: " << frstArwMode << " / " << frstArwLevelType << " / " << frstArwLevelValue
                << " secArw: " << secArwMode << " / " << secArwLevelType << " / " << secArwLevelValue
                << " label: " << labelMode << " / " << labelLevelType << " / " << labelLevelValue;

    qWarning() << "[update_mapDataAndLevel] done";*/
}

void Terrain::setColorMapMode(int dataType,int levelType, int levelValue) {
    if (colorMapMode != dataType || colorMapLevelType != levelType || colorMapLevelValue != levelValue ) {
        // valide data is under caller responsability
        colorMapMode=dataType;
        colorMapLevelType = levelType;
        colorMapLevelValue = levelValue;
        qWarning() << "[setColorMapMode] new value: " << colorMapMode << " / " << colorMapLevelType << " / " << colorMapLevelValue;
        Settings::setSetting(colorMap_mode, colorMapMode);
        Settings::setSetting(colorMap_levelType, colorMapLevelType);
        Settings::setSetting(colorMap_levelValue, colorMapLevelValue);
        mustRedraw = true;
        indicateWaitingMap();
    }
    else
        qWarning() << "[setColorMapMode] nothing changed => not redrawing";
}

void Terrain::setFrstArwMode(int dataType,int levelType, int levelValue) {
    if (frstArwMode != dataType || frstArwLevelType != levelType || frstArwLevelValue != levelValue) {
        frstArwMode=dataType;
        frstArwLevelType = levelType;
        frstArwLevelValue = levelValue;
        qWarning() << "[setFrstArwMode] new value: " << frstArwMode << " / " << frstArwLevelType << " / " << frstArwLevelValue;
        Settings::setSetting(frstArw_mode, frstArwMode);
        Settings::setSetting(frstArw_levelType, frstArwLevelType);
        Settings::setSetting(frstArw_levelValue, frstArwLevelValue);
        mustRedraw = true;
        indicateWaitingMap();
    }
    else
        qWarning() << "[setFrstArwMode] nothing changed => not redrawing";
}

void Terrain::setSecArwMode(int dataType,int levelType, int levelValue) {
    if (secArwMode != dataType || secArwLevelType != levelType || secArwLevelValue != levelValue) {
        secArwMode=dataType;
        secArwLevelType = levelType;
        secArwLevelValue = levelValue;
        qWarning() << "[setSecArwMode] new value: " << secArwMode << " / " << secArwLevelType << " / " << secArwLevelValue;
        Settings::setSetting(secArw_mode, secArwMode);
        Settings::setSetting(secArw_levelType, secArwLevelType);
        Settings::setSetting(secArw_levelValue, secArwLevelValue);
        mustRedraw = true;
        indicateWaitingMap();
    }
    else
        qWarning() << "[setSecArwMode] nothing changed => not redrawing";
}

void Terrain::setLabelMode(int dataType,int levelType, int levelValue) {
    if(labelMode!=dataType || labelLevelType!=levelType || labelLevelValue!=levelValue) {
        labelMode=dataType;
        labelLevelType = levelType;
        labelLevelValue = levelValue;
        qWarning() << "[setLabelMode] new value: " << labelMode << " / " << labelLevelType << " / " << labelLevelValue;
        Settings::setSetting(label_mode, labelMode);
        Settings::setSetting(label_levelType, labelLevelType);
        Settings::setSetting(label_levelValue, labelLevelValue);
        mustRedraw = true;
        indicateWaitingMap();
    }
    else
        qWarning() << "[setLabelMode] nothing changed => not redrawing";
}

void Terrain::setIsoBarAlt(int levelType,int levelValue) {
    if(isoBarLevelType!=levelType || isoBarLevelValue!=levelValue) {
        isoBarLevelType=levelType;
        isoBarLevelValue=levelValue;
        qWarning() << "[setIsoBarAlt] new value: " <<  labelLevelType << " / " << labelLevelValue;
        Settings::setSetting(isoBar_levelType, isoBarLevelType);
        Settings::setSetting(isoBar_levelValue, isoBarLevelValue);
        mustRedraw = true;
        indicateWaitingMap();
    }
    else
        qWarning() << "[setIsoBarAlt] nothing changed => not redrawing";
}

//-------------------------------------------------------
void Terrain::setColorMapSmooth (bool b) {
    if (colorMapSmooth != b) {
        colorMapSmooth=b;
        Settings::setSetting(colorMapSmoothSet, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setBarbules (bool b) {    
    if (showBarbules != b) {
        showBarbules = b;
        Settings::setSetting(showBarbulesSet, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setPressureMinMax (bool b) {    
    if (showPressureMinMax != b) {
        showPressureMinMax = b;
        Settings::setSetting(showPressureMinMaxSet, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawIsobars(bool b) {
    if (showIsobars != b) {
        showIsobars = b;
        DataManager * dataManager=centralWidget->get_dataManager();
        if(showIsobars && dataManager)
            dataManager->update_isos();
        qWarning() << "Drawing isoBar: " << b;
        Settings::setSetting(showIsobarsSet, b);
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
        Settings::setSetting(isobarsStepSet, step);
        isobarsStep = step;
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setDrawIsobarsLabels(bool b) {
    if (showIsobarsLabels != b) {
        showIsobarsLabels = b;
        Settings::setSetting(showIsobarsLabelsSet, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

//-------------------------------------------------------
void Terrain::setDrawIsotherms0(bool b) {
    if (showIsotherms0 != b) {
        showIsotherms0 = b;
        DataManager * dataManager=centralWidget->get_dataManager();
        if(showIsotherms0 && dataManager)
            dataManager->update_isos();
        Settings::setSetting(showIsotherms0Set, b);
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
        Settings::setSetting(isotherms0StepSet, step);
        isotherms0Step = step;
        mustRedraw = true;
        indicateWaitingMap();
    }
}
//-------------------------------------------------------
void Terrain::setDrawIsotherms0Labels(bool b) {
    if (showIsotherms0Labels != b) {
        showIsotherms0Labels = b;
        Settings::setSetting(showIsotherms0LabelsSet, b);
        mustRedraw = true;
        indicateWaitingMap();
    }
}

void Terrain::redrawAll()
{
    //qWarning() << "[redrawAll]";
    isEarthMapValid = false;
    isWindMapValid = false;
    indicateWaitingMap();
}

void Terrain::redrawGrib()
{
    //qWarning() << "[redrawGrib]";
    isWindMapValid = false;
    indicateWaitingMap();
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
    if(receivers(SIGNAL(terrainUpdated()))>0)
    {
        timerUpdated->start();
    }
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
#if 0
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
#endif
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
    if(pnt!=NULL && Settings::getSetting(showNight).toInt()!=1) return false;
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
    night.setAlpha(Settings::getSetting(nightOpacity).toInt());
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
