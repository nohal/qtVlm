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

#include "mycentralwidget.h"
#include "poi_delete.h"
#include "settings.h"
#include "opponentBoat.h"
#include "Projection.h"
#include "MainWindow.h"
#include "GshhsReader.h"
#include "Grib.h"
#include "Terrain.h"
#include "inetConnexion.h"
#include "MenuBar.h"
#include "mapcompass.h"
#include "selectionWidget.h"
#include "dialog_gribDate.h"
#include "POI_editor.h"
#include "POI.h"
#include "boatAccount.h"
#include "race_dialog.h"
#include "boatAccount_dialog.h"
#include "DialogLoadGrib.h"
#include "xmlBoatData.h"
#include "xmlPOIData.h"
#include "Route_Editor.h"

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
    connect(menuBar->acMap_Rivers, SIGNAL(triggered(bool)), terre,  SLOT(setDrawRivers(bool)));
    connect(menuBar->acMap_CountriesBorders, SIGNAL(triggered(bool)), terre,  SLOT(setDrawCountriesBorders(bool)));
    connect(menuBar->acMap_CountriesNames, SIGNAL(triggered(bool)), terre,  SLOT(setCountriesNames(bool)));
    connect(menuBar->acView_WindColors, SIGNAL(triggered(bool)), terre,  SLOT(slot_setDrawWindColors(bool)));
    connect(menuBar->acView_ColorMapSmooth, SIGNAL(triggered(bool)), terre,  SLOT(setColorMapSmooth(bool)));
    connect(menuBar->acView_WindArrow, SIGNAL(triggered(bool)), terre,  SLOT(setDrawWindArrows(bool)));
    connect(menuBar->acView_Barbules, SIGNAL(triggered(bool)), terre,  SLOT(setBarbules(bool)));

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



    connect(&dialogUnits, SIGNAL(accepted()), terre, SLOT(redrawAll()));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), terre, SLOT(updateGraphicsParameters()));
    scene->addItem(terre);

    // Compass
    compass = new mapCompass(proj,parent,this);
    scene->addItem(compass);
    connect(parent,SIGNAL(showCompassLine(int,int)),compass,SLOT(slot_compassLine(int,int)));
    connect(parent,SIGNAL(showCompassCenterBoat()),compass,SLOT(slot_compassCenterBoat()));
    connect(parent,SIGNAL(showCompassCenterWp()),compass,SLOT(slot_compassCenterWp()));
    connect(parent,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(selectedBoatChanged()),compass,SLOT(slot_paramChanged()));
    connect(scene,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(boatHasUpdated(boatAccount*)),compass,SLOT(slot_paramChanged()));
    connect(this, SIGNAL(showALL(bool)),compass,SLOT(slot_paramChanged()));
    connect(this, SIGNAL(hideALL(bool)),compass,SLOT(slot_shHidden()));
    connect(this, SIGNAL(shCom(bool)),compass,SLOT(slot_shCom()));
    connect(this, SIGNAL(shPol(bool)),compass,SLOT(slot_shPol()));
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
    connect(boatAcc,SIGNAL(writeBoat()),this,SLOT(slot_writeBoatData()));
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

    /* Boats */
    xmlData = new xml_boatData(proj,parent,this,inetManager);

    /* POIs*/
    while (!poi_list.isEmpty())
        delete poi_list.takeFirst();

    xmlPOI = new xml_POIData(proj,parent,this);


}

void myCentralWidget::loadData(void)
{
    emit readBoatData("boatAcc.dat",true);
    emit readPOIData("poi.dat");
}

myCentralWidget::~myCentralWidget()
{
    xmlPOI->slot_writeData(route_list,poi_list,"poi.dat");
    xmlData->slot_writeData(acc_list,race_list,QString("boatAcc.dat"));
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
    view->setGeometry(0,0,width(), height());
    scene->setSceneRect(QRect(0,0,width()-4, height()-4));
    terre->updateSize(width()-4, height()-4);
    proj->setScreenSize( width()-4, height()-4);
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

void myCentralWidget::slot_fileInfo_GRIB()
{
    #warning refaire le grib info
    /*
    QString msg;
    if (! terre->getGrib()->isOk())
    {
        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            tr("Aucun fichir GRIB n'est chargé."));
    }
    else {
        Grib * grib = terre->getGrib();

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
        msg += tr("    Vent  : %1\n").arg(dataPresentInGrib(grib,GRB_WIND_VX));

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
    }*/
}

/**************************/
/* POI                    */
/**************************/
void myCentralWidget::slot_addPOI(QString name,int type,float lat,float lon, float wph,int timestamp,bool useTimeStamp,boatAccount *boat)
{
    POI * poi;

    if(name=="")
        name=QString(tr("POI"));
    if(boat==NULL) boat=main->getSelectedBoat();
    poi = new POI(name,type,lat,lon, proj,
                  main, this,wph,timestamp,useTimeStamp,boat);

    slot_addPOI_list(poi);
    //poi->show();
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
                if(poi->getRoute()->getFrozen()) return;
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
                if(poi->getRoute()->getFrozen()) return;
            qWarning() << "POI: " << poi->getName() << " mask=" << poi->getTypeMask();
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

void myCentralWidget::slot_addRouteFromMenu()
{
    ROUTE * route=addRoute();
    slot_editRoute(route,true);
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
    }
    delete route_editor;
}
void myCentralWidget::deleteRoute(ROUTE * route)
{
    route_list.removeAll(route);
    delete route;
    update_menuRoute();
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
            QListIterator<ROUTE*> j (route_list);
            while(j.hasNext())
            {
                ROUTE * route=j.next();
                if(poi->getRouteName() == route->getName())
                {
                    poi->setRoute(route);
                }
            }
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
    QListIterator<ROUTE*> i (route_list);
    while(i.hasNext())
        menuBar->addMenuRoute(i.next());
}
float myCentralWidget::A360(float hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

/**************************/
/* Boats                  */
/**************************/

void myCentralWidget::slot_addBoat_list(boatAccount* boat)
{
    acc_list.append(boat);
    scene->addItem(boat);
    connect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    connect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    connect(boat,SIGNAL(getTrace(QString,QList<vlmPoint> *)),opponents,SLOT(getTrace(QString,QList<vlmPoint> *)));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
}

void myCentralWidget::slot_delBoat_list(boatAccount* boat)
{
    acc_list.removeAll(boat);
    scene->removeItem(boat);
    disconnect(proj,SIGNAL(projectionUpdated()),boat,SLOT(slot_projectionUpdated()));
    disconnect(boat, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    disconnect(boat,SIGNAL(getTrace(QString,QList<vlmPoint> *)),opponents,SLOT(getTrace(QString,QList<vlmPoint> *)));
    disconnect(&dialogGraphicsParams, SIGNAL(accepted()), boat, SLOT(slot_updateGraphicsParameters()));
}

void myCentralWidget::slot_boatDialog(void)
{
    boatAcc->initList(acc_list);
    boatAcc->exec();
}

void myCentralWidget::slot_writeBoatData(void)
{
    emit writeBoatData(acc_list,race_list,QString("boatAcc.dat"));
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
    raceParam->initList(acc_list,race_list);
    raceParam->exec();
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
