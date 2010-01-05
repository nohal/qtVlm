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

#include "mycentralwidget.h"
#include "poi_delete.h"

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

    /* scene and views */
    scene =  new myScene(this);
    scene->setSceneRect(QRect(0,0,width(),height()));

    view = new QGraphicsView(scene, this);
    view->setGeometry(0,0,width(),height());

    /* other child */
    gshhsReader = new GshhsReader("maps/gshhs", 0);
    grib = new Grib();
    route=new vlmLine(proj,scene,Z_VALUE_ROUTE);

    /* item child */
    // Terre
    terre = new Terrain(this,proj);
    terre->setGSHHS_map(gshhsReader);
    terre->setCitiesNamesLevel(Util::getSetting("showCitiesNamesLevel", 0).toInt());
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
    connect(&dialogUnits, SIGNAL(accepted()), terre, SLOT(redrawAll()));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), terre, SLOT(updateGraphicsParameters()));
    scene->addItem(terre);

    // Compass
    compass = new mapCompass(proj,parent,this);
    scene->addItem(compass);
    connect(parent,SIGNAL(showCompassLine(int,int)),compass,SLOT(slot_compassLine(int,int)));
    connect(parent,SIGNAL(paramVLMChanged()),compass,SLOT(slot_paramChanged()));
    connect(parent,SIGNAL(paramVLMChanged()),this,SLOT(slot_updateRoute()));
    if(Util::getSetting("showCompass",1).toInt()==1)
        compass->show();
    else
        compass->hide();

    // Selection zone
    selection=new selectionWidget(proj,scene);
    connect(menuBar->acMap_Orthodromie, SIGNAL(triggered(bool)),
            selection,  SLOT(slot_setDrawOrthodromie(bool)));
    scene->addItem(selection);

    // Menu
    connect(menuBar->acMap_GroupCitiesNames, SIGNAL(triggered(QAction *)),
            this, SLOT(slot_map_CitiesNames()));

    // Opponents
    opponents = new opponentList(proj,main,this);

     /* Dialogs */
    gribDateDialog = new dialog_gribDate();
    poi_editor=new POI_Editor(parent,this);
    gate_edit=new gate_editor(parent,this);
    boatAcc = new boatAccount_dialog(proj,parent,this);
    connect(boatAcc,SIGNAL(writeBoat()),this,SLOT(slot_writeBoatData()));
    raceParam = new race_dialog(parent,this);
    connect(raceParam,SIGNAL(readRace()),this,SLOT(slot_readRaceData()));
    connect(raceParam,SIGNAL(writeBoat()),this,SLOT(slot_writeBoatData()));
    dialogLoadGrib = new DialogLoadGrib();
    connect(dialogLoadGrib, SIGNAL(signalGribFileReceived(QString)),
            parent,  SLOT(slot_gribFileReceived(QString)));
    connect(menuBar->acOptions_Units, SIGNAL(triggered()), &dialogUnits, SLOT(exec()));
    connect(menuBar->acOptions_GraphicsParams, SIGNAL(triggered()), &dialogGraphicsParams, SLOT(exec()));

    /* Boats */
    connect(parent,SIGNAL(boatHasUpdated(boatAccount*)),this,SLOT(slot_updateRouteFromBoatChange(boatAccount*)));
    xmlData = new xml_boatData(proj,parent,this);
    //emit readBoatData("boatAcc.dat",true);

    /* POIs & Gates*/
    while (!poi_list.isEmpty())
        delete poi_list.takeFirst();
    while (!gate_list.isEmpty())
        delete gate_list.takeFirst();

    xmlPOI = new xml_POIData(proj,parent,this);
    //emit readPOIData("poi.dat");
}

void myCentralWidget::loadData(void)
{
    emit readBoatData("boatAcc.dat",true);
    emit readPOIData("poi.dat");
}

myCentralWidget::~myCentralWidget()
{
    xmlPOI->slot_writeData(poi_list,gate_list,"poi.dat");
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
    if(!compass)
        return COMPASS_NOTHING;

    if(compass->hasCompassLine())
        return COMPASS_LINEON;
    else if(compass->contains(QPointF(m_x-compass->x(),m_y-compass->y())))
        return COMPASS_UNDER;

    return COMPASS_NOTHING;
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

    slot_updateRoute();

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
        slot_updateRoute();
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
        slot_updateRoute();
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
    poi->show();
}

void myCentralWidget::slot_addPOI_list(POI * poi)
{
    poi_list.append(poi);
    scene->addItem(poi);
    connect(poi, SIGNAL(editPOI(POI*)),poi_editor, SLOT(editPOI(POI*)));
    connect(proj, SIGNAL(projectionUpdated()), poi, SLOT(slot_updateProjection()));
    connect(poi, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
    connect(poi, SIGNAL(updateRoute()),this,SLOT(slot_updateRoute()));
}

void myCentralWidget::slot_delPOI_list(POI * poi)
{
    poi_list.removeAll(poi);
    scene->removeItem(poi);
    disconnect(poi, SIGNAL(editPOI(POI*)),poi_editor, SLOT(editPOI(POI*)));
    disconnect(proj, SIGNAL(projectionUpdated()), poi, SLOT(slot_updateProjection()));
    disconnect(poi, SIGNAL(clearSelection()),this,SLOT(slot_clearSelection()));
//  disconnect(poi, SIGNAL(updateRoute()),this,SLOT(slot_updateRoute()));
}

void myCentralWidget::slot_delAllPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;
    bool changed_route=false;

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
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                slot_delPOI_list(poi);
                if(poi->getType()==POI_TYPE_WP)
                    changed_route=true;
                delete poi;
            }

        }
        selection->clearSelection();
        if(changed_route) slot_updateRoute();
    }
}

void myCentralWidget::slot_delSelPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;
    bool changed_route=false;

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
            qWarning() << "POI: " << poi->getName() << " mask=" << poi->getTypeMask();
            if(!(poi->getTypeMask() & res_mask))
                continue;
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                if(poi->getType()==POI_TYPE_WP)
                    changed_route=1;
                slot_delPOI_list(poi);
                delete poi;
            }
        }
        selection->clearSelection();
        if(changed_route) slot_updateRoute();
    }
}

void myCentralWidget::slot_updateRouteFromBoatChange(boatAccount * boat)
{
    if(boat==main->getSelectedBoat()) slot_updateRoute();
}

void myCentralWidget::slot_updateRoute()
{
    route->deleteAll();
    if(poi_list.count()==0) return;

    boatAccount * selectedBoat=main->getSelectedBoat();
    time_t eta;
    if(selectedBoat && selectedBoat->getPolarData() && grib)
    {
        if(Util::getSetting("routeStart",0).toInt()==0)
        {
            eta=selectedBoat->getPrevVac();
            time_t now = (QDateTime::currentDateTime()).toUTC().toTime_t();
            if(eta < now - selectedBoat->getVacLen()) /*cas du boat inscrit depuis longtemps mais pas encore parti*/
                eta=now;
        }
        else
            eta=grib->getCurrentDate();
        time_t startTime=eta;
        float   lon=selectedBoat->getLon();
        float   lat=selectedBoat->getLat();
        /* ajout du boat comme premier point de la route */
        route->addPoint(lat,lon);
        bool    has_eta=true;
        Orthodromie orth(0,0,0,0);
        qSort(poi_list.begin(),poi_list.end(),POI::myLessThan);
        QListIterator<POI*> i (poi_list);
        QString tip;
        float initial_distance,newSpeed,speed,distance,remaining_distance,angle,res_lon,res_lat,previous_remaining_distance,cap,cap1,cap2,diff1,diff2;
        double wind_angle,wind_speed;
        time_t  mult_vac=1;
        while(i.hasNext())
        {
            POI * poi = i.next();
            if(poi->getType()!=POI_TYPE_WP) continue;
            if(!grib->isOk())
            {
                tip=poi->getTypeStr() + " : " + poi->getName() + "<br>Ortho Dist from boat: "+
                           Util::formatDistance(initial_distance)+"<br>Initial Speed if Ortho: "+Util::formatSpeed(speed * 0.514444444444444 )+
                           "<br>Estimated ETA: No grib loaded" ;
                poi->setRouteTimeStamp(-1);
                poi->setTip(tip);
                continue;
            }

            //if(poi->getAccount()!=selectedBoat->getLogin()) continue;
            orth.setPoints(selectedBoat->getLon(), selectedBoat->getLat(), poi->getLongitude(),poi->getLatitude());
            initial_distance=orth.getDistance();
            newSpeed=0;
            speed=0;
            distance=0;
            remaining_distance=initial_distance;
            angle=orth.getAzimutDeg()-selectedBoat->getWindDir();
            if(qAbs(angle)>180)
            {
                if(angle<0)
                    angle=360+angle;
                else
                    angle=angle-360;
            }
            speed=selectedBoat->getPolarData()->getSpeed(selectedBoat->getWindSpeed(),angle);
            res_lon=0;
            res_lat=0;
            previous_remaining_distance=0;
            wind_angle=0;
            wind_speed=0;
            orth.setPoints(lon, lat, poi->getLongitude(),poi->getLatitude());
            time_t maxDate=grib->getMaxDate();
            if(has_eta)
            {
                do
                {
                    if(grib->getInterpolatedValue_byDates((double) lon,(double) lat,
                                              eta,&wind_speed,&wind_angle) && eta<=maxDate)
                    {
                        previous_remaining_distance=remaining_distance;
                        wind_angle=radToDeg(wind_angle);
                        cap=orth.getAzimutDeg();
                        angle=cap-wind_angle;
                        if(qAbs(angle)>180)
                        {
                            if(angle<0)
                                angle=360+angle;
                            else
                                angle=angle-360;
                        }
                        if(qAbs(angle)<selectedBoat->getBvmgUp(wind_speed))
                        {
                            angle=selectedBoat->getBvmgUp(wind_speed);
                            cap1=A360(wind_angle+angle);
                            cap2=A360(wind_angle-angle);
                            diff1=A360(qAbs(cap-cap1));
                            diff2=A360(qAbs(cap-cap2));
                            if(diff1<diff2)
                                cap=cap1;
                            else
                                cap=cap2;
                        }
                        else if(qAbs(angle)>selectedBoat->getBvmgDown(wind_speed))
                        {
                            angle=selectedBoat->getBvmgDown(wind_speed);
                            cap1=A360(wind_angle+angle);
                            cap2=A360(wind_angle-angle);
                            diff1=A360(qAbs(cap-cap1));
                            diff2=A360(qAbs(cap-cap2));
                            if(diff1<diff2)
                                cap=cap1;
                            else
                                cap=cap2;
                        }
                        newSpeed=selectedBoat->getPolarData()->getSpeed(wind_speed,angle);
                        distance=newSpeed*selectedBoat->getVacLen()*mult_vac/3600.00;
                        Util::getCoordFromDistanceAngle(lat, lon, distance, cap,&res_lat,&res_lon);
                        lon=res_lon;
                        lat=res_lat;
                        route->addPoint(lat,lon);
                        orth.setStartPoint(lon, lat);
                        eta= eta + selectedBoat->getVacLen()*mult_vac;
                        remaining_distance=orth.getDistance();
                    }
                    else
                        has_eta=false;
                } while (remaining_distance>newSpeed*selectedBoat->getVacLen()/7200.000 && previous_remaining_distance>distance && has_eta);
            }
            if(!has_eta)
            {
                tip=poi->getTypeStr() + " : " + poi->getName() + "<br>Ortho Dist from boat: "+
                           Util::formatDistance(initial_distance)+"<br>Initial Speed if Ortho: "+Util::formatSpeed(speed * 0.514444444444444 )+
                           "<br>Estimated ETA: Unreachable within Grib" ;
                poi->setRouteTimeStamp(0);
            }
            else
            {
                float days=(eta-startTime)/86400.0000;
                if(qRound(days)>days)
                    days=qRound(days)-1;
                else
                    days=qRound(days);
                float hours=(eta-startTime-days*86400)/3600.0000;
                if(qRound(hours)>hours)
                    hours=qRound(hours)-1;
                else
                    hours=qRound(hours);
                float mins=qRound((eta-startTime-days*86400-hours*3600)/60.0000);
                tip=poi->getTypeStr() + " : " + poi->getName() + "<br>Ortho Dist from boat: "+
                           Util::formatDistance(initial_distance)+"<br>Initial Speed if Ortho: "+Util::formatSpeed(speed * 0.514444444444444 )+
                           "<br>Estimated ETA from last VAC:<br>"+QString::number((int)days)+" days "+QString::number((int)hours)+" hours "+QString::number((int)mins)+" minutes";
                poi->setRouteTimeStamp((int)eta);
            }
            poi->setTip(tip);
            lon=poi->getLongitude();
            lat=poi->getLatitude();
        }
    }
    route->slot_showMe();
}
float myCentralWidget::A360(float hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

/**************************/
/* Gate                   */
/**************************/

void myCentralWidget::slot_addGate_list(gate * ptr)
{
    gate_list.append(ptr);
#warning il faudra ajouter les gates
    //scene->addItem(ptr);
    connect(ptr,SIGNAL(editGate(gate*)),gate_edit,SLOT(editGate(gate*)));
    connect(proj, SIGNAL(projectionUpdated()), ptr, SLOT(slot_updateProjection()));
}

void myCentralWidget::slot_delGate_list(gate * ptr)
{
    gate_list.removeAll(ptr);
    //scene->removeItem(ptr);
    disconnect(ptr,SIGNAL(editGate(gate*)),gate_edit,SLOT(editGate(gate*)));
    disconnect(proj, SIGNAL(projectionUpdated()), ptr, SLOT(slot_updateProjection()));
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
    emit writePOIData(poi_list,gate_list,"poi.dat");
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
