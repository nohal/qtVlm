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

#include "mycentralwidget.h"
#include "poi_delete.h"
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
#include "dialog_gribDate.h"
#include "POI_editor.h"
#include "POI.h"
#include "boatVLM.h"
#include "race_dialog.h"
#include "boatAccount_dialog.h"
#include "playerAccount.h"
#include "DialogLoadGrib.h"
#include "xmlBoatData.h"
#include "xmlPOIData.h"
#include "routage.h"
#include "Route_Editor.h"
#include "Routage_Editor.h"
#include "BoardVLM.h"
#include "vlmLine.h"
#include "dataDef.h"
#include "Util.h"
#include "dialoghorn.h"
#include "twaline.h"
#include "Player.h"

/*******************/
/*    myScene      */
/*******************/

myScene::myScene(myCentralWidget * parent) : QGraphicsScene(parent)
{
    this->parent = parent;
}

/* Events */

void  myScene::keyPressEvent (QKeyEvent *e)
{
    QString position;
    QStringList positions;
    switch(e->key())
    {
        case Qt::Key_Minus:
        case Qt::Key_M:
            parent->slot_Zoom_Out();
            break;
        case Qt::Key_Plus:
        case Qt::Key_P:
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
                position.sprintf("%.0f ; %.0f ; %.0f",parent->getProj()->getScale(),parent->getProj()->getCX(),
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
                position.sprintf("%.0f ; %.0f ; %.0f",parent->getProj()->getScale(),parent->getProj()->getCX(),
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
                position.sprintf("%.0f ; %.0f ; %.0f",parent->getProj()->getScale(),parent->getProj()->getCX(),
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
                position.sprintf("%.0f ; %.0f ; %.0f",parent->getProj()->getScale(),parent->getProj()->getCX(),
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
    parent->mouseMove(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos()));
}

void myScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::LeftButton)
        parent->mouseDoubleClick(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos()));
}
/*******************/
/* myCentralWidget */
/*******************/

myCentralWidget::myCentralWidget(Projection * proj,MainWindow * parent,MenuBar * menuBar) : QWidget(parent)
{
    this-> proj=proj;
    this->main=parent;
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
    scene->setSceneRect(QRect(0,0,width(),height()));

    view = new QGraphicsView(scene, this);
    view->setGeometry(0,0,width(),height());

    /* other child */
    gshhsReader = new GshhsReader("maps/gshhs", 0);
    grib = new Grib();
    inetManager = new inetConnexion(main);

    /* item child */
    // Terre
    terre = new Terrain(this,proj);
    terre->setGSHHS_map(gshhsReader);
    terre->setCitiesNamesLevel(Settings::getSetting("showCitiesNamesLevel", 0).toInt());

    #warning voir s il faut mettre le slot ds centralWidget ou utiliser myScene
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

    connect(menuBar->acOptions_SH_Lab, SIGNAL(triggered(bool)), this,  SIGNAL(shLab(bool)));
    connect(menuBar->acOptions_SH_Lab, SIGNAL(triggered(bool)), this,  SLOT(slot_shLab(bool)));

    connect(menuBar->acOptions_SH_Com, SIGNAL(triggered(bool)), this,  SIGNAL(shCom(bool)));

    connect(menuBar->acOptions_SH_Pol, SIGNAL(triggered(bool)), this,  SIGNAL(shPol(bool)));

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
    connect(parent,SIGNAL(showCompassLine(int,int)),compass,SLOT(slot_compassLine(int,int)));
    connect(parent,SIGNAL(showCompassCenterBoat()),compass,SLOT(slot_compassCenterBoat()));
    connect(parent,SIGNAL(showCompassCenterWp()),compass,SLOT(slot_compassCenterWp()));
    connect(parent,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(selectedBoatChanged()),compass,SLOT(slot_paramChanged()));
    connect(scene,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(boatHasUpdated(boat*)),compass,SLOT(slot_paramChanged()));
    connect(this, SIGNAL(showALL(bool)),compass,SLOT(slot_paramChanged()));
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
    opponents = new opponentList(proj,main,this,inetManager);

     /* Dialogs */
    gribDateDialog = new dialog_gribDate();
    poi_editor=new POI_Editor(parent,this);

    boatAcc = new boatAccount_dialog(proj,parent,this,inetManager);    
    playerAcc = new playerAccount(proj,main,this,inetManager);

    raceParam = new race_dialog(parent,this,inetManager);
    connect(raceParam,SIGNAL(readRace()),this,SLOT(slot_readRaceData()));
    connect(raceParam,SIGNAL(writeBoat()),this,SLOT(slot_writeBoatData()));

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
    emitUpdateRoute();
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
    xmlPOI->slot_writeData(route_list,poi_list,"poi.dat");
    xmlData->slot_writeData(player_list,race_list,QString("boatAcc.dat"));
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
    if(!compass || !compass->isVisible())
        return COMPASS_NOTHING;

    if(compass->hasCompassLine())
        return COMPASS_LINEON;
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
    return main->getSelectedBoat();
}

/*******************/
/* Zoom and move   */
/*******************/

void myCentralWidget::slot_Zoom_All()
{
    proj->zoomAll();
}

void myCentralWidget::slot_Go_Left()
{
    proj->move( 0.2, 0);
}

void myCentralWidget::slot_Go_Right()
{
    proj->move(-0.2, 0);
}

void myCentralWidget::slot_Go_Up()
{
    proj->move(0,  -0.2);
}

void myCentralWidget::slot_Go_Down()
{
    proj->move(0,  0.2);
}

void myCentralWidget::slot_Zoom_In()
{
    proj->zoom(1.3);
}

void myCentralWidget::slot_Zoom_Out()
{
    proj->zoom(1/1.3);
}

void myCentralWidget::slot_Zoom_Sel()
{
    double x0, y0, x1, y1;
    if (selection->getZone(&x0,&y0, &x1,&y1))
    {
        proj->zoomOnZone(x0,y0,x1,y1);
        selection->clearSelection();

    }
    else
    {
        zoomOnGrib();
    }
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
        main->statusBar_showSelectedZone(xa,ya,xb,yb);
    }
    else
    {
        double xx, yy;
        proj->screen2map(x,y, &xx, &yy);
        main->statusBar_showWindData(xx, yy);
        main->drawVacInfo();
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
    slot_addPOI("",POI_TYPE_POI,(float)lat,(float)lon,-1,-1,false,main->getSelectedBoat());
}

void myCentralWidget::escapeKeyPressed(void)
{
    emit stopCompassLine();
    emit POI_selectAborted(NULL);
    selection->clearSelection();
    horn->stop();
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
        mh = fabs(x0-x1)*0.05;
        mv = fabs(y0-y1)*0.05;
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
        zoomOnGrib();
    else
        emit redrawAll();
}

void myCentralWidget::setCurrentDate(time_t t)
{
    if (grib->getCurrentDate() != t)
    {
        grib->setCurrentDate(t);
        emit redrawGrib();
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
        emit updateRoute();
    }
    else
    {
        QMessageBox::warning (this,
            tr("Téléchargement d'un fichier GRIB"),
            tr("Vous devez sélectionner une zone de la carte."));
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
                                return tr("oui (calculé par la formule de Magnus-Tetens)");
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
            tr("Aucun fichir GRIB n'est chargé."));
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
        msg += tr("Données disponibles :\n");
	msg += tr("    Température : %1\n").arg(dataPresentInGrib(grib,GRB_TEMP,LV_ABOV_GND,2));
	msg += tr("    Pression : %1\n").arg(dataPresentInGrib(grib,GRB_PRESSURE,LV_MSL,0));
	msg += tr("    Vent  : %1\n").arg(dataPresentInGrib(grib,GRB_WIND_VX,LV_ABOV_GND,10));
	msg += tr("    Cumul de précipitations : %1\n").arg(dataPresentInGrib(grib,GRB_PRECIP_TOT,LV_GND_SURF,0));
	msg += tr("    Nébulosité : %1\n").arg(dataPresentInGrib(grib,GRB_CLOUD_TOT,LV_ATMOS_ALL,0));
	msg += tr("    Humidité relative : %1\n").arg(dataPresentInGrib(grib,GRB_HUMID_REL,LV_ABOV_GND,2));
	msg += tr("    Isotherme 0°C : %1\n").arg(dataPresentInGrib(grib,GRB_GEOPOT_HGT,LV_ISOTHERM0,0));
	msg += tr("    Point de rosée : %1\n").arg(dataPresentInGrib(grib,GRB_DEWPOINT,LV_ABOV_GND,2));
	msg += tr("    Température (min) : %1\n").arg(dataPresentInGrib(grib,GRB_TMIN,LV_ABOV_GND,2));
	msg += tr("    Température (max) : %1\n").arg(dataPresentInGrib(grib,GRB_TMAX,LV_ABOV_GND,2));
        msg += tr("    Température (pot) : %1\n").arg(dataPresentInGrib(grib,GRB_TEMP_POT,LV_SIGMA,9950));
	msg += tr("    Neige (risque) : %1\n").arg(dataPresentInGrib(grib,GRB_SNOW_CATEG,LV_GND_SURF,0));
	msg += tr("    Neige (épaisseur) : %1\n").arg(dataPresentInGrib(grib,GRB_SNOW_DEPTH,LV_GND_SURF,0));
        msg += tr("    Humidité spécifique :\n");
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
        msg += tr("Date de référence : %1\n")
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
    if(boat==NULL) boat=main->getSelectedBoat();
    poi = new POI(name,type,lat,lon, proj,
                  main, this,wph,timestamp,useTimeStamp,boat);

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
    connect(this, SIGNAL(shLab(bool)),poi,SLOT(slot_shLab()));
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

        while(i.hasNext())
        {
            POI * poi = i.next();
            if(poi->getRoute()!=NULL)
                if(poi->getRoute()->getFrozen()) continue;
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                slot_delPOI_list(poi);
                delete poi;
            }

        }
        selection->clearSelection();
    }
}

void myCentralWidget::slot_delSelPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        int res_mask;
        POI_delete * dialog_sel = new POI_delete();
        dialog_sel->exec();
        if((res_mask=dialog_sel->getResult())<0)
            return;

        QListIterator<POI*> i (poi_list);

        while(i.hasNext())
        {
            POI * poi = i.next();
            if(poi->getRoute()!=NULL)
                if(poi->getRoute()->getFrozen()) continue;
//            qWarning() << "POI: " << poi->getName() << " mask=" << poi->getTypeMask();
            if(!(poi->getTypeMask() & res_mask))
                continue;
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                slot_delPOI_list(poi);
                delete poi;
            }
        }
        selection->clearSelection();
    }
}

void myCentralWidget::slot_showALL(bool)
{
    shLab_st=false;
    shPoi_st=false;
    shRoute_st=false;
    shOpp_st=false;
    shPor_st=false;
}

void myCentralWidget::slot_hideALL(bool)
{
    shLab_st=true;
    shPoi_st=true;
    shRoute_st=true;
    shOpp_st=true;
    shPor_st=true;
}

void myCentralWidget::slot_shLab(bool)
{
       shLab_st=!shLab_st;
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
        if (route->getName()==name && route!=thisroute) return false;
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
                         tr("Ouvrir un fichier Route"), routePath, "Routes (*.csv *.txt)");
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
    list = line.split(';');
    bool sbsFormat=false;
    if(list[0].toUpper() != "POSITION")
    {
        if(list.count()==6)
            sbsFormat=true;
        else
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                 QString(QObject::tr("Fichier %1 invalide (doit commencer par POSITION et non '%2'), ou alors etre au format sbsRouteur"))
                            .arg(fileName)
                            .arg(list[0].toUpper()));
            routeFile.close();
        return;
        }
    }
    bool ok;
    QString routeName=QInputDialog::getText(this,tr("Nom de la route a importer"),tr("Nom de la route"),QLineEdit::Normal,"ImportedRoute",&ok);
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
    route->setBoat(main->getSelectedBoat());
    route->setStartFromBoat(false);
    route->setStartTimeOption(3);
    route->setColor(QColor(227,142,42,255));
    route->setImported();
    route->setFrozen(true);

    int n=0;
    QString poiName;
    double lon,lat;
    while(true)
    {
        if(!sbsFormat)
        {
            n++;
            line=stream.readLine();
            if(line.isNull()) break;
            list = line.split(';');
            lat=list[0].mid(0,2).toInt()+list[0].mid(3,6).toFloat()/60.0;
            if(list[0].mid(10,1)!="N") lat=-lat;
            lon=list[0].mid(13,3).toInt()+list[0].mid(17,6).toFloat()/60.0;
            if(list[0].mid(24,1)!="E") lon=-lon;
            QString poiN;
            poiN.sprintf("%.5i",n);
            poiName=route->getName()+poiN;
            POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,main->getSelectedBoat());
            poi->setRoute(route);
            QDateTime start=QDateTime::fromString(list[1],"dd/MM/yyyy hh:mm:ss");
            start.setTimeSpec(Qt::UTC);
            if(n==1)
            {
                route->setStartTime(start);

            }
            poi->setRouteTimeStamp(start.toTime_t());
        }
        else
        {
            n++;
            list = line.split(';');
            QDateTime start=QDateTime::fromString(list[0].simplified(),"dd-MMM.-yyyy hh:mm");
            if(!start.isValid())
            {
                start=QDateTime::fromString(list[0].simplified(),"dd-MMM hh:mm");
                start=start.addYears(QDate::currentDate().year()-1900);
            }
            start=start.toUTC();
            start.setTimeSpec(Qt::UTC);
            if(n==1)
            {
                route->setStartTime(start);

            }
            /* 45? 53' 29.785" N  2? 57' 44.532" W  */
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
            QString poiN;
            poiN.sprintf("%.5i",n);
            poiName=route->getName()+poiN;
            POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,main->getSelectedBoat());
            poi->setRoute(route);
            poi->setRouteTimeStamp(start.toTime_t());
            line=stream.readLine();
            if(line.isNull()) break;
        }
    }
    routeFile.close();
    route->setHidePois(true);
    route->setImported();
    route->setFrozen2(false);//calculate only once and relock immediately
    route->setFrozen2(true);
}
void myCentralWidget::slot_twaLine()
{
    int X,Y;
    main->getXY(&X,&Y);
    double lon, lat;
    proj->screen2map(X,Y, &lon, &lat);
    twaDraw(lon,lat);
}
void myCentralWidget::twaDraw(double lon, double lat)
{
    if (main->getSelectedBoat()==NULL) return;
    if (!grib->isOk()) return;
    QPointF start(lon,lat);
    if(twaTrace==NULL)
        twaTrace=new twaLine(start,this, main);
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
                         tr("Exporter une Route"), routePath, "Routes (*.csv *.txt)");
    if(fileName.isEmpty() || fileName.isNull()) return;
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
        time.setTimeSpec(Qt::UTC);
        list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
        stream<<list.join(";")<<endl;
    }
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
        time.setTimeSpec(Qt::UTC);
        list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
        stream<<list.join(";")<<endl;
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
ROUTE * myCentralWidget::addRoute()
{
    ROUTE * route=new ROUTE("Route", proj, grib, scene, this);
    route->setBoat(main->getSelectedBoat());
    connect(this,SIGNAL(updateRoute()),route,SLOT(slot_recalculate()));
    connect(main,SIGNAL(updateRoute()),route,SLOT(slot_recalculate()));
    connect(route,SIGNAL(editMe(ROUTE *)),this,SLOT(slot_editRoute(ROUTE *)));
    connect(route,SIGNAL(deletePoi(POI *)),this,SLOT(slot_delPOI_list(POI *)));


    connect(this, SIGNAL(showALL(bool)),route,SLOT(slot_shShow()));
    connect(this, SIGNAL(hideALL(bool)),route,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shRou(bool)),route,SLOT(slot_shRou()));


    route_list.append(route);
    return route;
}
ROUTAGE * myCentralWidget::addRoutage()
{
    ROUTAGE * routage=new ROUTAGE("Routage", proj, grib, scene, this);
    routage->setBoat(main->getSelectedBoat());
    connect(this,SIGNAL(updateRoutage()),routage,SLOT(slot_recalculate()));
    connect(main,SIGNAL(updateRoutage()),routage,SLOT(slot_recalculate()));
    connect(routage,SIGNAL(editMe(ROUTAGE *)),this,SLOT(slot_editRoutage(ROUTAGE *)));
    connect(routage,SIGNAL(deletePoi(POI *)),this,SLOT(slot_delPOI_list(POI *)));


    connect(this, SIGNAL(showALL(bool)),routage,SLOT(slot_shShow()));
    connect(this, SIGNAL(hideALL(bool)),routage,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shRoutage(bool)),routage,SLOT(slot_shRou()));


    routage_list.append(routage);
    return routage;
}
void myCentralWidget::slot_editRoute(ROUTE * route,bool createMode)
{
    ROUTE_Editor *route_editor=new ROUTE_Editor(route,this);
    if(route_editor->exec()!=QDialog::Accepted)
    {
        if(createMode)
        {
            delete route;
            route_list.removeAll(route);
            route=NULL;
        }
    }
    else
    {
        update_menuRoute();
        route->slot_recalculate();
        if(route->getSimplify())
        {
            if(route->getFrozen() || !route->getHas_eta())
                QMessageBox::critical(0,QString(QObject::tr("Simplification de route")),QString(QObject::tr("La simplification est impossible pour une route figee ou une route sans ETA")));
            else
            {
                bool ok=false;
                int maxLoss=QInputDialog::getInteger(0,QString(QObject::tr("Simplication de route")),QString(QObject::tr("Perte maximum de temps sur l'ETA finale (en minutes)")),0,0,10000,1,&ok);
                if(!ok)
                    route->setSimplify(false);
                else
                {
                    route->setSimplify(false);
                    QList<POI*> pois=route->getPoiList();
                    int ref_nbPois=pois.count();
                    time_t ref_eta=route->getEta();
                    int nbDel=0;
                    QProgressDialog p(tr("Simplification en cours"),"",1,ref_nbPois-2);
                    p.setLabelText(tr("Phase 1..."));
                    for (int n=1;n<ref_nbPois-2;n++)
                    {
                        POI *poi=pois.at(n);
                        poi->setRoute(NULL);
                        QApplication::processEvents();
                        if(!route->getHas_eta())
                            poi->setRoute(route);
                        else if(route->getEta()<=ref_eta)
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
                    p.setLabelText(tr("Phase 2..."));
                    pois=route->getPoiList();
                    p.setMaximum(pois.count()-2);
                    if(maxLoss!=0)
                    {
                        for (int n=1;n<pois.count()-2;n++)
                        {
                            POI *poi=pois.at(n);
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
                        result=result.sprintf("%d minutes perdues, %d POIs supprimes sur %d",-diff,nbDel,ref_nbPois);
                    else
                        result=result.sprintf("%d minutes gagnees(!), %d POIs supprimes sur %d",diff,nbDel,ref_nbPois);
                    QMessageBox::information(0,QString(QObject::tr("Resultat de la simplification")),result);
                }
            }
        }
    }
    delete route_editor;
}
void myCentralWidget::slot_editRoutage(ROUTAGE * routage,bool createMode)
{
    ROUTAGE_Editor *routage_editor=new ROUTAGE_Editor(routage,this);
    if(routage_editor->exec()!=QDialog::Accepted)
    {
        if(createMode)
        {
            delete routage;
            routage_list.removeAll(routage);
            routage=NULL;
            delete routage_editor;
        }
    }
    else
    {
        delete routage_editor;
        update_menuRoutage();
        if(createMode)
            routage->calculate();
    }
}
void myCentralWidget::deleteRoute(ROUTE * route)
{
    route_list.removeAll(route);
    delete route;
    update_menuRoute();
}
void myCentralWidget::deleteRoutage(ROUTAGE * routage)
{
    routage_list.removeAll(routage);
    delete routage;
    update_menuRoutage();
}
void myCentralWidget::assignPois()
{
    freezeRoutes(true);
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
    freezeRoutes(false);
    emit updateRoute();
}
void myCentralWidget::freezeRoutes(bool freeze)
{
    QListIterator<ROUTE*> i (route_list);
    while(i.hasNext()) i.next()->setFrozen(freeze);
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
    connect(player,SIGNAL(addBoat_list(boatVLM*)),this,SLOT(slot_addBoat_list(boatVLM*)));
    connect(player,SIGNAL(delBoat_list(boatVLM*)),this,SLOT(slot_delBoat_list(boatVLM*)));
}

void myCentralWidget::slot_delPlayer_list(Player* player)
{
    player_list.removeAll(player);
    disconnect(player,SIGNAL(addBoat_list(boatVLM*)),this,SLOT(slot_addBoat_list(boatVLM*)));
    disconnect(player,SIGNAL(delBoat_list(boatVLM*)),this,SLOT(slot_delBoat_list(boatVLM*)));
}

/**************************/
/* Boats                  */
/**************************/

void myCentralWidget::slot_addBoat_list(boatVLM* boat)
{
    //boat_list.append(boat);
    scene->addItem(boat);
    connect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    connect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    connect(boat,SIGNAL(getTrace(QString,QList<vlmPoint> *)),opponents,SLOT(getTrace(QString,QList<vlmPoint> *)));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
    boat->slot_paramChanged();
}

void myCentralWidget::slot_delBoat_list(boatVLM* boat)
{
    //boat_list.removeAll(boat);
    scene->removeItem(boat);
    disconnect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    disconnect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    disconnect(boat,SIGNAL(getTrace(QString,QList<vlmPoint> *)),opponents,SLOT(getTrace(QString,QList<vlmPoint> *)));
    disconnect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
}

void myCentralWidget::slot_boatDialog(void)
{
    if(boatAcc->initList(boat_list,currentPlayer))
        boatAcc->exec();
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
        QListIterator<boatVLM*> i(*boat_list);
        while(i.hasNext())
        {
            i.next()->hide();
        }
    }

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
    currentPlayer = player;
    if(player)
    {
        if(player->getType() == BOAT_VLM)
        {
            menuBar->boatList->setEnabled(true);
            menuBar->acVLMParamBoat->setEnabled(true);
            boat_list=player->getBoats();
            QListIterator<boatVLM*> i(*boat_list);
            while(i.hasNext())
            {
                boatVLM * boat=i.next();
                boat->setStatus(boat->getStatus());
            }
            realBoat=NULL;
            emit accountListUpdated();
        }
        else
        {
            menuBar->boatList->setEnabled(false);
            menuBar->acVLMParamBoat->setEnabled(false);
            realBoat=player->getRealBoat();
            main->slotSelectBoat((boat*)realBoat);
        }
    }
    else
    {
        menuBar->boatList->setVisible(false);
        menuBar->acVLMParamBoat->setEnabled(false);
        boat_list=NULL;
        realBoat=NULL;
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
    raceParam->initList(*boat_list,race_list);
    //raceParam->exec();
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
    }
}
