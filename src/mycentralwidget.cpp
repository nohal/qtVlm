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

***********************************************************************/

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QSound>
#include <QDesktopServices>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlQuery>
#include <QGesture>
#include "mycentralwidget.h"

#include "settings.h"
#include "opponentBoat.h"
#include "Projection.h"
#include "MainWindow.h"
#include "GshhsReader.h"
#include "Grib.h"
#include "GribRecord.h"
#include "Terrain.h"
#include "inetConnexion.h"
#include "MenuBar.h"
#include "mapcompass.h"
#include "selectionWidget.h"
#include "POI.h"
#include "boatVLM.h"
#include "xmlBoatData.h"
#include "xmlPOIData.h"
#include "routage.h"
#include "BoardVLM.h"
#include "vlmLine.h"
#include "dataDef.h"
#include "Util.h"
#include "DialogTwaLine.h"
#include "Player.h"
#include "Board.h"
#include "boat.h"
#include "boatReal.h"
#include "boatVLM.h"

#include "DialogSailDocs.h"
#include "DialogHorn.h"
#include "DialogPoiDelete.h"
#include "DialogRoute.h"
#include "DialogLoadGrib.h"
#include "DialogRace.h"
#include "DialogBoatAccount.h"
#include "DialogGribDate.h"
#include "DialogPoi.h"
#include "DialogPlayerAccount.h"
#include "DialogRoutage.h"
#include "DialogRealBoatConfig.h"

/*******************/
/*    myScene      */
/*******************/

myScene::myScene(myCentralWidget * parent) : QGraphicsScene(parent)
{
    this->parent = parent;
    hasWay=false;
    wheelTimer=new QTimer();
    wheelTimer->setSingleShot(true);
    wheelTimer->setInterval(5000);
    connect(wheelTimer,SIGNAL(timeout()),this, SLOT(wheelTimerElapsed()));
    wheelStrokes=0;
    QColor seaColor  = Settings::getSetting("seaColor", QColor(50,50,150)).value<QColor>();
    this->setBackgroundBrush(seaColor);
}

/* Events */

void  myScene::keyPressEvent (QKeyEvent *e)
{
    QString position;
    QStringList positions;
    switch(e->key())
    {
        case Qt::Key_Minus:
            parent->slot_Zoom_Out();
            break;
        case Qt::Key_Plus:
            parent->slot_Zoom_In();
            break;
        case Qt::Key_Up:
            parent->slot_Go_Up();
            break;
        case Qt::Key_Down:
            parent->slot_Go_Down();
            break;
        case Qt::Key_Left:
            parent->slot_Go_Left();
            break;
        case Qt::Key_Right:
            parent->slot_Go_Right();
            break;
        case Qt::Key_Escape:
            parent->escapeKeyPressed();
            break;
        case Qt::Key_Delete:
            parent->slot_delSelPOIs();
            break;
        case Qt::Key_F9:
            if(e->modifiers() & Qt::ControlModifier)
            {
                position.sprintf("%.4f ; %.4f ; %.4f",parent->getProj()->getScale(),parent->getProj()->getCX(),
                         parent->getProj()->getCY());
                Settings::setSetting("f9map",position);
           }
            else
            {
                position=Settings::getSetting("f9map","-1;-1;-1").toString();
                positions=position.split(";");
                if(positions[0]!="-1")
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toFloat(), positions[1].toDouble(), positions[2].toDouble());
            }
            break;
        case Qt::Key_F10:
            if(e->modifiers() & Qt::ControlModifier)
            {
                position.sprintf("%.4f ; %.4f ; %.4f",parent->getProj()->getScale(),parent->getProj()->getCX(),
                         parent->getProj()->getCY());
                Settings::setSetting("f10map",position);
           }
            else
            {
                position=Settings::getSetting("f10map","-1;-1;-1").toString();
                positions=position.split(";");
                if(positions[0]!="-1")
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toFloat(), positions[1].toDouble(), positions[2].toDouble());
            }
            break;
        case Qt::Key_F11:
            if(e->modifiers() & Qt::ControlModifier)
            {
                position.sprintf("%.4f ; %.4f ; %.4f",parent->getProj()->getScale(),parent->getProj()->getCX(),
                         parent->getProj()->getCY());
                Settings::setSetting("f11map",position);
           }
            else
            {
                position=Settings::getSetting("f11map","-1;-1;-1").toString();
                positions=position.split(";");
                if(positions[0]!="-1")
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toFloat(), positions[1].toDouble(), positions[2].toDouble());
            }
            break;
        case Qt::Key_F12:
            if(e->modifiers() & Qt::ControlModifier)
            {
                position.sprintf("%.4f ; %.4f ; %.4f",parent->getProj()->getScale(),parent->getProj()->getCX(),
                         parent->getProj()->getCY());
                Settings::setSetting("f12map",position);
           }
            else
            {
                position=Settings::getSetting("f12map","-1;-1;-1").toString();
                positions=position.split(";");
                if(positions[0]!="-1")
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toFloat(), positions[1].toDouble(), positions[2].toDouble());
            }
            break;
        default:
            parent->keyModif(e);
    }
}

void  myScene::keyReleaseEvent (QKeyEvent *e)
{
    parent->keyModif(e);
}

void myScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if(itemAt(e->scenePos())->data(0)==ISOPOINT)
    {
        ((vlmPointGraphic *) itemAt(e->scenePos()))->drawWay();
        hasWay=true;
        return;
    }
    else if(hasWay)
    {
        emit eraseWay();
        hasWay=false;
    }
    parent->mouseMove(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos()));
}

void myScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::LeftButton)
    {
        parent->mouseDoubleClick(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos()));
    }
}
void myScene::wheelEvent(QGraphicsSceneWheelEvent* e)
{
    if(e->orientation()!=Qt::Vertical) return;
    wheelPosX=e->scenePos().x();
    wheelPosY=e->scenePos().y();
    if(e->delta()<0)
        wheelStrokes--;
    else
        wheelStrokes++;
    if(e->modifiers()==Qt::ControlModifier)
        wheelCenter=true;
    else
        wheelCenter=false;
    wheelTimer->start(500);
}
void myScene::wheelTimerElapsed()
{
    parent->slot_Zoom_Wheel(wheelStrokes,wheelPosX,wheelPosY,wheelCenter);
    wheelStrokes=0;
}
bool myScene::event(QEvent * event)
{
#if 0
    if (event->type() == QEvent::Gesture)
    {
        qWarning()<<"gesture detected";
        QGestureEvent * gestureEvent=static_cast<QGestureEvent*>(event);
        wheelCenter=false;
        if (QGesture *g = gestureEvent->gesture(Qt::PanGesture))
        {
            if(g->state()!=Qt::GestureFinished) return false;
            QMessageBox::warning (0,
                tr("Gesture"),
                tr("Pan gesture detected"));
            //QPanGesture *pinch = static_cast<QPanGesture *>(g);
            //parent->slot_Go_Left();
        }
        else if (QGesture *g = gestureEvent->gesture(Qt::PinchGesture))
        {
            if(g->state()!=Qt::GestureFinished) return false;
            QMessageBox::warning (0,
                tr("Gesture"),
                tr("Pan gesture detected"));
            QPinchGesture *pinch = static_cast<QPinchGesture *>(g);
            if (pinch->property("scaleFactor").toReal()<0)
                wheelStrokes--;
            else
                wheelStrokes++;
        }
        else
        {
            QMessageBox::warning (0,
                tr("Gesture"),
                tr("Other gesture detected!!"));
        }
        wheelTimer->start(500);
        return true;
    }
#endif
    return QGraphicsScene::event(event);}

/*******************/
/* myCentralWidget */
/*******************/

myCentralWidget::myCentralWidget(Projection * proj,MainWindow * parent,MenuBar * menuBar) : QWidget(parent)
{
    this-> proj=proj;
    this->keepPos=true;
    this->mainW=parent;
    this->menuBar=menuBar;
    this->aboutToQuit=false;

    currentPlayer=NULL;

    resizing=false;
    this->twaTrace=NULL;
    /* item state */
    shLab_st = false;
    shPoi_st = false;
    shRoute_st = false;
    shOpp_st = false;
    shPor_st = false;

    /* scene and views */
    scene =  new myScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(QRect(0,0,width(),height()));

    view = new QGraphicsView(scene, this);
    view->setGeometry(0,0,width(),height());
//   view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    view->viewport()->grabGesture(Qt::PanGesture);
    view->viewport()->grabGesture(Qt::PinchGesture);
//    view->setAttribute(Qt::WA_AcceptTouchEvents);
//    view->grabGesture(Qt::PanGesture);
//    view->grabGesture(Qt::PinchGesture);

    /* other child */
    gshhsReader = new GshhsReader("maps/gshhs", 0);
    grib = new Grib();
    inetManager = new inetConnexion(mainW);
    replayTimer=new QTimer(this);
    replayTimer->setSingleShot(true);
    replayTimer->setInterval(Settings::getSetting("speed_replay",20).toInt());
    connect(replayTimer,SIGNAL(timeout()),this,SLOT(slot_replay()));

    /* item child */
    // Terre
    terre = new Terrain(this,proj);
    terre->setGSHHS_map(gshhsReader);
    terre->setCitiesNamesLevel(Settings::getSetting("showCitiesNamesLevel", 0).toInt());
//voir s'il faut mettre le slot ds centralWidget ou utiliser myScene
    connect(terre,SIGNAL(showContextualMenu(QGraphicsSceneContextMenuEvent *)),
            parent, SLOT(slotShowContextualMenu(QGraphicsSceneContextMenuEvent *)));
    connect(parent, SIGNAL(signalMapQuality(int)), terre, SLOT(slot_setMapQuality(int)));
    connect(menuBar->acView_GroupColorMap, SIGNAL(triggered(QAction *)), this, SLOT(slot_setColorMapMode(QAction *)));
    connect(menuBar->acMap_Rivers, SIGNAL(triggered(bool)), terre,  SLOT(setDrawRivers(bool)));
    connect(menuBar->acMap_CountriesBorders, SIGNAL(triggered(bool)), terre,  SLOT(setDrawCountriesBorders(bool)));
    connect(menuBar->acMap_CountriesNames, SIGNAL(triggered(bool)), terre,  SLOT(setCountriesNames(bool)));
    connect(menuBar->acView_WindColors, SIGNAL(triggered(bool)), terre,  SLOT(slot_setDrawWindColors(bool)));
    connect(menuBar->acView_ColorMapSmooth, SIGNAL(triggered(bool)), terre,  SLOT(setColorMapSmooth(bool)));
    connect(menuBar->acView_WindArrow, SIGNAL(triggered(bool)), terre,  SLOT(setDrawWindArrows(bool)));
    connect(menuBar->acView_Barbules, SIGNAL(triggered(bool)), terre,  SLOT(setBarbules(bool)));
    connect(menuBar->acView_TemperatureLabels, SIGNAL(triggered(bool)),terre,  SLOT(slotTemperatureLabels(bool)));
    connect(menuBar->acView_Isobars, SIGNAL(triggered(bool)), terre,  SLOT(setDrawIsobars(bool)));
    connect(menuBar->acView_GroupIsobarsStep, SIGNAL(triggered(QAction *)), this, SLOT(slotIsobarsStep()));
    connect(menuBar->acView_IsobarsLabels, SIGNAL(triggered(bool)), terre,  SLOT(setDrawIsobarsLabels(bool)));
    connect(menuBar->acView_PressureMinMax, SIGNAL(triggered(bool)), terre,  SLOT(setPressureMinMax(bool)));

    connect(menuBar->acView_Isotherms0, SIGNAL(triggered(bool)), terre,  SLOT(setDrawIsotherms0(bool)));

    connect(menuBar->acView_GroupIsotherms0Step, SIGNAL(triggered(QAction *)), this, SLOT(slotIsotherms0Step()));

    connect(menuBar->acView_Isotherms0Labels, SIGNAL(triggered(bool)), terre,  SLOT(setDrawIsotherms0Labels(bool)));

    connect(menuBar->acOptions_SH_sAll, SIGNAL(triggered(bool)), this,  SIGNAL(showALL(bool)));
    connect(menuBar->acOptions_SH_sAll, SIGNAL(triggered(bool)), this,  SLOT(slot_showALL(bool)));

    connect(menuBar->acOptions_SH_hAll, SIGNAL(triggered(bool)), this,  SIGNAL(hideALL(bool)));
    connect(menuBar->acOptions_SH_hAll, SIGNAL(triggered(bool)), this,  SLOT(slot_hideALL(bool)));

    connect(menuBar->acOptions_SH_Opp, SIGNAL(triggered(bool)), this,  SIGNAL(shOpp(bool)));
    connect(menuBar->acOptions_SH_Opp, SIGNAL(triggered(bool)), this,  SLOT(slot_shOpp(bool)));

    connect(menuBar->acOptions_SH_Poi, SIGNAL(triggered(bool)), this,  SIGNAL(shPoi(bool)));
    connect(menuBar->acOptions_SH_Poi, SIGNAL(triggered(bool)), this,  SLOT(slot_shPoi(bool)));

    connect(menuBar->acOptions_SH_Rou, SIGNAL(triggered(bool)), this,  SIGNAL(shRou(bool)));
    connect(menuBar->acOptions_SH_Rou, SIGNAL(triggered(bool)), this,  SLOT(slot_shRoute(bool)));

    connect(menuBar->acOptions_SH_Por, SIGNAL(triggered(bool)), this,  SIGNAL(shPor(bool)));
    connect(menuBar->acOptions_SH_Por, SIGNAL(triggered(bool)), this,  SLOT(slot_shPor(bool)));

    // removing direct forward of signal
    //connect(menuBar->acOptions_SH_Lab, SIGNAL(triggered(bool)), this,  SIGNAL(shLab(bool)));
    connect(menuBar->acOptions_SH_Lab, SIGNAL(triggered(bool)), this,  SLOT(slot_shLab(bool)));

    connect(menuBar->acOptions_SH_Com, SIGNAL(triggered(bool)), this,  SIGNAL(shCom(bool)));

    connect(menuBar->acOptions_SH_Pol, SIGNAL(triggered(bool)), this,  SIGNAL(shPol(bool)));
    connect(menuBar->acOptions_SH_Fla, SIGNAL(triggered(bool)), this,  SLOT(slot_shFla(bool)));

    connect(menuBar->acOptions_SH_Boa, SIGNAL(triggered(bool)), parent, SLOT(slot_centerBoat()));

    connect(this,SIGNAL(accountListUpdated()), parent, SLOT(slotAccountListUpdated()));

    connect(&dialogUnits, SIGNAL(accepted()), terre, SLOT(redrawAll()));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), terre, SLOT(updateGraphicsParameters()));
    scene->addItem(terre);
    horn=new QSound("img/boat_horn.wav");
    hornTimer=new QTimer(this);
    connect(hornTimer,SIGNAL(timeout()),this,SLOT(slot_playHorn()));
    this->hornDate=QDateTime::currentDateTime().toUTC();
    this->hornDate.setTimeSpec(Qt::UTC);
    this->hornActivated=false;

    // Compass
    compass = new mapCompass(proj,parent,this);
    scene->addItem(compass);
    connect(parent,SIGNAL(showCompassLine(double,double)),compass,SLOT(slot_compassLine(double,double)));
    connect(parent,SIGNAL(showCompassCenterBoat()),compass,SLOT(slot_compassCenterBoat()));
    connect(parent,SIGNAL(showCompassCenterWp()),compass,SLOT(slot_compassCenterWp()));
    connect(parent,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(selectedBoatChanged()),compass,SLOT(slot_paramChanged()));
    connect(scene,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(selectedBoatChanged()),this,SIGNAL(shRouBis()));
    connect(scene,SIGNAL(paramVLMChanged()),parent,SIGNAL(paramVLMChanged()));
    connect(scene,SIGNAL(paramVLMChanged()),this,SIGNAL(shRouBis()));
    connect(parent,SIGNAL(boatHasUpdated(boat*)),compass,SLOT(slot_paramChanged()));
    connect(this, SIGNAL(showALL(bool)),compass,SLOT(slot_paramChanged()));
    connect(this, SIGNAL(shFla()),scene,SIGNAL(paramVLMChanged()));
    connect(this, SIGNAL(hideALL(bool)),compass,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shCom(bool)),compass,SLOT(slot_shCom()));
    connect(this, SIGNAL(shPol(bool)),compass,SLOT(slot_shPol()));
    connect(proj,SIGNAL(projectionUpdated()),compass,SLOT(slot_compassCenterBoat()));
    this->compassRoute=NULL;
//    if(Settings::getSetting("showCompass",1).toInt()==1)
//        compass->show();
//    else
//        compass->hide();

    // Selection zone
    selection=new selectionWidget(proj,scene);
    connect(menuBar->acMap_Orthodromie, SIGNAL(triggered(bool)),
            selection,  SLOT(slot_setDrawOrthodromie(bool)));
    scene->addItem(selection);

    // Menu
    connect(menuBar->acMap_GroupCitiesNames, SIGNAL(triggered(QAction *)),
            this, SLOT(slot_map_CitiesNames()));

    // Opponents
    opponents = new opponentList(proj,mainW,this,inetManager);

     /* Dialogs */
    gribDateDialog = new DialogGribDate();
    poi_editor=new DialogPoi(parent,this);

    boatAcc = new DialogBoatAccount(proj,parent,this,inetManager);
    realBoatConfig = new DialogRealBoatConfig(this);
    playerAcc = new DialogPlayerAccount(proj,mainW,this,inetManager);

    raceDialog = new DialogRace(parent,this,inetManager);
    connect(raceDialog,SIGNAL(readRace()),this,SLOT(slot_readRaceData()));
    connect(raceDialog,SIGNAL(writeBoat()),this,SLOT(slot_writeBoatData()));

    dialogLoadGrib = new DialogLoadGrib();
    connect(dialogLoadGrib, SIGNAL(signalGribFileReceived(QString)),
            parent,  SLOT(slot_gribFileReceived(QString)));
    connect(menuBar->acOptions_Units, SIGNAL(triggered()), &dialogUnits, SLOT(exec()));
    connect(menuBar->acOptions_GraphicsParams, SIGNAL(triggered()), &dialogGraphicsParams, SLOT(exec()));

    /*Routes*/
    connect(menuBar->acRoute_add, SIGNAL(triggered()), this, SLOT(slot_addRouteFromMenu()));
    connect(menuBar->acRoute_import, SIGNAL(triggered()), this, SLOT(slot_importRouteFromMenu()));

    /*Routages*/
    connect(menuBar->acRoutage_add, SIGNAL(triggered()), this, SLOT(slot_addRoutageFromMenu()));
    nbRoutage=0;
    /* Boats */
    xmlData = new xml_boatData(proj,parent,this,inetManager);

    /* POIs*/
    while (!poi_list.isEmpty())
        delete poi_list.takeFirst();

    xmlPOI = new xml_POIData(proj,parent,this);
    /*Races*/
    this->NSZ=NULL;
}
void myCentralWidget::setCompassFollow(ROUTE * route)
{
    this->compassRoute=route;
    menuBar->ac_compassCenterBoat->setChecked(false);
    Settings::setSetting("compassCenterBoat", "0");
    emitUpdateRoute(mainW->getSelectedBoat());
}
void myCentralWidget::centerCompass(double lon, double lat)
{
    this->compass->compassCenter(lon,lat);
}

void myCentralWidget::loadBoat(void)
{
    emit readBoatData("boatAcc.dat",true);
}

void myCentralWidget::loadPOI(void)
{
    emit readPOIData("poi.dat");
}

myCentralWidget::~myCentralWidget()
{
    if(!mainW->getNoSave() && xmlPOI && xmlData)
    {
        xmlPOI->slot_writeData(route_list,poi_list,"poi.dat");
        xmlData->slot_writeData(player_list,race_list,QString("boatAcc.dat"));
    }
}

/***********************/
/* Data/pointer access */
/***********************/

bool myCentralWidget::compassHasLine(void)
{
    if(compass)
        return compass->hasCompassLine();
    else
        return false;
}

int myCentralWidget::getCompassMode(int m_x,int m_y)
{

    if(compass->hasCompassLine())
        return COMPASS_LINEON;
    else if(!compass || !compass->isVisible())
        return COMPASS_NOTHING;
    else if(compass->contains(QPointF(m_x-compass->x(),m_y-compass->y())))
        return COMPASS_UNDER;

    return COMPASS_NOTHING;
}

Grib * myCentralWidget::getGrib(void)
{
    if(grib && grib->isOk())
        return grib;
    else
        return NULL;
}

boat * myCentralWidget::getSelectedBoat(void)
{
    return mainW->getSelectedBoat();
}
time_t myCentralWidget::getNextVac(void)
{
    if(mainW->getSelectedBoat() && mainW->getSelectedBoat()->getType()==BOAT_VLM)
    {
        if(mainW->getSelectedBoat()->getLoch()<0.01)
            return QDateTime::currentDateTime().toTime_t();
        else
            return ((boatVLM *)mainW->getSelectedBoat())->getPrevVac()+((boatVLM *)mainW->getSelectedBoat())->getVacLen();
    }
    else
        return QDateTime::currentDateTime().toTime_t();
}

/*******************/
/* Zoom and move   */
/*******************/

#define BLOCK_SIG_BOAT() { if(mainW->getSelectedBoat()) mainW->getSelectedBoat()->blockSignals(true); }
#define UNBLOCK_SIG_BOAT() { if(mainW->getSelectedBoat()) mainW->getSelectedBoat()->blockSignals(false); }

void myCentralWidget::slot_Zoom_All()
{
    BLOCK_SIG_BOAT()
    proj->zoomAll();
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Go_Left()
{
    BLOCK_SIG_BOAT()
    proj->move( 0.2, 0);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Go_Right()
{

    BLOCK_SIG_BOAT()
    proj->move(-0.2, 0);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Go_Up()
{
    BLOCK_SIG_BOAT()
    proj->move(0,  -0.2);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Go_Down()
{
    BLOCK_SIG_BOAT()
    proj->move(0,  0.2);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Zoom_In(float quantity)
{
    BLOCK_SIG_BOAT()
    if(keepPos)
    {
        if(mainW->getSelectedBoat())
        {
            if (proj->isPointVisible(mainW->getSelectedBoat()->getLon(),mainW->getSelectedBoat()->getLat()))
            {
                proj->zoomKeep(mainW->getSelectedBoat()->getLon(),mainW->getSelectedBoat()->getLat(),quantity);
                UNBLOCK_SIG_BOAT()
                return;
            }
        }
    }
    proj->zoom(quantity);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Zoom_Out(float quantity)
{
    BLOCK_SIG_BOAT()
    if(keepPos)
    {
        if(mainW->getSelectedBoat())
        {
           if (proj->isPointVisible(mainW->getSelectedBoat()->getLon(),mainW->getSelectedBoat()->getLat()))
           {
               proj->zoomKeep(mainW->getSelectedBoat()->getLon(),mainW->getSelectedBoat()->getLat(),1/quantity);
               mainW->getSelectedBoat()->blockSignals(false);
               return;
           }
        }
    }
    proj->zoom(1/quantity);
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Zoom_Wheel(float quantity, int XX, int YY, bool centerOnWheel)
{
    BLOCK_SIG_BOAT()
    double lat,lon;
    proj->screen2map(XX,YY,&lon,&lat);
    double newScale=proj->getScale();
    if(quantity>0)
        newScale=newScale*(1.0+1.5*quantity/10.0);
    else
        newScale=newScale/(1.0-1.5*quantity/10.0);
    if(centerOnWheel)
    {
        //qWarning()<<"scale:"<<proj->getScale()<<"quantity:"<<quantity<<"newScale:"<<newScale;
        proj->setScaleAndCenterInMap(newScale,lon,lat);
        proj->map2screen(proj->getCX(),proj->getCY(),&XX,&YY);
        QPoint pointer(XX,YY);
        pointer=this->mapToGlobal(pointer);
        QCursor::setPos(pointer);
    }
    else
    {
        if(keepPos)
        {
            if(mainW->getSelectedBoat())
            {
               if (proj->isPointVisible(mainW->getSelectedBoat()->getLon(),mainW->getSelectedBoat()->getLat()))
               {
                   proj->zoomKeep(mainW->getSelectedBoat()->getLon(),
                                  mainW->getSelectedBoat()->getLat(),
                                  newScale/proj->getScale());
                   UNBLOCK_SIG_BOAT()
                   return;
               }
            }
        }
        proj->setScale(newScale);
    }
    UNBLOCK_SIG_BOAT()
}

void myCentralWidget::slot_Zoom_Sel()
{
    BLOCK_SIG_BOAT()
    double x0, y0, x1, y1;
    if (selection->getZone(&x0,&y0, &x1,&y1))
    {
        //qWarning() << "zoom on " << x0 << "," << y0 << " " << x1 << "," << y1;
        proj->zoomOnZone(x0,y0,x1,y1);
        selection->clearSelection();

    }
    else
    {
        zoomOnGrib();
    }
    UNBLOCK_SIG_BOAT()
}

/**************************/
/* Events                 */
/**************************/

void myCentralWidget::resizeEvent (QResizeEvent * /*e*/)
{
    if(resizing)
        return;
    resizing=true;
    view->setGeometry(0,0,width(), height());
    scene->setSceneRect(QRect(0,0,width()-4, height()-4));
    terre->updateSize(width()-4, height()-4);
    proj->setScreenSize( width()-4, height()-4);
    resizing=false;
}

void myCentralWidget::mouseMove(int x, int y, QGraphicsItem * )
{
    if(selection->isSelecting())
    {
        double xa,xb,ya,yb;
        selection->getZoneWithSens(&xa,&ya,&xb,&yb);
        mainW->statusBar_showSelectedZone(xa,ya,xb,yb);
    }
    else
    {
        double xx, yy;
        proj->screen2map(x,y, &xx, &yy);
        mainW->statusBar_showWindData(xx, yy);
        mainW->drawVacInfo();
    }

    if(selection->tryMoving(x,y))
        return;

    QListIterator<QGraphicsItem *> it (scene->items());

    while(it.hasNext())
    {
        QGraphicsItem * item = it.next();

        if(item->data(0) == POI_WTYPE && ((POI*)item)->tryMoving(x,y))
        {
            break ;
        }
        if(item->data(0) == COMPASS_WTYPE && ((mapCompass*)item)->tryMoving(x,y))
        {
            break ;
        }
        if(item->data(0) == BOATREAL_WTYPE && ((boatReal*)item)->tryMoving(x,y))
        {
            break ;
        }
    }

    /* no current move in sub item */
}

void myCentralWidget::keyModif(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        terre->setCursor(Qt::SizeAllCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
    else if (e->modifiers() == Qt::ShiftModifier) {
        terre->setCursor(Qt::UpArrowCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
    else {
        terre->setCursor(Qt::CrossCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
}

void myCentralWidget::mouseDoubleClick(int x, int y, QGraphicsItem * )
{
    double lon, lat;
    proj->screen2map(x,y, &lon, &lat);
    qWarning() << "Creating POI at: " << lat << "," << lon << " - " << Util::formatLatitude(lat) << "," << Util::formatLongitude(lon);
    slot_addPOI("",POI_TYPE_POI,(float)lat,(float)lon,-1,-1,false,mainW->getSelectedBoat());
}

void myCentralWidget::escapeKeyPressed(void)
{
    emit stopCompassLine();
    emit POI_selectAborted(NULL);
    selection->clearSelection();
    horn->stop();
    this->replayStep=10e6;
}
void myCentralWidget::slot_mousePress(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::MidButton)
        proj->setCentralPixel(e->scenePos().x(),e->scenePos().y());
    else
        selection->startSelection(e->scenePos().x(),e->scenePos().y());
}
void myCentralWidget::slot_mouseRelease(QGraphicsSceneMouseEvent* e)
{
    if(selection->isSelecting())
    {
        selection->stopSelection();
        if(e->modifiers() == Qt::ControlModifier)
        {
            double x0, y0, x1, y1;
            if (selection->getZone(&x0,&y0, &x1,&y1))
            {
                //qWarning() << "zoom on " << x0 << "," << y0 << " " << x1 << "," << y1;
                proj->zoomOnZone(x0,y0,x1,y1);
                selection->clearSelection();
            }
        }
    }
}

/**************************/
/* Grib                   */
/**************************/

void myCentralWidget::zoomOnGrib(void)
{
    double x0,y0, x1,y1, mh, mv;
    if (grib->getZoneExtension(&x0,&y0, &x1,&y1))
    {
        //qWarning() << "zoom on " << x0 << "," << y0 << " " << x1 << "," << y1;
        mh = fabs(x0-x1)*0.05;
        mv = fabs(y0-y1)*0.05;
        //proj->zoomOnZone(x0-mh,y0-mv, x1+mh,y1+mv);
        if(x0>x1)
        {
            double a=x1;
            x1=x0;
            x0=a;
        }
        if(y0<y1)
        {
            double a=y1;
            y1=y0;
            y0=a;
        }
        //proj->zoomOnZone(x0,y0,x1,y1);
        proj->zoomOnZone(x0-mh,y0-mv, x1+mh,y1+mv);
    }
}

void myCentralWidget::loadGribFile(QString fileName, bool zoom)
{
    if (!grib)
        return;

    grib->loadGribFile(fileName);
    if(!grib->isOk())
    {
        emit redrawAll();
        return;
    }
    if (zoom)
    {
        proj->blockSignals(true);
        zoomOnGrib();
        proj->blockSignals(false);
    }
    //else
        emit redrawAll();
}

void myCentralWidget::setCurrentDate(time_t t, bool uRoute)
{
    if (grib->getCurrentDate() != t)
    {
        grib->setCurrentDate(t);
        emit redrawGrib();
        if(uRoute)
        {
            emit updateRoute(NULL);
            emit updateRoutage();
        }
    }
}

time_t myCentralWidget::getCurrentDate(void)
{
    if(grib->isOk())
        return grib->getCurrentDate();
    return 0;
}

void myCentralWidget::showGribDate_dialog(void)
{
    if(gribDateDialog && grib->isOk())
    {
        time_t res;
        gribDateDialog->showDialog(grib->getCurrentDate(),grib->getListDates(),&res);
        setCurrentDate(res);
    }
}

void myCentralWidget::slot_fileLoad_GRIB()
{
    double x0, y0, x1, y1;

    if (selection->getZone(&x0,&y0, &x1,&y1))
    {
        dialogLoadGrib->setZone(x0, y0, x1, y1);
        dialogLoadGrib->exec();
        //emit updateRoute();
    }
    else
    {
        QMessageBox::warning (this,
            tr("Telechargement d'un fichier GRIB"),
            tr("Vous devez selectionner une zone de la carte."));
    }
}

void myCentralWidget::slotLoadSailsDocGrib(void)
{
    QString queryStr;
    QString param;


    double x0, y0, x1, y1;

#define DIR_STR_LAT(VAL) (VAL>=0?"N":"S")
#define DIR_STR_LON(VAL) (VAL>=0?"E":"W")


    if (selection->getZone(&x0,&y0, &x1,&y1))
    {
        param.sprintf("%f%s,%f%s,%f%s,%f%s",fabs(y0),DIR_STR_LAT(y0),
                      fabs(y1),DIR_STR_LAT(y1),
                      fabs(x0),DIR_STR_LON(x0),
                      fabs(x1),DIR_STR_LON(x1));
        QTextStream(&queryStr) << "mailto:query@saildocs.com?subject=Give me a Grib - "
                << QDateTime::currentDateTime().toString(tr("dd/MM/yyyy hh:mm"))
                               << "&body=GFS:"
                               << param
                               << "|0.5,0.5|0,3,6..384|WIND";

        // Format: mailto:query@saildocs.com?subject=Give me a Grib&body=send GFS:56N,59S,33E,87W|0.5,0.5|0,3,6..384|WIND


        if(Settings::getSetting("sDocExternalMail",1).toInt()==1)
            QDesktopServices::openUrl(QUrl(queryStr));
        else
        {
            DialogSailDocs * sailDocs_diag = new DialogSailDocs("send GFS:" + param + "|0.5,0.5|0,3,6..384|WIND",this);
            sailDocs_diag->exec();
            delete sailDocs_diag;
        }
        selection->clearSelection();
    }
    else
    {
        QMessageBox::warning (this,
            tr("Demande d'un fichier GRIB a sailsDoc"),
            tr("Vous devez selectionner une zone de la carte."));
    }


}

QString myCentralWidget::dataPresentInGrib(Grib* grib,
                                int dataType,int levelType,int levelValue,
                                bool *ok)
{
        if (dataType == GRB_DEWPOINT) {
                switch (grib->getDewpointDataStatus(levelType,levelValue)) {
                        case Grib::DATA_IN_FILE :
                                if (ok != NULL) *ok = true;
                                return tr("oui");
                                break;
                        case Grib::NO_DATA_IN_FILE :
                                if (ok != NULL) *ok = false;
                                return tr("non");
                                break;
                        case Grib::COMPUTED_DATA :
                        default :
                                if (ok != NULL) *ok = true;
                                return tr("oui (calcule par la formule de Magnus-Tetens)");
                                break;
                }
        }
        else {
                if (grib->getNumberOfGribRecords(dataType,levelType,levelValue) > 0) {
                        if (ok != NULL) *ok = true;
                        return tr("oui");
                }
                else {
                        if (ok != NULL) *ok = false;
                        return tr("non");
                }
        }
}

void myCentralWidget::slot_fileInfo_GRIB()
{
    
    QString msg;
    if (!grib ||  ! grib->isOk())
    {
        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            tr("Aucun fichir GRIB n'est charge."));
    }
    else {
        msg += tr("Fichier : %1\n") .arg(grib->getFileName().c_str());
        msg += tr("Taille : %1 octets\n") .arg(grib->getFileSize());
        msg += tr("\n");

        msg += tr("%1 enregistrements, ").arg(grib->getTotalNumberOfGribRecords());
        msg += tr("%1 dates :\n").arg(grib->getNumberOfDates());

        std::set<time_t> * sdates = grib->getListDates();
        msg += tr("    du %1\n").arg( Util::formatDateTimeLong(*(sdates->begin())) );
        msg += tr("    au %1\n").arg( Util::formatDateTimeLong(*(sdates->rbegin())) );

        msg += tr("\n");
        msg += tr("Donnees disponibles :\n");
        msg += tr("    Temperature : %1\n").arg(dataPresentInGrib(grib,GRB_TEMP,LV_ABOV_GND,2));
	msg += tr("    Pression : %1\n").arg(dataPresentInGrib(grib,GRB_PRESSURE,LV_MSL,0));
	msg += tr("    Vent  : %1\n").arg(dataPresentInGrib(grib,GRB_WIND_VX,LV_ABOV_GND,10));
	msg += tr("    Cumul de précipitations : %1\n").arg(dataPresentInGrib(grib,GRB_PRECIP_TOT,LV_GND_SURF,0));
        msg += tr("    Nebulosite : %1\n").arg(dataPresentInGrib(grib,GRB_CLOUD_TOT,LV_ATMOS_ALL,0));
        msg += tr("    Humidite relative : %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_REL,LV_ABOV_GND,2));
        msg += tr("    Isotherme 0degC : %1\n").arg(dataPresentInGrib(grib,GRB_GEOPOT_HGT,LV_ISOTHERM0,0));
        msg += tr("    Point de rosee : %1\n").arg(dataPresentInGrib(grib,GRB_DEWPOINT,LV_ABOV_GND,2));
        msg += tr("    Temperature (min) : %1\n").arg(dataPresentInGrib(grib,GRB_TMIN,LV_ABOV_GND,2));
        msg += tr("    Temperature (max) : %1\n").arg(dataPresentInGrib(grib,GRB_TMAX,LV_ABOV_GND,2));
        msg += tr("    Temperature (pot) : %1\n").arg(dataPresentInGrib(grib,GRB_TEMP_POT,LV_SIGMA,9950));
	msg += tr("    Neige (risque) : %1\n").arg(dataPresentInGrib(grib,GRB_SNOW_CATEG,LV_GND_SURF,0));
        msg += tr("    Neige (epaisseur) : %1\n").arg(dataPresentInGrib(grib,GRB_SNOW_DEPTH,LV_GND_SURF,0));
        msg += tr("    Humidite specifique :\n");
        msg += tr("        - 200: %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_SPEC,LV_ISOBARIC,200));
        msg += tr("        - 300: %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_SPEC,LV_ISOBARIC,300));
        msg += tr("        - 500: %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_SPEC,LV_ISOBARIC,500));
        msg += tr("        - 700: %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_SPEC,LV_ISOBARIC,700));
        msg += tr("        - 850: %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_SPEC,LV_ISOBARIC,850));
        

        GribRecord * gr = grib->getFirstGribRecord();
        msg += tr("\n");
        msg += tr("Grille : %1 points (%2x%3)\n")
                        .arg(gr->getNi()*gr->getNj()).arg(gr->getNi()).arg(gr->getNj());
        msg += tr("\n");
        msg += tr("Etendue :\n");
        QString pos1, pos2;
        pos1 = Util::formatPosition( gr->getX(0), gr->getY(0) );
        pos2 = Util::formatPosition( gr->getX(gr->getNi()-1), gr->getY(gr->getNj()-1) );
        msg += tr("%1  ->  %2\n").arg( pos1, pos2);

        msg += tr("\n");
        msg += tr("Date de reference : %1\n")
                        .arg(Util::formatDateTimeLong(gr->getRecordRefDate()));

        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            msg );
    }
}

/**************************/
/* POI                    */
/**************************/
POI * myCentralWidget::slot_addPOI(QString name,int type,float lat,float lon, float wph,int timestamp,bool useTimeStamp,boat *boat)
{
    POI * poi;

    if(name=="")
        name=QString(tr("POI"));
    if(boat==NULL) boat=mainW->getSelectedBoat();
    poi = new POI(name,type,lat,lon, proj,
                  mainW, this,wph,timestamp,useTimeStamp,boat);

    slot_addPOI_list(poi);
    //poi->show();
    return poi;
}

void myCentralWidget::slot_addPOI_list(POI * poi)
{
    poi_list.append(poi);
    scene->addItem(poi);
    connect(poi, SIGNAL(editPOI(POI*)),poi_editor, SLOT(editPOI(POI*)));
    connect(proj, SIGNAL(projectionUpdated()), poi, SLOT(slot_updateProjection()));
    connect(poi, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));

    connect(this, SIGNAL(showALL(bool)),poi,SLOT(slot_shShow()));
    connect(this, SIGNAL(hideALL(bool)),poi,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shPoi(bool)),poi,SLOT(slot_shPoi()));
    connect(this, SIGNAL(shLab(bool)),poi,SLOT(slot_shLab(bool)));
}

void myCentralWidget::slot_delPOI_list(POI * poi)
{
    poi_list.removeAll(poi);
    scene->removeItem(poi);
    disconnect(poi, SIGNAL(editPOI(POI*)),poi_editor, SLOT(editPOI(POI*)));
    disconnect(proj, SIGNAL(projectionUpdated()), poi, SLOT(slot_updateProjection()));
    disconnect(poi, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    emit twaDelPoi(poi);
}

void myCentralWidget::slot_delAllPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        QListIterator<POI*> i (poi_list);

        int rep = QMessageBox::question (this,
            tr("Suppression de toutes les marques"),
             tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (rep != QMessageBox::Yes)
            return;
        QListIterator<ROUTE*> r (route_list);
        while(r.hasNext())
        {
            ROUTE * route=r.next();
            route->setTemp(true);
        }

        while(i.hasNext())
        {
            POI * poi = i.next();
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                if(poi->getRoute()!=NULL)
                {
                    if(poi->getRoute()->getFrozen()||poi->getRoute()->getHidden()||poi->getRoute()->isBusy()) continue;
                    poi->setRoute(NULL);
                }
                slot_delPOI_list(poi);
                delete poi;
            }

        }
        selection->clearSelection();
        r.toFront();
        while(r.hasNext())
        {
            ROUTE * route=r.next();
            route->setTemp(false);
        }
        emit updateRoute(NULL);
    }
}

void myCentralWidget::slot_delSelPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        int res_mask;
        DialogPoiDelete * dialog_sel = new DialogPoiDelete();
        dialog_sel->exec();
        if((res_mask=dialog_sel->getResult())<0)
            return;
        QListIterator<ROUTE*> r (route_list);
        while(r.hasNext())
        {
            ROUTE * route=r.next();
            route->setTemp(true);
        }

        QListIterator<POI*> i (poi_list);

        while(i.hasNext())
        {
            POI * poi = i.next();
            if(!(poi->getTypeMask() & res_mask))
                continue;
            //qWarning() << "POI: " << poi->getName() << " mask=" << poi->getTypeMask();
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                if(poi->getRoute()!=NULL)
                {
                    if(poi->getRoute()->getFrozen()||poi->getRoute()->getHidden()||poi->getRoute()->isBusy()) continue;
                    poi->setRoute(NULL);
                }
                slot_delPOI_list(poi);
                delete poi;
            }
        }
        selection->clearSelection();
        r.toFront();
        while(r.hasNext())
        {
            ROUTE * route=r.next();
            route->setTemp(false);
        }
        selection->clearSelection();
        emit updateRoute(NULL);
    }
}

void myCentralWidget::slot_showALL(bool)
{
    shLab_st=false;
    emit shLab(shLab_st);
    shPoi_st=false;
    shRoute_st=false;
    shOpp_st=false;
    shPor_st=false;
}

void myCentralWidget::slot_hideALL(bool)
{
    shLab_st=true;
    emit shLab(shLab_st);
    shPoi_st=true;
    shRoute_st=true;
    shOpp_st=true;
    shPor_st=true;
}

void myCentralWidget::slot_shLab(bool)
{
       shLab_st=!shLab_st;
       emit shLab(shLab_st);
}

void myCentralWidget::slot_shPoi(bool)
{
       shPoi_st=!shPoi_st;
}

void myCentralWidget::slot_shRoute(bool)
{
       shRoute_st=!shRoute_st;
}

void myCentralWidget::slot_shOpp(bool)
{
       shOpp_st=!shOpp_st;
}

void myCentralWidget::slot_shPor(bool)
{
    shPor_st=!shPor_st;
}
void myCentralWidget::slot_editHorn()
{
    DialogHorn *dh=new DialogHorn(this);
    dh->exec();
    delete dh;
    setHorn();
}
void myCentralWidget::slot_startReplay()
{
    if(mainW->getSelectedBoat()==NULL) return;
    emit startReplay(true);
    replayTimer->setInterval(Settings::getSetting("speed_replay",20).toInt());
    replayTimer->start();
    this->replayStep=-1;
}
void myCentralWidget::slot_replay()
{
    replayStep++;
    if(replayStep>Settings::getSetting("trace_length",12).toInt()*60*60/mainW->getSelectedBoat()->getVacLen())
    {
        replayTimer->stop();
        emit startReplay(false);
        return;
    }
    emit replay(replayStep);
    QApplication::processEvents();
    replayTimer->start();
}
void myCentralWidget::slot_takeScreenshot()
{
    // Create the image and render it...
    int w=proj->getW(),h=proj->getH();
    QImage * image = new QImage(w,h,QImage::Format_ARGB32_Premultiplied);
    QPainter * p = new QPainter(image);
    p->setRenderHint(QPainter::Antialiasing);
    scene->render(p);
    p->end();
    QString screenshotPath=Settings::getSetting("screenShotFolder","").toString();
    QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Photo Ecran"), screenshotPath, "Screenshot (*.png)");
    if(fileName.isEmpty() || fileName.isNull()) return;
    QFile::remove(fileName);
    QFile screenshotFile(fileName);
    QFileInfo info(screenshotFile);
    if(QString::compare(info.suffix(),"png")!=0) {
        QMessageBox::warning(0,QObject::tr("Sauvegarde ecran"),
             QString(QObject::tr("Un nom de fichier valide portant l'extension .png est requis")).arg(fileName));
        return;
    }
    if(!screenshotFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0,QObject::tr("Sauvegarde ecran"),
             QString(QObject::tr("Impossible de creer le fichier %1")).arg(fileName));
        return;
    }
    Settings::setSetting("screenShotFolder",info.absoluteDir().path());
    // Save it..
    image->save(fileName, "PNG", -1);
    if (mainW->getSelectedBoat()->getType()==BOAT_VLM)
        ((boatVLM*)mainW->getSelectedBoat())->exportBoatInfoLog(fileName);
}

void myCentralWidget::setHorn()
{
    if(this->hornIsActivated())
    {
        QDateTime now=QDateTime::currentDateTime().toUTC();
        now.setTimeSpec(Qt::UTC);
        int r=now.secsTo(hornDate);
        if (r<0)
        {
            hornTimer->stop();
            this->hornActivated=false;
        }
        else
        {
            hornTimer->setSingleShot(true);
            hornTimer->start(r*1000);
        }
    }
    else
        hornTimer->stop();
}
void myCentralWidget::slot_playHorn()
{
    horn->setLoops(2000);
    horn->play();
    QMessageBox::information(0,QObject::tr("Corne de brume activee"),QString(),QString(QObject::tr("Arreter la corne de brume")));
    horn->stop();
}
bool myCentralWidget::freeRouteName(QString name,ROUTE * thisroute)
{
    QListIterator<ROUTE*> i (route_list);

    while(i.hasNext())
    {
        ROUTE * route = i.next();
        if (route->getName()==name && (thisroute==NULL || route!=thisroute)) return false;
    }
    return true;
}
bool myCentralWidget::freeRoutageName(QString name,ROUTAGE * thisroutage)
{
    QListIterator<ROUTAGE*> i (routage_list);

    while(i.hasNext())
    {
        ROUTAGE * routage = i.next();
        if (routage->getName()==name && routage!=thisroutage) return false;
    }
    return true;
}
void myCentralWidget::slot_importRouteFromMenu()
{
    QString routePath=Settings::getSetting("importRouteFolder","").toString();
    QDir dirRoute(routePath);
    if(!dirRoute.exists())
    {
        routePath=QApplication::applicationDirPath();
        Settings::setSetting("importRouteFolder",routePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Ouvrir un fichier Route"), routePath, "Routes (*.csv *.txt *.CSV *.TXT *.xml *.XML)");
    if(fileName.isEmpty() || fileName.isNull()) return;

    QFile routeFile(fileName);
    if(!routeFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(0,QObject::tr("Lecture de route"),
             QString(QObject::tr("Impossible d'ouvrir le fichier %1")).arg(fileName));
        return;
    }
    QFileInfo info(routeFile);
    Settings::setSetting("importRouteFolder",info.absoluteDir().path());
    bool ok;
    QString routeName;
    if(info.suffix().toLower()!="xml")
    {
        QTextStream stream(&routeFile);
        QString line;
        QStringList list;
        line=stream.readLine();
        if(line.isNull())
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                 QString(QObject::tr("Fichier %1 vide")).arg(fileName));
            routeFile.close();
            return;
        }
        int format=ADRENA_FORMAT;
        list=line.split('\t');
        int timeOffset=0;
        QString temp=list[0];
        temp.squeeze();
        if(temp.toUpper()=="W0")
        {
            format=MS_FORMAT;
            bool ok;
            timeOffset=QInputDialog::getInteger(0,QString(QObject::tr("Importation de routage MaxSea")),QString(QObject::tr("Heures a ajouter/enlever pour obtenir UTC (par ex -2 pour la France)")),0,-24,24,1,&ok);
            if(!ok) return;
        }
        else
        {
            list = line.split(';');
            if(list[0].toUpper() != "POSITION" && format!=MS_FORMAT)
            {
                if(list.count()==6)
                {
                    format=SBS_FORMAT;
                }
                else
                {
                    QMessageBox::warning(0,QObject::tr("Lecture de route"),
                         QString(QObject::tr("Fichier %1 invalide (doit commencer par POSITION et non '%2'), ou alors etre au format sbsRouteur"))
                                    .arg(fileName)
                                    .arg(list[0].toUpper().left(20)));
                    routeFile.close();
                return;
                }
            }
        }
        routeName=QInputDialog::getText(this,tr("Nom de la route a importer"),tr("Nom de la route"),QLineEdit::Normal,"ImportedRoute",&ok);
        if(!ok)
        {
            routeFile.close();
            return;
        }
        ROUTE * route=addRoute();
        if (routeName.isEmpty() || !freeRouteName(routeName.trimmed(),route))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Ce nom est deja utilise ou invalide"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            routeFile.close();
            this->deleteRoute(route);
            return;
        }
        route->setName(routeName);
        update_menuRoute();
        route->setBoat(mainW->getSelectedBoat());
        route->setStartFromBoat(false);
        route->setStartTimeOption(3);
        route->setColor(QColor(227,142,42,255));
        route->setImported();
        route->setFrozen(true);

        int n=0;
        QString poiName;
        double lon,lat;
        QDateTime start;
        if(format==ADRENA_FORMAT)
            line=stream.readLine();
        while(!line.isNull())
        {
            n++;
            switch(format)
            {
            case ADRENA_FORMAT:
                {
                    list = line.split(';');
                    lat=list[0].mid(0,2).toInt()+list[0].mid(3,6).toFloat()/60.0;
                    if(list[0].mid(10,1)!="N") lat=-lat;
                    lon=list[0].mid(13,3).toInt()+list[0].mid(17,6).toFloat()/60.0;
                    if(list[0].mid(24,1)!="E") lon=-lon;
                    start=QDateTime::fromString(list[1],"dd/MM/yyyy hh:mm:ss");
                    start.setTimeSpec(Qt::UTC);
                    break;
                }
            case SBS_FORMAT:
                {
                    list = line.split(';');
                    start=QDateTime::fromString(list[0].simplified(),"dd-MMM.-yyyy hh:mm");
                    if(!start.isValid())
                    {
                        start=QDateTime::fromString(list[0].simplified(),"dd-MMM hh:mm");
                        start=start.addYears(QDate::currentDate().year()-1900);
                    }
                    start=start.toUTC();
                    start.setTimeSpec(Qt::UTC);
                    QString position=list[3];
                    position.remove(" ",Qt::CaseInsensitive);
                    position.replace("\"","q");
                    position.remove(QChar(0xC2));
                    position.replace(QChar(0xB0),"d");
                    if(position.contains("N",Qt::CaseInsensitive))
                        position.truncate(position.indexOf("N")+1);
                    else
                        position.truncate(position.indexOf("S")+1);
                    QStringList temp=position.split("d");
                    float deg=temp[0].toInt();
                    temp=temp[1].split("'");
                    float min=temp[0].toInt();
                    temp=temp[1].split("q");
                    float sec=temp[0].toFloat();
                    min=min+sec/60;
                    lat=deg+min/60;
                    if(list[3].contains("S"))
                        lat=-lat;
                    position=list[3];
                    position.remove(" ",Qt::CaseInsensitive);
                    position.replace("\"","q");
                    position.remove(QChar(0xC2));
                    position.replace(QChar(0xB0),"d");
                    if(position.contains("N",Qt::CaseInsensitive))
                        position=position.mid(position.indexOf("N")+1);
                    else
                        position=position.mid(position.indexOf("S")+1);
                    temp=position.split("d");
                    deg=temp[0].toInt();
                    temp=temp[1].split("'");
                    min=temp[0].toInt();
                    temp=temp[1].split("q");
                    sec=temp[0].toFloat();
                    min=min+sec/60;
                    lon=deg+min/60;
                    if(list[3].contains("W",Qt::CaseInsensitive))
                        lon=-lon;
                    break;
                }
            case MS_FORMAT:
                {
                    list = line.split('\t');
                    start=QDateTime::fromString(list[3].simplified(),"dd/MM/yyyy hh:mm:ss");
                    if (!start.isValid())
                        start=QDateTime::fromString(list[3].simplified(),"dd/MM/yyyy");
                    start.setTimeSpec(Qt::UTC);
                    start=start.addSecs(timeOffset*60*60);
                    QStringList position=list[13].split(QChar(0xB0));
                    lat=position.at(0).toInt();
                    QString temp=position.at(1);
                    if(temp.contains("N",Qt::CaseInsensitive))
                    {
                        temp.truncate(temp.indexOf("N")-1);
                        lat=lat+temp.toFloat()/60;
                    }
                    else
                    {
                        temp.truncate(temp.indexOf("S")-1);
                        lat=-(lat+temp.toFloat()/60);
                    }
                    position=list[14].split(QChar(0xB0));
                    lon=position.at(0).toInt();
                    temp=position.at(1);
                    if(temp.contains("E",Qt::CaseInsensitive))
                    {
                        temp.truncate(temp.indexOf("E")-1);
                        lon=lon+temp.toFloat()/60;
                    }
                    else
                    {
                        temp.replace("O","W");
                        temp.truncate(temp.indexOf("W")-1);
                        lon=-(lon+temp.toFloat()/60);
                    }
                    break;
                }
            }
            if(n==1)
                route->setStartTime(start);
            QString poiN;
            poiN.sprintf("%.5i",n);
            poiName=route->getName()+poiN;
            POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
            poi->setRoute(route);
            poi->setRouteTimeStamp(start.toTime_t());
            line=stream.readLine();
        }
        routeFile.close();
        route->setHidePois(true);
        route->setImported();
        route->setFrozen2(false);//calculate only once and relock immediately
        route->setFrozen2(true);
    }
    else
    {
        QFile xsd("export.xsd");
        if(!xsd.open(QIODevice::ReadOnly))
        {
            qWarning()<<"cannot open export.xsd";
            return;
        }
        QXmlSchema schema;
        schema.load(&xsd);
        xsd.close();
        if(!schema.isValid())
        {
            qWarning()<<"shema is unvalid";
            return;
        }
        QXmlSchemaValidator validator(schema);
        //freopen("stderr.txt","w",stderr);
        if(!validator.validate(&routeFile,QUrl::fromLocalFile(routeFile.fileName())))
        {
            qWarning()<<"xml is NOT valid";
            routeFile.close();
            return;
        }
        qWarning()<<"validation ok";
        routeFile.seek(0);
        QDomDocument doc;
        int errorLine,errorColumn;
        QString errorStr;
        if(!doc.setContent(&routeFile,true,&errorStr,&errorLine,&errorColumn))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                                 QString("Erreur ligne %1, colonne %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
            return ;
        }
        routeFile.close();
        QDomElement root = doc.documentElement();
        QDomElement routeList=root.firstChildElement("RouteList");
        QDomElement routeDetail=routeList.firstChildElement("Route");
        while(!routeDetail.isNull())
        {
            routeName=routeDetail.firstChildElement("Name").firstChild().toText().data();
            routeName=QInputDialog::getText(this,tr("Nom de la route a importer"),tr("Nom de la route"),QLineEdit::Normal,routeName,&ok);
            if(!ok)
            {
                return;
            }
            ROUTE * route=addRoute();
            if (routeName.isEmpty() || !freeRouteName(routeName.trimmed(),route))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Ce nom est deja utilise ou invalide"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                routeFile.close();
                this->deleteRoute(route);
                return;
            }
            route->setName(routeName);
            update_menuRoute();
            route->setBoat(mainW->getSelectedBoat());
            route->setStartFromBoat(false);
            route->setStartTimeOption(3);
            QDomElement ss=routeDetail.firstChildElement("TrackColor");
            route->setColor(QColor(ss.firstChildElement("R").firstChild().toText().data().toInt(),
                                   ss.firstChildElement("G").firstChild().toText().data().toInt(),
                                   ss.firstChildElement("B").firstChild().toText().data().toInt(),
                                   ss.firstChildElement("A").firstChild().toText().data().toInt()));
            route->setImported();
            route->setFrozen(true);
            QDomElement track=routeDetail.firstChildElement("Track");
            QDomElement trackPoint=track.firstChildElement("TrackPoint");
            int n=0;
            QString poiName;
            double lon,lat;
            QDateTime start;
            while(!trackPoint.isNull())
            {
                QVariant var=trackPoint.firstChildElement("ActionDate").firstChild().toText().data();
                var.convert(QVariant::DateTime);
                start=var.toDateTime();
                start=start.toUTC();
                if(n==0)
                    route->setStartTime(start);
                n++;
                QString poiN;
                lon=trackPoint.firstChildElement("Coords").firstChildElement("Lon").firstChild().toText().data().toFloat();
                lat=trackPoint.firstChildElement("Coords").firstChildElement("Lat").firstChild().toText().data().toFloat();
                poiN.sprintf("%.5i",n);
                poiName="I"+poiN;
                POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
                poi->setRoute(route);
                poi->setRouteTimeStamp(start.toTime_t());
                trackPoint=trackPoint.nextSiblingElement("TrackPoint");
            }
            route->setHidePois(true);
            route->setImported();
            route->setFrozen2(false);//calculate only once and relock immediately
            route->setFrozen2(true);
            routeDetail=routeDetail.nextSiblingElement("Route");
        }
    }
}

void myCentralWidget::slot_twaLine()
{
    int X,Y;
    mainW->getXY(&X,&Y);
    double lon, lat;
    proj->screen2map(X,Y, &lon, &lat);
    twaDraw(lon,lat);
}
void myCentralWidget::twaDraw(double lon, double lat)
{
    if (mainW->getSelectedBoat()==NULL) return;
    if (!grib->isOk()) return;
    QPointF start(lon,lat);
    if(twaTrace==NULL)
        twaTrace=new DialogTwaLine(start,this, mainW);
    else
    {
        if (!twaTrace->isHidden()) return;
        twaTrace->setStart(start);
    }
    twaTrace->show();
    twaTrace->activateWindow();
}
void myCentralWidget::exportRouteFromMenu(ROUTE * route)
{
    QString routePath=Settings::getSetting("importRouteFolder","").toString();
    QDir dirRoute(routePath);
    if(!dirRoute.exists())
    {
        routePath=QApplication::applicationDirPath();
        Settings::setSetting("importRouteFolder",routePath);
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Exporter une Route"), routePath, "Routes (*.csv *.txt *.CSV *.TXT)");
    if(fileName.isEmpty() || fileName.isNull()) return;
    QMessageBox mb(0);
    mb.setText(tr("Exporter seulement les POIs ou egalement tous les details?"));
    mb.setWindowTitle(tr("Exporter une route"));
    mb.setIcon(QMessageBox::Question);
    QPushButton *POIbutton = mb.addButton(tr("POIs"),QMessageBox::YesRole);
    QPushButton *ALLbutton = mb.addButton(tr("Details"),QMessageBox::NoRole);
    mb.exec();
    bool POIonly=false;
    if(mb.clickedButton()==POIbutton)
        POIonly=true;
    else if(mb.clickedButton()==ALLbutton)
        POIonly=false;
    QFile::remove(fileName);
    QFile routeFile(fileName);
    if(!routeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(0,QObject::tr("Lecture de route"),
             QString(QObject::tr("Impossible de creer le fichier %1")).arg(fileName));
        return;
    }
    QFileInfo info(routeFile);
    Settings::setSetting("importRouteFolder",info.absoluteDir().path());
    QTextStream stream(&routeFile);
    QStringList list;
    list.append("position");
    list.append("heure");
    stream<<list.join(";")<<endl;
    if(route->getStartFromBoat())
    {
        list.clear();
        int deg = (int) fabs(route->getStartLat());
        float min = (fabs(route->getStartLat()) - deg)*60.0;
        const char *cdeg = "°";
        QString latitude;
        latitude.sprintf("%02d%s%06.3f", deg, cdeg, min);
        if(route->getStartLat()<0)
            latitude=latitude+" S";
        else
            latitude=latitude+" N";
        deg = (int) fabs(route->getStartLon());
        min = (fabs(route->getStartLon()) - deg)*60.0;
        QString longitude;
        longitude.sprintf("%03d%s%06.3f", deg, cdeg, min);
        if(route->getStartLon()<0)
            longitude=longitude+" W";
        else
            longitude=longitude+" E";
        list.append(latitude+"  "+longitude);
        QDateTime time;
        time.setTime_t(route->getStartDate());
        time=time.toUTC();
        time.setTimeSpec(Qt::UTC);
        list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
        stream<<list.join(";")<<endl;
    }
    if (POIonly)
    {
        QList<POI*> poiList=route->getPoiList();
        QListIterator<POI*> i(poiList);
        while(i.hasNext())
        {
            list.clear();
            POI * poi=i.next();

            int deg = (int) fabs(poi->getLatitude());
            float min = (fabs(poi->getLatitude()) - deg)*60.0;
            const char *cdeg = "°";
            QString latitude;
            latitude.sprintf("%02d%s%06.3f", deg, cdeg, min);
            if(poi->getLatitude()<0)
                latitude=latitude+" S";
            else
                latitude=latitude+" N";
            deg = (int) fabs(poi->getLongitude());
            min = (fabs(poi->getLongitude()) - deg)*60.0;
            QString longitude;
            longitude.sprintf("%03d%s%06.3f", deg, cdeg, min);
            if(poi->getLongitude()<0)
                longitude=longitude+" W";
            else
                longitude=longitude+" E";
            list.append(latitude+"  "+longitude);
            QDateTime time;
            time.setTime_t(poi->getRouteTimeStamp());
            time.toUTC();
            time.setTimeSpec(Qt::UTC);
            list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
            stream<<list.join(";")<<endl;
        }
    }
    else
    {
        QList<vlmPoint> *poiList=route->getLine()->getPoints();
        QListIterator<vlmPoint> i(*poiList);
        while(i.hasNext())
        {
            list.clear();
            vlmPoint poi=i.next();

            int deg = (int) fabs(poi.lat);
            float min = (fabs(poi.lat) - deg)*60.0;
            const char *cdeg = "°";
            QString latitude;
            latitude.sprintf("%02d%s%06.3f", deg, cdeg, min);
            if(poi.lat<0)
                latitude=latitude+" S";
            else
                latitude=latitude+" N";
            deg = (int) fabs(poi.lon);
            min = (fabs(poi.lon) - deg)*60.0;
            QString longitude;
            longitude.sprintf("%03d%s%06.3f", deg, cdeg, min);
            if(poi.lon<0)
                longitude=longitude+" W";
            else
                longitude=longitude+" E";
            list.append(latitude+"  "+longitude);
            QDateTime time;
            time.setTime_t(poi.eta);
            time.toUTC();
            time.setTimeSpec(Qt::UTC);
            list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
            stream<<list.join(";")<<endl;
        }
    }
    routeFile.close();
}
void myCentralWidget::slot_addRouteFromMenu()
{
    ROUTE * route=addRoute();
    slot_editRoute(route,true);
}
void myCentralWidget::slot_addRoutageFromMenu()
{
    ROUTAGE * routage=addRoutage();
    slot_editRoutage(routage,true);
}
void myCentralWidget::addPivot(ROUTAGE * fromRoutage,bool editOptions)
{
    ROUTAGE * routage=addRoutage();
    update_menuRoutage();
    routage->setFromRoutage(fromRoutage,editOptions);
}
ROUTE * myCentralWidget::addRoute()
{
    ROUTE * route=new ROUTE("Route", proj, grib, scene, this);
    route->setBoat(mainW->getSelectedBoat());
    connect(this,SIGNAL(updateRoute(boat *)),route,SLOT(slot_recalculate(boat *)));
    connect(mainW,SIGNAL(updateRoute(boat *)),route,SLOT(slot_recalculate(boat *)));
    connect(route,SIGNAL(editMe(ROUTE *)),this,SLOT(slot_editRoute(ROUTE *)));


    connect(this, SIGNAL(showALL(bool)),route,SLOT(slot_shShow()));
    connect(this, SIGNAL(hideALL(bool)),route,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shRou(bool)),route,SLOT(slot_shRou()));
    connect(this, SIGNAL(shRouBis()),route,SLOT(slot_shShow()));


    route_list.append(route);
    if(this->getPlayer()->getType()!=BOAT_REAL)
        route->setSpeedLossOnTack((double)Settings::getSetting("speedLossOnTackVlm","100").toInt()/100.00);
    else
        route->setSpeedLossOnTack((double)Settings::getSetting("speedLossOnTackReal","100").toInt()/100.00);

    return route;
}
ROUTAGE * myCentralWidget::addRoutage()
{
    nbRoutage++;
    QString rName;
    ROUTAGE * routage=new ROUTAGE(rName.sprintf(tr("Routage%d").toLocal8Bit(),nbRoutage), proj, grib, scene, this);
    routage->setBoat(mainW->getSelectedBoat());
    connect(routage,SIGNAL(editMe(ROUTAGE *)),this,SLOT(slot_editRoutage(ROUTAGE *)));


//    connect(this, SIGNAL(showALL(bool)),routage,SLOT(slot_shShow()));
//    connect(this, SIGNAL(hideALL(bool)),routage,SLOT(slot_shHidden()));
//    connect(this, SIGNAL(shRoutage(bool)),routage,SLOT(slot_shRou()));


    routage_list.append(routage);
    return routage;
}
void myCentralWidget::slot_editRoute(ROUTE * route,bool createMode)
{
    DialogRoute *route_editor=new DialogRoute(route,this);
    if(route_editor->exec()!=QDialog::Accepted)
    {
        if(createMode)
        {
            route_list.removeAll(route);
            delete route;
            route=NULL;
        }
        return;
    }
    else
    {
        update_menuRoute();
        QApplication::processEvents();
        route->slot_recalculate();
        if(route->getSimplify() && !route->isBusy())
        {
            bool detectCoast=route->getDetectCoasts();
            route->setDetectCoasts(false);
            route->setSimplify(false);
            if(route->getFrozen() || !route->getHas_eta())
                QMessageBox::critical(0,QString(QObject::tr("Simplification de route")),QString(QObject::tr("La simplification est impossible pour une route figee ou une route sans ETA")));
            else if(route->getUseVbvmgVlm())
                QMessageBox::critical(0,QString(QObject::tr("Simplification de route")),QString(QObject::tr("La simplification est impossible si le mode de calcul VBVMG est celui de VLM")));
            else
            {
                bool ok=false;
                int maxLoss=QInputDialog::getInteger(0,QString(QObject::tr("Simplication de route")),QString(QObject::tr("Perte maximum de temps sur l'ETA finale (en minutes)")),0,0,10000,1,&ok);
                if(ok)
                {
                    int firstPOI=1;
                    if(route->getStartFromBoat())
                        firstPOI=0;
                    QList<POI*> pois=route->getPoiList();
                    int ref_nbPois=pois.count();
                    time_t ref_eta=route->getEta();
                    int nbDel=0;
                    QProgressDialog p(tr("Simplification en cours"),"",1,ref_nbPois-2);
                    p.setCancelButton(0);
                    int phase=1;
                    p.setLabelText(tr("Phase ")+QString().setNum(phase));
                    bool notFinished=true;
                    time_t bestEta=ref_eta;
                    while(notFinished)
                    {
                        notFinished=false;
                        pois=route->getPoiList();
                        p.setValue(0);
                        p.setMaximum(pois.count()-2);
                        for (int n=firstPOI;n<pois.count()-2;n++)
                        {
                            POI *poi=pois.at(n);
                            if(poi->getNotSimplificable()) continue;
                            poi->setRoute(NULL);
                            QApplication::processEvents();
                            if(!route->getHas_eta())
                                poi->setRoute(route);
                            else if(route->getEta()<=bestEta)
                            {
                                bestEta=route->getEta();
                                notFinished=true;
                                slot_delPOI_list(poi);
                                delete poi;
                                nbDel++;
                            }
                            else
                                poi->setRoute(route);
                            p.setValue(n);
                            QApplication::processEvents();
                        }
                        //if(!notFinished) break;
                        ++phase;
                        p.setLabelText(tr("Phase ")+QString().setNum(phase));
                        pois=route->getPoiList();
                        p.setValue(pois.count()-2);
                        p.setMaximum(pois.count()-2);
                        for (int n=pois.count()-2;n>=firstPOI;n--)
                        {
                            POI *poi=pois.at(n);
                            if(poi->getNotSimplificable()) continue;
                            poi->setRoute(NULL);
                            QApplication::processEvents();
                            if(!route->getHas_eta())
                                poi->setRoute(route);
                            else if(route->getEta()<=bestEta)
                            {
                                bestEta=route->getEta();
                                notFinished=true;
                                slot_delPOI_list(poi);
                                delete poi;
                                nbDel++;
                            }
                            else
                                poi->setRoute(route);
                            p.setValue(n);
                            QApplication::processEvents();
                        }
                        pois=route->getPoiList();
                        ++phase;
                        p.setLabelText(tr("Phase ")+QString().setNum(phase));
                        p.setValue(0);
                        p.setMaximum(pois.count()-2);

                        for (int n=firstPOI;n<pois.count()-3;++n)
                        {
                            POI *poi1=pois.at(n);
                            if(poi1->getNotSimplificable()) continue;
                            POI *poi2=pois.at(n+1);
                            if(poi2->getNotSimplificable()) continue;
                            route->setTemp(true);
                            poi1->setRoute(NULL);
                            route->setTemp(false);
                            poi2->setRoute(NULL);
                            QApplication::processEvents();
                            if(!route->getHas_eta())
                            {
                                route->setTemp(true);
                                poi1->setRoute(route);
                                route->setTemp(false);
                                poi2->setRoute(route);
                            }
                            else if(route->getEta()<=ref_eta+maxLoss*60)
                            {
                                bestEta=route->getEta();
                                notFinished=true;
                                slot_delPOI_list(poi1);
                                delete poi1;
                                slot_delPOI_list(poi2);
                                delete poi2;
                                nbDel=nbDel+2;
                                n=firstPOI-1;
                                pois=route->getPoiList();
                                p.setValue(0);
                                p.setMaximum(pois.count()-2);
                                continue;
                            }
                            else
                            {
                                route->setTemp(true);
                                poi1->setRoute(route);
                                route->setTemp(false);
                                poi2->setRoute(route);
                            }
                            p.setValue(n);
                            QApplication::processEvents();
                        }

                        ++phase;
                        p.setLabelText(tr("Phase ")+QString().setNum(phase));
                        pois=route->getPoiList();
                        p.setValue(0);
                        p.setMaximum(pois.count()-2);

                        for (int n=firstPOI;n<pois.count()-4;++n)
                        {
                            POI *poi1=pois.at(n);
                            if(poi1->getNotSimplificable()) continue;
                            POI *poi2=pois.at(n+1);
                            if(poi2->getNotSimplificable()) continue;
                            POI *poi3=pois.at(n+2);
                            if(poi3->getNotSimplificable()) continue;
                            route->setTemp(true);
                            poi1->setRoute(NULL);
                            poi2->setRoute(NULL);
                            route->setTemp(false);
                            poi3->setRoute(NULL);
                            QApplication::processEvents();
                            if(!route->getHas_eta())
                            {
                                route->setTemp(true);
                                poi1->setRoute(route);
                                poi2->setRoute(route);
                                route->setTemp(false);
                                poi3->setRoute(route);
                            }
                            else if(route->getEta()<=ref_eta+maxLoss*60)
                            {
                                bestEta=route->getEta();
                                notFinished=true;
                                slot_delPOI_list(poi1);
                                delete poi1;
                                slot_delPOI_list(poi2);
                                delete poi2;
                                slot_delPOI_list(poi3);
                                delete poi3;
                                nbDel=nbDel+3;
                                n=firstPOI-1;
                                pois=route->getPoiList();
                                p.setValue(0);
                                p.setMaximum(pois.count()-2);
                                continue;
                            }
                            else
                            {
                                route->setTemp(true);
                                poi1->setRoute(route);
                                poi2->setRoute(route);
                                route->setTemp(false);
                                poi3->setRoute(route);
                            }
                            p.setValue(n);
                            QApplication::processEvents();
                        }


                    }

                    ++phase;
                    p.setLabelText(tr("Phase ")+QString().setNum(phase));
                    pois=route->getPoiList();
                    p.setValue(0);
                    p.setMaximum(pois.count()-2);

                    if(maxLoss!=0)
                    {
                        for (int n=firstPOI;n<pois.count()-2;n++)
                        {
                            POI *poi=pois.at(n);
                            if(poi->getNotSimplificable()) continue;
                            poi->setRoute(NULL);
                            QApplication::processEvents();
                            if(!route->getHas_eta())
                                poi->setRoute(route);
                            else if(route->getEta()<=ref_eta+maxLoss*60)
                            {
                                slot_delPOI_list(poi);
                                delete poi;
                                nbDel++;
                            }
                            else
                                poi->setRoute(route);
                            p.setValue(n);
                            QApplication::processEvents();
                        }
                    }
                    p.close();
                    int diff=(ref_eta-route->getEta())/60;
                    QString result;
                    if(diff<=0)
                        result=QString().setNum(-diff)+tr(" minutes perdues, ")+
                               QString().setNum(nbDel)+tr(" POIs supprimes sur ")+
                               QString().setNum(ref_nbPois);
//                        result=result.sprintf(tr("%d minutes perdues, %d POIs supprimes sur %d").toStdString(),-diff,nbDel,ref_nbPois);
                    else
                        result=QString().setNum(diff)+tr(" minutes gagnees(!), ")+
                               QString().setNum(nbDel)+tr(" POIs supprimes sur ")+
                               QString().setNum(ref_nbPois);
//                        result=result.sprintf("%d minutes gagnees(!), %d POIs supprimes sur %d",diff,nbDel,ref_nbPois);
                    QDateTime before;
                    before=before.fromTime_t(ref_eta);
                    before=before.toUTC();
                    before.setTimeSpec(Qt::UTC);
                    QDateTime after;
                    after=after.fromTime_t(route->getEta());
                    after=after.toUTC();
                    after.setTimeSpec(Qt::UTC);
                    result=result+"<br>"+tr("ETA avant simplification: ")+before.toString("dd/MM/yy hh:mm:ss");
                    result=result+"<br>"+tr("ETA apres simplification: ")+after.toString("dd/MM/yy hh:mm:ss");
                    QMessageBox::information(0,QString(QObject::tr("Resultat de la simplification")),result);
                }
            }
            route->setDetectCoasts(detectCoast);
        }
    }
    //route->slot_recalculate();
    delete route_editor;
    if(route->getPilototo())
    {
        if(!route->getStartFromBoat() || route->getStartTimeOption()!=1 || !route->getUseVbvmgVlm())
        {
            QMessageBox::critical(0,tr("Envoyer la route au pilototo"),tr("Pour pouvoir envoyer la route au pilototo if faut que:<br>-La route demarre du bateau et de la prochaine vac<br>et que le mode VbVmg-Vlm soit active"));
            return;
        }
        mainW->setPilototoFromRoute(route);

    }
}
void myCentralWidget::setPilototo(QList<POI *> poiList)
{
    if(!poiList.isEmpty())
    {
        mainW->setPilototoFromRoute(poiList);
    }
}

void myCentralWidget::slot_editRoutage(ROUTAGE * routage,bool createMode)
{
    DialogRoutage *routage_editor=new DialogRoutage(routage,this);
    if(routage_editor->exec()!=QDialog::Accepted)
    {
        if(createMode || routage->getIsNewPivot())
        {
            delete routage;
            routage_list.removeAll(routage);
            routage=NULL;
            delete routage_editor;
            nbRoutage--;
        }
    }
    else
    {
        delete routage_editor;
        update_menuRoutage();
        if(createMode || routage->getIsNewPivot())
            routage->calculate();
        if(routage->isDone() && routage->isConverted())
            deleteRoutage(routage);
    }
}
void myCentralWidget::deleteRoute(ROUTE * route)
{
    route_list.removeAll(route);
    update_menuRoute();
    delete route;
}
void myCentralWidget::slot_deleteRoute()
{
    QAction *sender=(QAction*)QObject::sender();
    ROUTE *route=reinterpret_cast<struct ROUTE *>(qvariant_cast<void*>(sender->data()));
    if(!route || route==NULL) return;
    myDeleteRoute(route);
}
void myCentralWidget::myDeleteRoute(ROUTE * route)
{
    if(route->isBusy()) return ;
    if(route->getFrozen())
    {
        QMessageBox::critical(0,
            tr("Suppression d'une route"),
            tr("Vous ne pouvez pas supprimer une route figee"));
        return ;
    }
    int rep = QMessageBox::question (0,
            tr("Detruire la route : %1").arg(route->getName()),
            tr("La destruction d'une route est definitive.\n\nVoulez-vous egalement supprimer tous les POIs lui appartenant?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (rep == QMessageBox::Cancel) return ;
    route->setTemp(true);
//    route->setHidden(false);
//    QCoreApplication::sendPostedEvents(scene,0);
//    QCoreApplication::processEvents();
//    QCoreApplication::flush();
    while(!route->getPoiList().isEmpty())
    {
        POI * poi = route->getPoiList().takeFirst();
        poi->setRoute(NULL);
        poi->setMyLabelHidden(false);
        if(rep==QMessageBox::Yes)
        {
            slot_delPOI_list(poi);
            delete poi;
        }
    }
    deleteRoute(route);
}
void myCentralWidget::slot_deleteRoutage()
{
    QAction *sender=(QAction*)QObject::sender();
    ROUTAGE *routage=reinterpret_cast<struct ROUTAGE *>(qvariant_cast<void*>(sender->data()));
    if(!routage || routage==NULL) return;
    if(routage->isRunning()) return;
    int rep = QMessageBox::question (0,
            tr("Detruire le routage : %1?").arg(routage->getName()),
            tr("La destruction d'un routage est definitive."),
            QMessageBox::Yes | QMessageBox::Cancel);
    if (rep == QMessageBox::Cancel) return;
    deleteRoutage(routage);
}

void myCentralWidget::deleteRoutage(ROUTAGE * routage)
{
    if(routage)
    {
        routage_list.removeAll(routage);
        update_menuRoutage();
        delete(routage);
    }
}
void myCentralWidget::assignPois()
{
    qWarning() << "AssignPOI "  << route_list.count() << " routes, " << poi_list.count() << " pois";
    QList<bool> frozens;
    QListIterator<ROUTE*> r (route_list);
    while(r.hasNext())
    {
        ROUTE * route=r.next();
        frozens.append(route->getFrozen());
        route->setFrozen(true);
    }
    QListIterator<POI*> i (poi_list);
    while(i.hasNext())
    {
        POI * poi=i.next();
        if(poi->getRouteName()!="")
        {
            bool found=false;
            QListIterator<ROUTE*> j (route_list);
            while(j.hasNext())
            {
                ROUTE * route=j.next();
                if(poi->getRouteName() == route->getName())
                {
                    poi->setRoute(route);
                    found=true;
                    break;
                }
            }
            if(!found)
                poi->setRoute(NULL);
         }
    }
    update_menuRoute();
    r.toFront();
    int n=0;
    while(r.hasNext())
    {
        ROUTE * route=r.next();
        if(frozens.at(n))
        {
            route->setFrozen(false);
        }
        route->setFrozen(frozens.at(n));
        n++;
    }
}


void myCentralWidget::update_menuRoute()
{
    qSort(route_list.begin(),route_list.end(),ROUTE::myLessThan);
    menuBar->mnRoute_edit->clear();
    menuBar->mnRoute_delete->clear();
    menuBar->mnRoute_export->clear();
    menuBar->mnCompassCenterRoute->clear();
    QAction * a=menuBar->addReleaseCompass();
    connect(a, SIGNAL(triggered()), this, SLOT(slot_releaseCompassFollow()));
    QListIterator<ROUTE*> i (route_list);
    while(i.hasNext())
    {
        menuBar->addMenuRoute(i.next());
    }
}
void myCentralWidget::update_menuRoutage()
{
    qSort(routage_list.begin(),routage_list.end(),ROUTAGE::myLessThan);
    menuBar->mnRoutage_edit->clear();
    menuBar->mnRoutage_delete->clear();
    QListIterator<ROUTAGE*> i (routage_list);
    while(i.hasNext())
        menuBar->addMenuRoutage(i.next());
}
float myCentralWidget::A360(float hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

/**************************/
/* Players                */
/**************************/
void myCentralWidget::slot_addPlayer_list(Player* player)
{
    player_list.append(player);
    connect(player,SIGNAL(addBoat(boat*)),this,SLOT(slot_addBoat(boat*)));
    connect(player,SIGNAL(delBoat(boat*)),this,SLOT(slot_delBoat(boat*)));
}

void myCentralWidget::slot_delPlayer_list(Player* player)
{
    player_list.removeAll(player);
    disconnect(player,SIGNAL(addBoat(boat*)),this,SLOT(slot_addBoat(boat*)));
    disconnect(player,SIGNAL(delBoat(boat*)),this,SLOT(slot_delBoat(boat*)));
}

/**************************/
/* Boats                  */
/**************************/

void myCentralWidget::slot_addBoat(boat* boat)
{
    //boat_list.append(boat);
    scene->addItem(boat);
    connect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    connect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    connect(boat,SIGNAL(getTrace(QByteArray,QList<vlmPoint> *)),opponents,SLOT(getTrace(QByteArray,QList<vlmPoint> *)));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
    boat->slot_paramChanged();
}

void myCentralWidget::slot_delBoat(boat* boat)
{
    //boat_list.removeAll(boat);
    scene->removeItem(boat);
    disconnect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    disconnect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    disconnect(boat,SIGNAL(getTrace(QByteArray,QList<vlmPoint> *)),opponents,SLOT(getTrace(QByteArray,QList<vlmPoint> *)));
    disconnect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
}

void myCentralWidget::slot_boatDialog(void)
{
    if(currentPlayer)
    {
        if(currentPlayer->getType()==BOAT_VLM)
        {
            if(boatAcc->initList(boat_list,currentPlayer))
                boatAcc->exec();
        }
        else
            realBoatConfig->launch(realBoat);
    }

}

void myCentralWidget::slot_moveBoat(double lat, double lon)
{
    if(currentPlayer && currentPlayer->getType()==BOAT_REAL)
    {
        realBoat->setPosition(lat,lon);
        realBoat->emitMoveBoat();
    }
}

void myCentralWidget::slot_manageAccount()
{
    manageAccount();
}

void myCentralWidget::manageAccount(bool * res)
{
    /* managing previous account */
    if(currentPlayer && boat_list)
    {
        if(currentPlayer->getType()==BOAT_VLM)
        {
            QListIterator<boatVLM*> i(*boat_list);
            while(i.hasNext())
            {
                boatVLM * acc=i.next();
                acc->playerDeActivated();
                acc->setInitialized(false);
            }
        }
    }
    else if(currentPlayer && currentPlayer->getType()!=BOAT_VLM)
        realBoat->playerDeActivated();

    playerAcc->initList(&player_list);
    int tmp_res= playerAcc->exec();
    if(res)
        *res=(tmp_res == QDialog::Accepted);

}

void myCentralWidget::updatePlayer(Player * player)
{
    playerAcc->doUpdate(player);
}

void myCentralWidget::slot_playerSelected(Player * player)
{
    if(currentPlayer && boat_list)
    {
        //qWarning() << "Deactivate current player";
        if(currentPlayer->getType()==BOAT_VLM)
        {
            QListIterator<boatVLM*> i(*boat_list);
            while(i.hasNext())
            {
                i.next()->playerDeActivated();
            }
        }
        else
            realBoat->playerDeActivated();
        //qWarning() << "Deactivate current player ==> done";
    }

    currentPlayer = player;
    if(player)
    {
        if(player->getType() == BOAT_VLM)
        {
            //qWarning() << "Activate player: managing boats (VLM)";
            menuBar->boatList->setVisible(true);
            menuBar->ac_moveBoat->setVisible(false);
            menuBar->ac_moveBoatSep->setVisible(false);
            menuBar->acRace->setVisible(true);
            menuBar->acVLMSync->setVisible(true);
            menuBar->acPilototo->setVisible(true);
            menuBar->acShowPolar->setVisible(true);
            //menuBar->acVLMParamBoat->setEnabled(true);
            boat_list=player->getBoats();
            QListIterator<boatVLM*> i(*boat_list);
            bool reselected=false;
            int thisOne=0;
            int nn=-1;
            while(i.hasNext())
            {
                boatVLM * boat=i.next();
                nn++;
                if(boat->getPlayer()!=player) continue;
                boat->playerActivated();
                if(!reselected && boat->getStatus())
                {
                    thisOne=nn;
                    reselected=true;
                }
                //boat->setStatus(boat->getStatus());
            }
            realBoat=NULL;
            emit accountListUpdated();
            mainW->getBoard()->playerChanged(player);
            //qWarning()<<"reselected="<<reselected;
            if(reselected)
            {
                mainW->slotSelectBoat(boat_list->at(thisOne));
                boat_list->at(thisOne)->setSelected(true);
                mainW->getBoard()->boatUpdated(boat_list->at(thisOne));
            }
            emit shRouBis();
        }
        else
        {
            menuBar->boatList->setVisible(false);
            menuBar->ac_moveBoat->setVisible(true);
            menuBar->ac_moveBoatSep->setVisible(true);
            menuBar->acRace->setVisible(false);
            menuBar->acVLMSync->setVisible(false);
            menuBar->acPilototo->setVisible(false);
            menuBar->acShowPolar->setVisible(true);
            //menuBar->acVLMParamBoat->setEnabled(false);
            realBoat=player->getRealBoat();
            realBoat->reloadPolar();
            mainW->slotSelectBoat(realBoat);
            realBoat->playerActivated();
            mainW->getBoard()->playerChanged(player);
            mainW->getBoard()->boatUpdated(realBoat);
            mainW->slotBoatUpdated(realBoat,true,false);;
            emit shRouBis();
            menuBar->insertBoatReal(realBoat->getplayerName());
        }
    }
    else
    {
        menuBar->boatList->setVisible(false);
        //menuBar->acVLMParamBoat->setEnabled(false);
        boat_list=NULL;
        realBoat=NULL;
        emit shRouBis();
    }

}

void myCentralWidget::slot_writeBoatData(void)
{
    emit writeBoatData(player_list,race_list,QString("boatAcc.dat"));
}

void myCentralWidget::slot_readBoatData(void)
{
    // on vide la liste

    emit readBoatData("boatAcc.dat",true);
}

/**************************/
/* Races                  */
/**************************/

void myCentralWidget::slot_addRace_list(raceData* race)
{
    race_list.append(race);
}

void myCentralWidget::slot_delRace_list(raceData* race)
{
    race_list.removeAll(race);
}

void myCentralWidget::slot_raceDialog(void)
{
    raceDialog->initList(*boat_list,race_list);
}

void myCentralWidget::slot_readRaceData(void)
{
    /* clean race list */
    while(race_list.count()!=0)
    {
        raceData* ptr = race_list.last();
        slot_delRace_list(ptr);
        delete ptr;
    }
    emit readBoatData("boatAcc.dat",false);
}

/**************************/
/* Menu slot              */
/**************************/
void myCentralWidget::slot_clearSelection(void)
{
    selection->clearSelection();
}

void myCentralWidget::slot_map_CitiesNames()
{
    MenuBar  *mb = menuBar;
    QAction *act = mb->acMap_GroupCitiesNames->checkedAction();

    if (act == mb->acMap_CitiesNames0)
        terre->setCitiesNamesLevel(0);
    else if (act == mb->acMap_CitiesNames1)
        terre->setCitiesNamesLevel(1);
    else if (act == mb->acMap_CitiesNames2)
        terre->setCitiesNamesLevel(2);
    else if (act == mb->acMap_CitiesNames3)
        terre->setCitiesNamesLevel(3);
    else if (act == mb->acMap_CitiesNames4)
        terre->setCitiesNamesLevel(4);
}

void myCentralWidget::slotIsobarsStep()
{
    int s = 4;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acView_GroupIsobarsStep->checkedAction();
    if (act == mb->acView_IsobarsStep1)
        s = 1;
    else if (act == mb->acView_IsobarsStep2)
        s = 2;
    else if (act == mb->acView_IsobarsStep3)
        s = 3;
    else if (act == mb->acView_IsobarsStep4)
        s = 4;
    else if (act == mb->acView_IsobarsStep5)
        s = 5;
    else if (act == mb->acView_IsobarsStep6)
        s = 6;
    else if (act == mb->acView_IsobarsStep8)
        s = 8;
    else if (act == mb->acView_IsobarsStep10)
        s = 10;
    terre->setIsobarsStep(s);
}

void myCentralWidget::slotIsotherms0Step()
{
    int s = 100;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acView_GroupIsotherms0Step->checkedAction();
    if (act == mb->acView_Isotherms0Step10)
        s = 10;
    else if (act == mb->acView_Isotherms0Step20)
        s = 20;
    else if (act == mb->acView_Isotherms0Step50)
        s = 50;
    else if (act == mb->acView_Isotherms0Step100)
        s = 100;
    else if (act == mb->acView_Isotherms0Step200)
        s = 200;
    else if (act == mb->acView_Isotherms0Step500)
        s = 500;
    else if (act == mb->acView_Isotherms0Step1000)
        s = 1000;

    terre->setIsotherms0Step(s);
}

void myCentralWidget::slot_setColorMapMode(QAction* act)
{
    MenuBar  *mb = menuBar;
    int mode;
    if (act == mb->acView_WindColors)
        mode = Terrain::drawWind;
    else if (act == mb->acView_RainColors)
        mode = Terrain::drawRain;
    else if (act == mb->acView_CloudColors)
        mode = Terrain::drawCloud;
    else if (act == mb->acView_HumidColors)
        mode = Terrain::drawHumid;
    else if (act == mb->acView_TempColors)
        mode = Terrain::drawTemp;
    else if (act == mb->acView_TempPotColors)
        mode = Terrain::drawTempPot;
    else if (act == mb->acView_DeltaDewpointColors)
        mode = Terrain::drawDeltaDewpoint;
    else if (act == mb->acView_SnowCateg)
        mode = Terrain::drawSnowCateg;
    else if (act == mb->acView_FrzRainCateg)
        mode = Terrain::drawFrzRainCateg;
    else if (act == mb->acView_SnowDepth)
        mode = Terrain::drawSnowDepth;
    else if (act == mb->acView_CAPEsfc)
        mode = Terrain::drawCAPEsfc;
    else
        mode = Terrain::drawNone;

    //qWarning() << "New mode " << mode;

    terre->setColorMapMode(mode);
}

/**************************/
/* Menu slot              */
/**************************/

void myCentralWidget::slot_POISave(void)
{
    emit writePOIData(route_list,poi_list,"poi.dat");
    QMessageBox::information(this,tr("Sauvegarde des POIs et des routes"),tr("Sauvegarde reussie"));
}
void myCentralWidget::slot_POIRestore(void)
{
    while(!route_list.isEmpty())
    {
        route_list.first()->setTemp(true);
        QListIterator<POI*> i (route_list.first()->getPoiList());
        while(i.hasNext())
        {
            POI * poi = i.next();
            poi->setRoute(NULL);
            poi->setMyLabelHidden(false);
            slot_delPOI_list(poi);
            delete poi;
        }
        deleteRoute(route_list.first());
    }
    while (!poi_list.isEmpty())
    {
        POI * poi=poi_list.first();
        slot_delPOI_list(poi);
        delete poi;
    }
    loadPOI();
    QMessageBox::information(this,tr("Chargement des POIs et des routes"),tr("Chargement reussi"));
}

void myCentralWidget::slot_POIimport(void)
{
    emit importZyGrib();
}

/**************************/
/* Selection Widget       */
/**************************/

bool myCentralWidget::isSelecting(void)
{
    double a,b,c,d;
    return selection->getZone(&a,&b,&c,&d);
}
void myCentralWidget::drawNSZ(int i)
{
    if (NSZ!=NULL)
    {
        delete NSZ;
        NSZ=NULL;
    }
    if(i==-1) return;
    if(race_list[i]->displayNSZ)
    {
        NSZ=new vlmLine(proj,this->scene,Z_VALUE_GATE);
        QPen pen;
        pen.setColor(race_list[i]->colorNSZ);
        pen.setBrush(race_list[i]->colorNSZ);
        pen.setWidthF(race_list[i]->widthNSZ);
        NSZ->setLinePen(pen);
        for (int j=-180;j<361;j++)
            NSZ->addPoint(race_list[i]->latNSZ,j);
        NSZ->slot_showMe();
        qWarning()<<"drawing NSZ";
    }
}
void myCentralWidget::removeOpponent(QString oppId, QString raceId)
{
    foreach(raceData* race,race_list)
    {
        if(race->idrace==raceId)
        {
            QStringList op=race->oppList.split(";");
            op.removeAll(oppId);
            race->oppList=op.join(";");
            break;
        }
    }
}
void myCentralWidget::slot_shFla(bool)
{
    //qWarning()<<"key F pressed";
    int f=Settings::getSetting("showFlag",0).toInt();
    if(f==0) f=1;
    else f=0;
    Settings::setSetting("showFlag",f);
    emit shFla();
}
