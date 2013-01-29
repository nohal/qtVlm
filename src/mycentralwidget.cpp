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
#ifdef QT_V5
#include <QtMultimedia/QSound>
#else
#include <QSound>
#endif
#include <QDesktopServices>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlQuery>
#include <QGesture>
#include <QVariantMap>
#include <QVariant>
#include <QClipboard>


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
#include "faxMeteo.h"
#include "loadImg.h"
#include "GshhsDwnload.h"

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
#include "DialogVlmLog.h"
#include "DialogDownloadTracks.h"
#include "dialogFaxMeteo.h"
#include "dialogLoadImg.h"
#include "parser.h"
#include "DialogRemovePoi.h"

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
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toDouble(), positions[1].toDouble(), positions[2].toDouble());
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
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toDouble(), positions[1].toDouble(), positions[2].toDouble());
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
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toDouble(), positions[1].toDouble(), positions[2].toDouble());
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
                    parent->getProj()->setScaleAndCenterInMap(positions[0].toDouble(), positions[1].toDouble(), positions[2].toDouble());
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
    if(parent->getIsStartingUp()) return;
#if 0
    if(hasWay)
    {
        emit eraseWay();
        hasWay=false;
    }
    if(!parent->getIsSelecting())
    {
        if(itemAt(e->scenePos())->data(0)==ISOPOINT)
        {
            vlmPointGraphic * vg=((vlmPointGraphic *) itemAt(e->scenePos()));
            if(vg->getRoutage()->isDone() && vg->getRoutage()->getShowIso())
            {
                vg->drawWay();
                hasWay=true;
            }
        }
    }
#endif
    parent->mouseMove(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos(),parent->getView()->transform()));
    QGraphicsScene::mouseMoveEvent(e);
}

void myScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    if(e->button()==Qt::LeftButton)
    {
        parent->mouseDoubleClick(e->scenePos().x(),e->scenePos().y(),itemAt(e->scenePos(),parent->getView()->transform()));
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
    this->boat_list=NULL;
    this->fax=NULL;
    this->kap=NULL;
    this->routeClipboard=NULL;

    currentPlayer=NULL;

    resizing=false;
    this->twaTrace=NULL;
    /* item state */
    shLab_st = false;
    shPoi_st = false;
    shRoute_st = false;
    shOpp_st = false;
    shPor_st = false;

    gshhsReader = NULL;

    /* scene and views */
    scene =  new myScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(QRect(0,0,width(),height()));

    view = new QGraphicsView(scene, this);
    view->setGeometry(0,0,width(),height());
    view->viewport()->grabGesture(Qt::PanGesture);
    view->viewport()->grabGesture(Qt::PinchGesture);
    view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

    /* other child */
    inetManager = new inetConnexion(mainW);

    gshhsDwnload = new GshhsDwnload(this,inetManager);
    terre=NULL;
    loadGshhs();
    grib = new Grib();
    gribCurrent = new Grib();
    gribCurrent->setIsCurrentGrib();
    grib->setGribCurrent(gribCurrent);

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
    horn=new QSound(appFolder.value("img")+"boat_horn.wav");
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
    dialogLoadGrib->checkQtvlmVersion();
    connect(dialogLoadGrib, SIGNAL(signalGribFileReceived(QString)),
            parent,  SLOT(slot_gribFileReceived(QString)));
    connect(menuBar->acOptions_Units, SIGNAL(triggered()), &dialogUnits, SLOT(exec()));
    connect(menuBar->acOptions_GraphicsParams, SIGNAL(triggered()), &dialogGraphicsParams, SLOT(exec()));

    vlmLogViewer = new DialogVlmLog(this);
    vlmTrackRetriever = new DialogDownloadTracks(parent,this,this->getInet());
    /*Routes*/
    connect(menuBar->acRoute_add, SIGNAL(triggered()), this, SLOT(slot_addRouteFromMenu()));
    connect(menuBar->acRoute_import, SIGNAL(triggered()), this, SLOT(slot_importRouteFromMenu()));
    connect(menuBar->acRoute_import2, SIGNAL(triggered()), this, SLOT(slot_importRouteFromMenu2()));

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
#if 0 //test pb de marano (resolu)
    QLineF marano(17.5895855642,-61.926110110422,17.582814750193,-61.915067613033);
    QLineF gate(17.5840000000,-61.9170000000,17.5840000000,-61.8630000000);
    QPointF cross;
    if(marano.intersect(gate,&cross)==QLineF::BoundedIntersection)
        qWarning()<<"Marano is crossing at"<<QString().sprintf("%.10f",cross.x())<<QString().sprintf("%.10f",cross.y());
    else
        qWarning()<<"Marano is NOT crossing"<<QString().sprintf("%.10f",cross.x())<<QString().sprintf("%.10f",cross.y());
    QPen pen;
    pen.setColor(Qt::black);
    pen.setBrush(Qt::black);
    pen.setWidthF(1.0);
    vlmLine * mar=new vlmLine(proj,this->scene,10);
    mar->setLinePen(pen);
    vlmLine * gat=new vlmLine(proj,this->scene,10);
    pen.setColor(Qt::red);
    pen.setBrush(Qt::red);
    gat->setLinePen(pen);
    gat->addPoint(17.5840000000,-61.9170000000);
    gat->addPoint(17.5840000000,-61.8630000000);
    gat->slot_showMe();
    mar->addPoint(17.5895855642,-61.926110110422);
    mar->addPoint(17.582814750193,-61.915067613033);
    mar->slot_showMe();
#endif
}

void myCentralWidget::loadGshhs(void) {

    if(gshhsReader) {
        if(terre)
            terre->setGSHHS_map(NULL);

        delete gshhsReader;
        gshhsReader=NULL;
    }

    QString mapDir = Settings::getSetting("mapsFolder",appFolder.value("maps")).toString();

    //qWarning() << "Searching for maps in " << mapDir;

    gshhsReader = new GshhsReader((mapDir+"/gshhs").toLatin1().data());
    gshhsReader->setProj(proj);

    int polyVersion = gshhsReader->getPolyVersion();
    if(polyVersion==-1 || polyVersion!=220)
    {
        mapDir=".";
        delete gshhsReader;
        gshhsReader = new GshhsReader((mapDir+"/gshhs").toLatin1().data());
        gshhsReader->setProj(proj);
        polyVersion = gshhsReader->getPolyVersion();
    }
    bool dwnloadMaps = false;
    bool gshhsOk=true;

    QMessageBox msgBox(QMessageBox::Question,tr("Maps loading"),"",QMessageBox::NoButton,this);
    QPushButton * selectFolderBtn = msgBox.addButton(tr("Select existing maps folder"),QMessageBox::ApplyRole);
    QPushButton * downloadMapBtn = msgBox.addButton(tr("Downloading"),QMessageBox::AcceptRole);
    msgBox.addButton(tr("Annuler"),QMessageBox::RejectRole);

    if(polyVersion == -1) {
        qWarning() << "Missing maps";
        gshhsOk=false;
        msgBox.setText(tr("Maps are missing\nWhat do you want to do?"));
    }
    else if(polyVersion!=220) {
        qWarning()<<"wrong poly version->"<<gshhsReader->getPolyVersion();
        gshhsOk=false;
        msgBox.setText(tr("An old version of maps has been detected\nWhat do you want to do?"));
    }
    QDir dir(mapDir);
    QDir appDir=QDir::currentPath();
    if(dir.rootPath()==appDir.rootPath())
        mapDir=appDir.relativeFilePath(mapDir);
    else
        mapDir=appDir.absoluteFilePath(mapDir);
    qWarning() << "Setting map folder to " << mapDir;
    Settings::setSetting("mapsFolder",mapDir);
    if(!gshhsOk)
    {
        msgBox.exec();

        if(msgBox.clickedButton() == downloadMapBtn) {
            dwnloadMaps=true;
        }
        else if(msgBox.clickedButton() == selectFolderBtn) {
            mapDir = QFileDialog::getExistingDirectory(this, tr("Select maps folder"),
                                                            mapDir,
                                                            QFileDialog::ShowDirsOnly);
            QDir dir(mapDir);
            QDir appDir=QDir::currentPath();
            if(dir.rootPath()==appDir.rootPath())
                mapDir=appDir.relativeFilePath(mapDir);
            else
                mapDir=appDir.absoluteFilePath(mapDir);
            qWarning() << "Setting map folder to " << mapDir;
            Settings::setSetting("mapsFolder",mapDir);
            delete gshhsReader;
            gshhsReader=NULL;

            loadGshhs();

            return;
        }
        delete gshhsReader;
        gshhsReader=NULL;
    }

    QProgressDialog * progress=mainW->get_progress();

    if(dwnloadMaps) {
        if(progress) {
            progress->setLabelText(tr("Download and decompress maps"));
            progress->setValue(progress->value()+5);
        }
        gshhsDwnload->getMaps();

    }



    if(progress) {
        progress->setLabelText(tr("Finishing map init"));
        progress->setValue(progress->value()+5);
    }

    if(gshhsOk)
    {
        if(terre)
        {
            terre->setGSHHS_map(gshhsReader);
        }
    }

}

void myCentralWidget::setCompassFollow(ROUTE * route)
{
    this->compassRoute=route;
    if(route!=NULL)
    {
        menuBar->ac_compassCenterBoat->setChecked(false);
        Settings::setSetting("compassCenterBoat", "0");
    }
    emitUpdateRoute(mainW->getSelectedBoat());
}
void myCentralWidget::centerCompass(double lon, double lat)
{
    this->compass->compassCenter(lon,lat);
}

void myCentralWidget::loadBoat(void)
{
    emit readBoatData(appFolder.value("userFiles")+"boatAcc.dat",true);
}

void myCentralWidget::loadPOI(void)
{
    emit readPOIData(appFolder.value("userFiles")+"poi.dat");
    foreach(POI * poi1,this->poi_list)
    {
        if(poi1->getLonConnected()!=-1)
        {
            foreach (POI * poi2,poi_list)
            {
                if(qRound(poi2->getLongitude()*10000)==qRound(poi1->getLonConnected()*10000)&&
                   qRound(poi2->getLatitude()*10000)==qRound(poi1->getLatConnected()*10000))
                {
                    if(poi2->getConnectedPoi()==NULL)
                    {
                        poi1->setConnectedPoi(poi2);
                        poi2->setConnectedPoi(poi1);
                        poi2->setLineWidth(poi1->getLineWidth());
                        poi2->setLineColor(poi1->getLineColor());
                        vlmLine * lineBetweenPois=new vlmLine(proj,this->getScene(),Z_VALUE_POI);
                        lineBetweenPois->addVlmPoint(vlmPoint(poi1->getLongitude(),poi1->getLatitude()));
                        lineBetweenPois->addVlmPoint(vlmPoint(poi2->getLongitude(),poi2->getLatitude()));
                        poi1->setLineBetweenPois(lineBetweenPois);
                        poi2->setLineBetweenPois(lineBetweenPois);
                        QPen pen(poi1->getLineColor());
                        pen.setWidthF(poi1->getLineWidth());
                        lineBetweenPois->setLinePen(pen);
                        lineBetweenPois->slot_showMe();
                        break;
                    }
                }
            }
        }
    }
    //qWarning()<<"POIs loaded";
}

myCentralWidget::~myCentralWidget()
{
    if(!mainW->getNoSave() && xmlPOI && xmlData)
    {
        xmlPOI->slot_writeData(route_list,poi_list,appFolder.value("userFiles")+"poi.dat");
        xmlData->slot_writeData(player_list,race_list,QString(appFolder.value("userFiles")+"boatAcc.dat"));
    }
    // Delete POIs and routes
    this->setCompassFollow(NULL);
    while(!route_list.isEmpty())
    {
        ROUTE* route = route_list.takeFirst();

        route->setTemp (true);
        QListIterator<POI*> i (route->getPoiList());
        while(i.hasNext())
        {
            POI * poi = i.next();
            poi->setRoute (NULL);
            poi->setMyLabelHidden (false);
            slot_delPOI_list (poi);
            delete poi;
        }
        delete route;
    }
    while (!poi_list.isEmpty())
    {
        POI * poi=poi_list.first();
        slot_delPOI_list(poi);
        delete poi;
    }
    // Delete GRIB data
    delete gribCurrent;
    gribCurrent=NULL;
    delete grib;
    grib=NULL;
    delete currentPlayer;
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

Grib * myCentralWidget::getGrib(bool calibrate)
{
    if(calibrate) return grib;
    if(grib && grib->isOk())
        return grib;
    else
        return NULL;
}
Grib * myCentralWidget::getGribCurrent(void)
{
    if(gribCurrent && gribCurrent->isOk())
        return gribCurrent;
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

void myCentralWidget::slot_Zoom_In(double quantity)
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

void myCentralWidget::slot_Zoom_Out(double quantity)
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

void myCentralWidget::slot_Zoom_Wheel(double quantity, int XX, int YY, bool centerOnWheel)
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
    //qWarning()<<"calling resize due to resizeEvent in mcw";
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
        if(item->data(0) == FAXMETEO_WTYPE && ((faxMeteo*)item)->tryMoving(x,y))
        {
            break ;
        }
    }

    /* no current move in sub item */
}

void myCentralWidget::keyModif(QKeyEvent *)
{
#if 0
    if (e->modifiers() == Qt::ControlModifier) {
        terre->setCursor(Qt::SizeAllCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
    else if (e->modifiers() == Qt::ShiftModifier) {
        terre->setCursor(Qt::UpArrowCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
    else {
        terre->setCursor(Qt::ArrowCursor);
        //cur_cursor=Qt::SizeAllCursor;
    }
#endif
}

void myCentralWidget::mouseDoubleClick(int x, int y, QGraphicsItem * )
{
    double lon, lat;
    proj->screen2map(x,y, &lon, &lat);
    //qWarning() << "Creating POI at: " << lat << "," << lon << " - " << Util::formatLatitude(lat) << "," << Util::formatLongitude(lon);
    slot_addPOI("",POI_TYPE_POI,lat,lon,-1,-1,false,mainW->getSelectedBoat());
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

void myCentralWidget::zoomOnGrib(Grib * gr)
{
    Grib * myGrib;
    if(gr!=NULL)
        myGrib=gr;
    else
    {
        if(menuBar->acView_CurrentColors->isChecked() && grib->getNumberOfGribRecords(GRB_CURRENT_VX,LV_MSL,0) == 0)
            myGrib=gribCurrent;
        else
            myGrib=grib;
    }
    double x0,y0, x1,y1;
    if (myGrib->getZoneExtension(&x0,&y0, &x1,&y1))
    {
        if(x0 > 180.0 && x1 > 180.0)
        {
            x0-=360.0;
            x1-=360.0;
        }
        //qWarning() << "zoom on grib" << x0 << "," << y0 << " " << x1 << "," << y1;
        const double mh = fabs(x0-x1)*0.05;
        const double mv = fabs(y0-y1)*0.05;
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
    //emit redrawAll();
}
void myCentralWidget::loadGribFileCurrent(QString fileName, bool zoom)
{
    if (!gribCurrent)
        return;

    gribCurrent->loadGribFile(fileName);
    if(!gribCurrent->isOk())
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
        gribCurrent->setCurrentDate(t);
        //emit redrawGrib();
        if(uRoute)
        {
            emit updateRoute(NULL);
            emit updateRoutage();
        }
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
        if(Settings::getSetting("sailsDocPress",0).toInt()==1)
            queryStr+=",PRESS";

        // Format: mailto:query@saildocs.com?subject=Give me a Grib&body=send GFS:56N,59S,33E,87W|0.5,0.5|0,3,6..384|WIND


        if(Settings::getSetting("sDocExternalMail",1).toInt()==1)
            QDesktopServices::openUrl(QUrl(queryStr));
        else
        {
            QString opt="|0.5,0.5|0,3,6..384|WIND";
            if(Settings::getSetting("sailsDocPress",0).toInt()==1)
                opt+=",PRESS";
            DialogSailDocs * sailDocs_diag = new DialogSailDocs("GFS:" + param + opt,this);
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
        else
        {
            if (grib->getNumberOfGribRecords(dataType,levelType,levelValue) > 0) {
                    if (ok != NULL) *ok = true;
                    return tr("oui");
            }
            else
            {
                if(dataType==GRB_CURRENT_VX)
                {
                    if (gribCurrent && gribCurrent->isOk() && gribCurrent->getNumberOfGribRecords(dataType,levelType,levelValue) > 0)
                    {
                        if (ok != NULL) *ok = true;
                        return tr("oui (GRIB Courants)");
                    }
                }
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
        if(grib->getNumberOfDates()!=0 && grib->getTotalNumberOfGribRecords()!=0)
        {
            std::set<time_t> * sdates = grib->getListDates();
            msg += tr("    du %1\n").arg( Util::formatDateTimeLong(*(sdates->begin())) );
            msg += tr("    au %1\n").arg( Util::formatDateTimeLong(*(sdates->rbegin())) );
        }

        msg += tr("\n");
        msg += tr("Donnees disponibles :\n");
        msg += tr("    Temperature : %1\n").arg(dataPresentInGrib(grib,GRB_TEMP,LV_ABOV_GND,2));
	msg += tr("    Pression : %1\n").arg(dataPresentInGrib(grib,GRB_PRESSURE,LV_MSL,0));
	msg += tr("    Vent  : %1\n").arg(dataPresentInGrib(grib,GRB_WIND_VX,LV_ABOV_GND,10));
    msg += tr("    Courant  : %1\n").arg(dataPresentInGrib(grib,GRB_CURRENT_VX,LV_MSL,0));
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

        if(grib->getNumberOfDates()!=0 && grib->getTotalNumberOfGribRecords()!=0)
        {

            GribRecord * gr = grib->getFirstGribRecord();
            msg += tr("\n");
            msg += tr("Grille : %1 points (%2x%3)\n")
                            .arg(gr->getNi()*gr->getNj()).arg(gr->getNi()).arg(gr->getNj());
            msg += tr("Resolution : %1x%2\n").arg(gr->getDi()).arg(gr->getDj());
            msg += tr("\n");
            msg += tr("Etendue :\n");
            QString pos1, pos2;
            pos1 = Util::formatPosition( gr->getX(0), gr->getY(0) );
            pos2 = Util::formatPosition( gr->getX(gr->getNi()-1), gr->getY(gr->getNj()-1) );
            msg += tr("%1  ->  %2\n").arg( pos1, pos2);

            msg += tr("\n");
            msg += tr("Date de reference : %1\n")
                            .arg(Util::formatDateTimeLong(gr->getRecordRefDate()));
        }
        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            msg );
    }
}

/**************************/
/* POI                    */
/**************************/
POI * myCentralWidget::slot_addPOI(QString name,int type,double lat,double lon, double wph,int timestamp,bool useTimeStamp,boat *boat)
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

void myCentralWidget::removePOI(void) {
    DialogRemovePoi dialogRemovePoi(mainW,this);
    dialogRemovePoi.exec();
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
    poi->chkIsWP();
}
void myCentralWidget::slot_simpAllPOIs()
{
    simpAllPOIs(false);
}
void myCentralWidget::slot_notSimpAllPOIs()
{
    simpAllPOIs(true);
}

void myCentralWidget::simpAllPOIs(bool simp)
{
    double lat0,lon0,lat1,lon1;

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        double x0,y0,x1,y1;
        proj->map2screenDouble(lon0,lat0,&x0,&y0);
        proj->map2screenDouble(lon1,lat1,&x1,&y1);
        QRectF selRect=QRectF(QPointF(x0,y0),QPointF(x1,y1)).normalized();
        QListIterator<POI*> i (poi_list);


        while(i.hasNext())
        {
            POI * poi = i.next();
            const double lat=poi->getLatitude();
            const double lon=poi->getLongitude();
            double x,y;
            proj->map2screenDouble(lon,lat,&x,&y);

            if(selRect.contains(x,y))
            {
                poi->setNotSimplificable(simp);
            }

        }
    }
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

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        double x0,y0,x1,y1;


        proj->map2screenDouble(lon0,lat0,&x0,&y0);
        proj->map2screenDouble(lon1,lat1,&x1,&y1);
        QRectF selRect=QRectF(QPointF(x0,y0),QPointF(x1,y1)).normalized();
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
            const double lat=poi->getLatitude();
            const double lon=poi->getLongitude();
            double x,y;
            proj->map2screenDouble(lon,lat,&x,&y);


            if(selRect.contains(x,y))
            {
                if(poi->getRoute()!=NULL)
                {
                    if(poi->getRoute()->getFrozen()||poi->getRoute()->getHidden()||poi->getRoute()->isBusy()) continue;
                    poi->setRoute(NULL);
                }
                slot_delPOI_list(poi);
                poi->deleteLater();
            }
//            else
//                qWarning()<<"poi outside"<<poi->getName()<<x<<y;
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

    if(selection->getZone(&lon0,&lat0,&lon1,&lat1))
    {
        double x0,y0,x1,y1;
        proj->map2screenDouble(lon0,lat0,&x0,&y0);
        proj->map2screenDouble(lon1,lat1,&x1,&y1);
        QRectF selRect=QRectF(QPointF(x0,y0),QPointF(x1,y1)).normalized();
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
            const double lat=poi->getLatitude();
            const double lon=poi->getLongitude();
            double x,y;
            proj->map2screenDouble(lon,lat,&x,&y);

            if(selRect.contains(x,y))
            {
                if(poi->getRoute()!=NULL)
                {
                    if(poi->getRoute()->getFrozen()||poi->getRoute()->getHidden()||poi->getRoute()->isBusy()) continue;
                    poi->setRoute(NULL);
                }
                slot_delPOI_list(poi);
                poi->deleteLater();
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
    ++replayStep;
    if(replayStep>Settings::getSetting("trace_length",12).toInt()*60*60/mainW->getSelectedBoat()->getVacLen())
    {
        replayTimer->stop();
        emit startReplay(false);
        return;
    }
    if(mainW->getSelectedBoat()->getTraceDrawing()->getPoints()->count()<=replayStep)
    {
        replayTimer->stop();
        emit startReplay(false);
        return;
    }
    emit replay(mainW->getSelectedBoat()->getTraceDrawing()->getPoints()->at(replayStep).timeStamp);
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
//    if (mainW->getSelectedBoat()->getType()==BOAT_VLM)
//        ((boatVLM*)mainW->getSelectedBoat())->exportBoatInfoLog(fileName);
}

void myCentralWidget::slot_showVlmLog()
{
    if (mainW->getSelectedBoat()->getType()==BOAT_VLM)
        vlmLogViewer->initWithBoat( (boatVLM*)mainW->getSelectedBoat() );
    else {
        QMessageBox::warning(0,QObject::tr("Voir Vlm Logs"),
             QString(QObject::tr("Pas de bateau VLM actif.")));
        return;
    }

}

void myCentralWidget::slot_fetchVLMTrack()
{
    Player * cur_player=this->getPlayer();
    if(cur_player)
        vlmTrackRetriever->init();
    else {
        QMessageBox::warning(0,QObject::tr("Telecharger traces VLM"),
             QString(QObject::tr("Pas de compte VLM actif.")));
        return;
    }

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
void myCentralWidget::slot_importRouteFromMenu2()
{
    slot_importRouteFromMenu(true);
}

void myCentralWidget::slot_importRouteFromMenu(bool ortho)
{
    if (!mainW->getSelectedBoat())
    {
        QMessageBox::warning(0,QObject::tr("Aucun Bateau"),
             QString(QObject::tr("L'import de route necessite un bateau actif.")));
        return;
    }
    QString routePath=Settings::getSetting("importRouteFolder","").toString();
    QDir dirRoute(routePath);
    if(!dirRoute.exists())
    {
        routePath=QDir::currentPath();
        Settings::setSetting("importRouteFolder",routePath);
    }
#if 1
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Ouvrir un fichier Route"), routePath, "Routes (*.csv *.txt *.xml *.json *.kml *.CSV *.TXT *.XML *.Json *.JSON *.KML)");
#else
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Ouvrir un fichier Route"), routePath, "Routes (*.csv *.txt *.xml *.csv *.txt *.xml *.kml *.json)");
#endif
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
    if(info.suffix().toLower()=="kml")
        this->importRouteFromMenuKML(fileName,false,ortho);
    else if(info.suffix().toLower()=="json")
    {
        QTextStream stream(&routeFile);
        QString line;
        line=stream.readLine();
        routeFile.close();
        if(line.contains("[{"))
        {
            line.remove(0,1);
            line.remove(line.count()-1,1);
        }
        QStringList lines;
        if(line.contains("[{"))
        {
            while(line.contains("[{"))
            {
                int start=line.indexOf("[{")+1;
                int end=line.indexOf("}]");
                lines.append(line.mid(start,end-start+1));
                line.remove(start,end-start+1);
            }
        }
        else
        {
            lines.append(line);
        }
        foreach(QString track,lines)
        {
            if (track.count()<3) continue;

            if(!track.contains("tracks"))
            {
                track.insert(0,"{\"tracks\":");
                track.append("}");
            }
            //qWarning()<<"track"<<track.left(30);
            QString frName;
            QByteArray buff;
            buff.append(track);
            QJson::Parser parser;
            QVariantMap result = parser.parse (buff, &ok).toMap();
            if (!ok)
            {
                qWarning() << "Error parsing json data ";
                qWarning() << "Error: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
                return;
            }
            if(track.contains("nom"))
            {
                frName=result["nom"].toString();
            }
            routeName=QInputDialog::getText(this,
                                            tr("Nom de la route a importer"),
                                            frName+"<br>"+tr("Nom de la route"),
                                            QLineEdit::Normal,"ImportedRoute",&ok);
            if(!ok)
            {
                continue;
            }
            ROUTE * route=addRoute();
            if (routeName.isEmpty() || !freeRouteName(routeName.trimmed(),route))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Ce nom est deja utilise ou invalide"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.exec();
                this->deleteRoute(route);
                continue;
            }
            QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Import de routes"),
                                      tr("Import en cours, veuillez patienter..."));
            waitBox->setStandardButtons(QMessageBox::NoButton);
            waitBox->show();
            QApplication::processEvents();
            route->setName(routeName);
            update_menuRoute();
            route->setBoat(mainW->getSelectedBoat());
            route->setStartFromBoat(false);
            route->setStartTimeOption(3);
            route->setColor(QColor(227,142,42,255));
            route->setImported();
            route->setFrozen(true);
            route->setTemp(true);
            QVariantList details=result["tracks"].toList();
            QMap<int,QPointF> points;
            for(int n=0;n<details.count();++n)
            {
                QVariant detail=details.at(n);
                QVariantList FTpoint=detail.toList();
                time_t eta=FTpoint.at(0).toInt();
                double lon=FTpoint.at(1).toDouble()/1000.00;
                double lat=FTpoint.at(2).toDouble()/1000.00;
                points.insert(eta,QPointF(lon,lat));
            }
            QDateTime start=QDateTime().fromTime_t(points.begin().key());
            route->setStartTime(start);
            QMapIterator<int,QPointF> p(points);
            int nPoi=0;
            while(p.hasNext())
            {
                ++nPoi;
                int eta=p.next().key();
                double lon=p.value().x();
                double lat=p.value().y();
                QString poiName=route->getName()+QString().sprintf("%.5i",nPoi);
                POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
                if(ortho)
                    poi->setNavMode(2);
                poi->setRoute(route);
                poi->setRouteTimeStamp(eta);
            }
            route->setHidePois(true);
            route->setImported();
            route->setTemp(false);
            route->setFrozen2(false);//calculate only once and relock immediately
            route->setFrozen2(true);
            delete waitBox;
        }
    }
    else if(info.suffix().toLower()!="xml")
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
            timeOffset=QInputDialog::getInt(0,QString(QObject::tr("Importation de routage MaxSea")),QString(QObject::tr("Heures a ajouter/enlever pour obtenir UTC (par ex -2 pour la France)")),0,-24,24,1,&ok);
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
                    lat=list[0].mid(0,2).toInt()+list[0].mid(3,6).toDouble()/60.0;
                    if(list[0].mid(10,1)!="N") lat=-lat;
                    lon=list[0].mid(13,3).toInt()+list[0].mid(17,6).toDouble()/60.0;
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
                    double deg=temp[0].toInt();
                    temp=temp[1].split("'");
                    double min=temp[0].toInt();
                    temp=temp[1].split("q");
                    double sec=temp[0].toDouble();
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
                    sec=temp[0].toDouble();
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
                        lat=lat+temp.toDouble()/60;
                    }
                    else
                    {
                        temp.truncate(temp.indexOf("S")-1);
                        lat=-(lat+temp.toDouble()/60);
                    }
                    position=list[14].split(QChar(0xB0));
                    lon=position.at(0).toInt();
                    temp=position.at(1);
                    if(temp.contains("E",Qt::CaseInsensitive))
                    {
                        temp.truncate(temp.indexOf("E")-1);
                        lon=lon+temp.toDouble()/60;
                    }
                    else
                    {
                        temp.replace("O","W");
                        temp.truncate(temp.indexOf("W")-1);
                        lon=-(lon+temp.toDouble()/60);
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
            if(ortho)
                poi->setNavMode(2);
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
                lon=trackPoint.firstChildElement("Coords").firstChildElement("Lon").firstChild().toText().data().toDouble();
                lat=trackPoint.firstChildElement("Coords").firstChildElement("Lat").firstChild().toText().data().toDouble();
                poiN.sprintf("%.5i",n);
                poiName="I"+poiN;
                POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
                if(ortho)
                    poi->setNavMode(2);
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
        routePath=QDir::currentPath();
        Settings::setSetting("importRouteFolder",routePath);
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Exporter une Route"), routePath, "Routes (*.json *.csv *.txt *.CSV *.gpx *.kml)");
    if(fileName.isEmpty() || fileName.isNull()) return;

    QFile::remove(fileName);
    QFile routeFile(fileName);
    QFileInfo info(routeFile);
    if(!routeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(0,QObject::tr("Export de route"),
             QString(QObject::tr("Impossible de creer le fichier %1")).arg(fileName));
        return;
    }
    Settings::setSetting("importRouteFolder",info.absoluteDir().path());
    if (QString::compare(info.suffix().toLower(),"kml")==0)
    {
        routeFile.close();
        exportRouteFromMenuKML(route,fileName,false);
        return;
    }
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
    QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Import de routes"),
                              tr("Export en cours, veuillez patienter..."));
    waitBox->setStandardButtons(QMessageBox::NoButton);
    waitBox->show();
    QApplication::processEvents();
    if (QString::compare(info.suffix().toLower(),"gpx")==0)
    {
        exportRouteFromMenuGPX(route,fileName,POIonly);
        delete waitBox;
        return;
    }
    if(info.suffix().toLower()=="json")
    {
        QTextStream stream(&routeFile);
        stream<<"[";
        bool first=true;
        if (POIonly)
        {
            if(route->getStartFromBoat())
            {
                stream<<"[\""<<route->getStartDate()<<"\",\""<<
                        QString().sprintf("%.10f",route->getStartLon()*1000.00)<<"\",\""<<
                        QString().sprintf("%.10f",route->getStartLat()*1000.00)<<"\"]";
                first=false;
            }
            foreach(POI * poi, route->getPoiList())
            {
                if(!poi->getUseRouteTstamp()) break;
                if(!first)
                    stream<<",";
                else
                    first=false;
                stream<<"[\""<<poi->getRouteTimeStamp()<<"\",\""<<
                        QString().sprintf("%.10f",poi->getLongitude()*1000.00)<<"\",\""<<
                        QString().sprintf("%.10f",poi->getLatitude()*1000.00)<<"\"]";
            }
        }
        else
        {
            for(int nn=0;nn<route->getLine()->getPoints()->count();++nn)
            {
                if(!first)
                    stream<<",";
                else
                    first=false;
                vlmPoint p=route->getLine()->getPoints()->at(nn);
                stream<<"[\""<<p.eta<<"\",\""<<
                        QString().sprintf("%.10f",p.lon*1000.00)<<"\",\""<<
                        QString().sprintf("%.10f",p.lat*1000.00)<<"\"]";
            }
        }
        stream<<"]"<<endl;
        routeFile.close();
        delete waitBox;
        return;
    }
    QTextStream stream(&routeFile);
    QStringList list;
    list.append("position");
    list.append("heure");
    stream<<list.join(";")<<endl;
    if (POIonly)
    {
        if(route->getStartFromBoat())
        {
            list.clear();
            int deg = (int) fabs(route->getStartLat());
            double min = (fabs(route->getStartLat()) - deg)*60.0;
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
        QList<POI*> poiList=route->getPoiList();
        QListIterator<POI*> i(poiList);
        while(i.hasNext())
        {
            list.clear();
            POI * poi=i.next();

            int deg = (int) fabs(poi->getLatitude());
            double min = (fabs(poi->getLatitude()) - deg)*60.0;
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
            time=time.toUTC();
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
            double min = (fabs(poi.lat) - deg)*60.0;
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
            time=time.toUTC();
            time.setTimeSpec(Qt::UTC);
            list.append(time.toString("dd/MM/yyyy hh:mm:ss"));
            stream<<list.join(";")<<endl;
        }
    }
    routeFile.close();
    delete waitBox;
}
void myCentralWidget::importRouteFromMenuKML(QString fileName,bool toClipboard, bool ortho)
{
    if(!mainW->getSelectedBoat()) return;
    QDomDocument doc;
    if(!toClipboard)
    {
        QFile routeFile(fileName);
        if(!routeFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                 QString(QObject::tr("Impossible d'ouvrir le fichier %1")).arg(fileName));
            return;
        }
        int errorLine,errorColumn;
        QString errorStr;
        if(!doc.setContent(&routeFile,true,&errorStr,&errorLine,&errorColumn))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                                 QString("Erreur ligne %1, colonne %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
            routeFile.close();
            return ;
        }
        routeFile.close();
    }
    else
    {
        QString clip=QApplication::clipboard()->text();
        if(!clip.contains("<kml") || !clip.contains("Placemark")) return;
        int errorLine,errorColumn;
        QString errorStr;
        QByteArray clipBoard;
        clipBoard.append(clip.toUtf8());
        if(!doc.setContent(clipBoard,true,&errorStr,&errorLine,&errorColumn))
        {
            QMessageBox::warning(0,QObject::tr("Lecture de route"),
                                 QString("Erreur ligne %1, colonne %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
            return ;
        }
    }
    QDomElement kml = doc.documentElement();
    QDomElement root = kml.firstChildElement("Document");
    //qWarning()<<kml.tagName()<<root.tagName();
    QString name=root.firstChildElement("name").firstChild().toText().data();
    if (name.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Le nom de la route est invalide"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    ROUTE * route=addRoute();
    route->setName(name.trimmed());
    if(!freeRouteName(name.trimmed(),route))
    {
        int rep = QMessageBox::question (0,
                tr("Importer la route : %1").arg(name),
                tr("Cette route existe deja.\n\nVoulez-vous la remplacer?"),
                QMessageBox::Yes | QMessageBox::No);
        if(rep==QMessageBox::No)
        {
            this->deleteRoute(route);
            return;
        }
        ROUTE * r=NULL;
        for(int n=0;n<this->route_list.count();++n)
        {
            if(name.trimmed()==route_list.at(n)->getName().trimmed())
            {
                r=route_list.at(n);
                break;
            }
        }
        if(r!=NULL)
        {
            r->setTemp(true);
            while(!r->getPoiList().isEmpty())
            {
                POI * poi = r->getPoiList().takeFirst();
                poi->setRoute(NULL);
                poi->setMyLabelHidden(false);
                slot_delPOI_list(poi);
                poi->deleteLater();
            }
            deleteRoute(r);
        }
        QApplication::processEvents();
    }
    update_menuRoute();
    route->setBoat(mainW->getSelectedBoat());
    route->setHidePois(false);
    route->setTemp(true);
    route->setSortPoisByName(false);
    QDomElement placeMark=root.firstChildElement("Placemark");
    while(!placeMark.isNull())
    {
        name=placeMark.firstChildElement("name").firstChild().toText().data();
        QDomElement point=placeMark.firstChildElement("Point");
        route->setStartTimeOption(3);
        route->setStartFromBoat(false);
        route->setStartTime(QDateTime().currentDateTimeUtc());
        if(!point.isNull())
        {
            QStringList position=point.firstChildElement("coordinates").firstChild().toText().data().split(",");
            double lon=position.at(0).toDouble();
            double lat=position.at(1).toDouble();
            POI * poi = slot_addPOI(name,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
            if(ortho)
                poi->setNavMode(2);
            QDomElement extData=placeMark.firstChildElement("ExtendedData");
            if(extData.isNull())
                continue;
            QDomElement poiOption=extData.firstChildElement("navigationMode");
            if(!poiOption.isNull())
            {
                name=poiOption.firstChild().toText().data();
                if(name=="vbVmg")
                    poi->setNavMode(0);
                else if(name=="vmg")
                    poi->setNavMode(1);
                else
                    poi->setNavMode(2);
            }
            else
                poi->setNavMode(ortho?2:0);
            poi->setRoute(route);
            poiOption=extData.firstChildElement("sequence");
            if(!poiOption.isNull())
            {
                name=poiOption.firstChild().toText().data();
                poi->setSequence(name.toInt());
            }
            else
                route->setSortPoisByName(true);
            QDomElement routeData=extData.firstChildElement("route");
            if(!routeData.isNull())
            {
                QDomElement routeOption=routeData.firstChildElement("startFromBoat");
                if(!routeOption.isNull())
                {
                    route->setStartFromBoat(routeOption.firstChild().toText().data()=="true");
                }
                routeOption=routeData.firstChildElement("startTimeOption");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    if(name=="lastBoatUpdateTime")
                        route->setStartTimeOption(1);
                    else if (name=="gribDate")
                        route->setStartTimeOption(2);
                    else if (name=="fixedDate")
                        route->setStartTimeOption(3);
                }
                routeOption=routeData.firstChildElement("startDateTime");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    QDateTime sd=QDateTime().fromString(name,"yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'");
                    sd.setTimeSpec(Qt::UTC);
                    route->setStartTime(sd);
                }
                routeOption=routeData.firstChildElement("speedTackGybe");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    name.remove("%");
                    route->setSpeedLossOnTack(name.toDouble()/100.0);
                }
                routeOption=routeData.firstChildElement("color");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    QStringList colors=name.split(" ");
                    QColor color;
                    color.setRed(colors.at(0).toInt());
                    color.setGreen(colors.at(1).toInt());
                    color.setBlue(colors.at(2).toInt());
                    route->setColor(color);
                }
                routeOption=routeData.firstChildElement("width");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    route->setWidth(name.toDouble());
                }
                routeOption=routeData.firstChildElement("detectCoast");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    route->setDetectCoasts(name=="true");
                }
                routeOption=routeData.firstChildElement("sortPoisbyName");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    route->setSortPoisByName(name=="true");
                }
                routeOption=routeData.firstChildElement("hideIntermediaryPois");
                if(!routeOption.isNull())
                {
                    name=routeOption.firstChild().toText().data();
                    route->setHidePois(name=="true");
                }
                if(mainW->getSelectedBoat() && mainW->getSelectedBoat()->getType()==BOAT_REAL)
                {
                    if(!((boatReal *)mainW->getSelectedBoat())->gpsIsRunning())
                    {
                        routeOption=routeData.firstChildElement("ownship");
                        if(!routeOption.isNull())
                        {
                            position=routeOption.firstChild().toText().data().split(" ");
                            this->slot_moveBoat(position.at(1).toDouble(),position.at(0).toDouble());
                        }
                    }
                }
            }
        }
        placeMark=placeMark.nextSiblingElement("Placemark");
    }
    QApplication::processEvents();
    route->setTemp(false);
    route->slot_recalculate();
}

void myCentralWidget::exportRouteFromMenuKML(ROUTE * route,QString fileName,bool toClipboard)
{
    if(route==NULL) return;
    QList<POI *> pList=route->getPoiList();
    if(pList.isEmpty()) return;
    QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Import de routes"),
                              tr("Export en cours, veuillez patienter..."));
    waitBox->setStandardButtons(QMessageBox::NoButton);
    waitBox->show();
    QApplication::processEvents();
    if(!toClipboard)
        QFile::remove(fileName);
    else
        QApplication::clipboard()->clear();
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));
    QDomElement kml = doc.createElement("kml");
    doc.appendChild(kml);

    kml.setAttribute("xmlns","http://www.opengis.net/kml/2.2");
    kml.setAttribute("xmlns:gx","http://www.google.com/kml/ext/2.2");
    kml.setAttribute("xmlns:kml","http://www.opengis.net/kml/2.2");
    kml.setAttribute("xmlns:atom","http://www.w3.org/2005/Atom");
    kml.setAttribute("xmlns:vlm","http://www.virtual-loup-de-mer.org");
    QDomElement d1 = doc.createElement("Document");
    kml.appendChild(d1);
    QDomElement d2=doc.createElement("name");
    d1.appendChild(d2);
    QDomText d3=doc.createTextNode(route->getName());
    d2.appendChild(d3);
    QDomText t;
    for (int i=0;i<pList.count();++i)
    {
        QDomElement qd1=doc.createElement("Placemark");
        d1.appendChild(qd1);
        QDomElement qd2=doc.createElement("name");
        qd1.appendChild(qd2);
        t=doc.createTextNode(pList.at(i)->getName());
        qd2.appendChild(t);


        QDomElement qd3=doc.createElement("ExtendedData");
        qd1.appendChild(qd3);

        QDomElement qd4=doc.createElement("vlm:sequence");
        qd3.appendChild(qd4);
        if(route->getSortPoisByName())
        {
            t=doc.createTextNode(QString().sprintf("%04d",i));
            qd4.appendChild(t);
        }
        else
        {
            t=doc.createTextNode(QString().sprintf("%04d",pList.at(i)->getSequence()));
            qd4.appendChild(t);
        }

        qd4=doc.createElement("Data");
        qd3.appendChild(qd4);
        qd4.setAttribute("name","eta");
        QDomElement qd5=doc.createElement("value");
        qd4.appendChild(qd5);
        if(pList.at(i)->getRouteTimeStamp()!=-1)
            t=doc.createTextNode(QDateTime().fromTime_t(pList.at(i)->getRouteTimeStamp()).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
        else
            t=doc.createTextNode("N/A");
        qd5.appendChild(t);

        qd4=doc.createElement("vlm:navigationMode");
        qd3.appendChild(qd4);
        switch (pList.at(i)->getNavMode())
        {
            case 0:
                t=doc.createTextNode("vbVmg");
                break;
            case 1:
                t=doc.createTextNode("vmg");
                break;
            default:
                t=doc.createTextNode("ortho");
                break;
        }
        qd4.appendChild(t);
        if(grib && grib->isOk() && pList.at(i)->getRouteTimeStamp()!=-1)
        {
            double speed,angle;
            if(grib->getInterpolatedValue_byDates(pList.at(i)->getLongitude(), pList.at(i)->getLatitude(),
                                                  pList.at(i)->getRouteTimeStamp(),&speed,&angle,INTERPOLATION_DEFAULT))
            {
                angle=radToDeg(angle);
                qd4=doc.createElement("Data");
                qd3.appendChild(qd4);
                qd4.setAttribute("name","TWS");
                qd5=doc.createElement("value");
                qd4.appendChild(qd5);
                t=doc.createTextNode(QString().sprintf("%.2f kts",speed));
                qd5.appendChild(t);

                qd4=doc.createElement("Data");
                qd3.appendChild(qd4);
                qd4.setAttribute("name","TWD");
                qd5=doc.createElement("value");
                qd4.appendChild(qd5);
                t=doc.createTextNode(QString().sprintf("%.2f",angle)+tr("deg"));
                qd5.appendChild(t);
                if(grib->getInterpolatedValueCurrent_byDates(pList.at(i)->getLongitude(), pList.at(i)->getLatitude(),
                                                      pList.at(i)->getRouteTimeStamp(),&speed,&angle,INTERPOLATION_DEFAULT))
                {
                    qd4=doc.createElement("Data");
                    qd3.appendChild(qd4);
                    qd4.setAttribute("name","CS");
                    qd5=doc.createElement("value");
                    qd4.appendChild(qd5);
                    t=doc.createTextNode(QString().sprintf("%.2f kts",speed));
                    qd5.appendChild(t);

                    qd4=doc.createElement("Data");
                    qd3.appendChild(qd4);
                    qd4.setAttribute("name","CD");
                    qd5=doc.createElement("value");
                    qd4.appendChild(qd5);
                    t=doc.createTextNode(QString().sprintf("%.2f",angle)+tr("deg"));
                    qd5.appendChild(t);
                }
                if(!route->getFrozen())
                {
                    QList<QList<double> > * roadBook=route->getRoadMap();
                    for(int ind=0;ind<roadBook->count();++ind)
                    {
                        if(roadBook->at(ind).at(0)==pList.at(i)->getRouteTimeStamp())
                        {
                            if(roadBook->at(ind).at(4)!=-1)
                            {
                                qd4=doc.createElement("Data");
                                qd3.appendChild(qd4);
                                qd4.setAttribute("name","BS");
                                qd5=doc.createElement("value");
                                qd4.appendChild(qd5);
                                t=doc.createTextNode(QString().sprintf("%.2f kts",roadBook->at(ind).at(4)));
                                qd5.appendChild(t);
                                qd4=doc.createElement("Data");
                                qd3.appendChild(qd4);
                                qd4.setAttribute("name","HDG");
                                qd5=doc.createElement("value");
                                qd4.appendChild(qd5);
                                t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(ind).at(15))+tr("deg"));
                                qd5.appendChild(t);
                                qd4=doc.createElement("Data");
                                qd3.appendChild(qd4);
                                qd4.setAttribute("name","CNM");
                                qd5=doc.createElement("value");
                                qd4.appendChild(qd5);
                                t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(ind).at(16))+tr("deg"));
                                qd5.appendChild(t);
                                qd4=doc.createElement("Data");
                                qd3.appendChild(qd4);
                                qd4.setAttribute("name","DNM");
                                qd5=doc.createElement("value");
                                qd4.appendChild(qd5);
                                t=doc.createTextNode(QString().sprintf("%.2f NM",roadBook->at(ind).at(10)));
                                qd5.appendChild(t);
                            }
                            break;
                        }
                    }
                }
            }
        }

        if(i==0)
        {
            QDomElement qd7=doc.createElement("vlm:route");
            qd3.appendChild(qd7);
            QDomElement qd8=doc.createElement("startFromBoat");
            qd7.appendChild(qd8);
            t=doc.createTextNode(route->getStartFromBoat()?"true":"false");
            qd8.appendChild(t);
            qd8=doc.createElement("startTimeOption");
            qd7.appendChild(qd8);
            if(route->getStartTimeOption()==1)
                t=doc.createTextNode("lastBoatUpdateTime");
            else if(route->getStartTimeOption()==1)
                t=doc.createTextNode("fixedDate");
            else
                t=doc.createTextNode("gribDate");
            qd8.appendChild(t);
            qd8=doc.createElement("startDateTime");
            qd7.appendChild(qd8);
            t=doc.createTextNode(QDateTime().fromTime_t(route->getStartDate()).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
            qd8.appendChild(t);
            qd8=doc.createElement("eta");
            qd7.appendChild(qd8);
            if(route->getHas_eta())
                t=doc.createTextNode(QDateTime().fromTime_t(route->getEta()).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
            else
                t=doc.createTextNode("N/A");
            qd8.appendChild(t);
            qd8=doc.createElement("speedTackGybe");
            qd7.appendChild(qd8);
            t=doc.createTextNode(QString().sprintf("%.2f",route->getSpeedLossOnTack()*100.0)+"%");
            qd8.appendChild(t);
            qd8=doc.createElement("color");
            qd7.appendChild(qd8);
            t=doc.createTextNode(QString().sprintf("%03d %03d %03d",route->getColor().red(),route->getColor().green(),route->getColor().blue()));
            qd8.appendChild(t);
            qd8=doc.createElement("width");
            qd7.appendChild(qd8);
            t=doc.createTextNode(QString().sprintf("%.2f",route->getWidth()));
            qd8.appendChild(t);
            qd8=doc.createElement("detectCoast");
            qd7.appendChild(qd8);
            t=doc.createTextNode(route->getDetectCoasts()?"true":"false");
            qd8.appendChild(t);
            qd8=doc.createElement("sortPoisByName");
            qd7.appendChild(qd8);
            t=doc.createTextNode(route->getSortPoisByName()?"true":"false");
            qd8.appendChild(t);
            qd8=doc.createElement("hideIntermediaryPois");
            qd7.appendChild(qd8);
            t=doc.createTextNode(route->getHidePois()?"true":"false");
            qd8.appendChild(t);
#if 0 //unflag to send roadBook in kml
            if(!route->getFrozen())
            {
                qd8=doc.createElement("vlm:roadBook");
                qd3.appendChild(qd8);
                QList<QList<double> > * roadBook=route->getRoadMap();
                for (int i=0;i<roadBook->count();++i)
                {
                    if(roadBook->at(i).at(4)==-1) continue;
                    QDomElement qd9=doc.createElement("roadPoint");
                    qd8.appendChild(qd9);
                    QDomElement qd10=doc.createElement("coordinates");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.14f,%.14f,0.0",
                                                           roadBook->at(i).at(13),
                                                           roadBook->at(i).at(14)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("eta");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QDateTime().fromTime_t(qRound(roadBook->at(i).at(0))).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
                    qd10.appendChild(t);

                    qd10=doc.createElement("bs");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(4)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("hdg");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(15)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("cnm");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(16)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("dnm");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(10)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("engineUsed");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(roadBook->at(i).at(12)==-1?"false":"true");
                    qd10.appendChild(t);

                    qd10=doc.createElement("twd");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(6)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("tws");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(7)));
                    qd10.appendChild(t);

                    qd10=doc.createElement("twa");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",roadBook->at(i).at(8)));
                    qd10.appendChild(t);

                    double twa=qAbs(roadBook->at(i).at(8));
                    double tws=roadBook->at(i).at(7);
                    double Y=90-twa;
                    double a=tws*cos(degToRad(Y));
                    double b=tws*sin(degToRad(Y));
                    double bb=b+roadBook->at(i).at(4);
                    double aws=sqrt(a*a+bb*bb);
                    double awa=90-radToDeg(atan(bb/a));
                    if(roadBook->at(i).at(8)<0) awa = -awa;

                    qd10=doc.createElement("aws");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",aws));
                    qd10.appendChild(t);

                    qd10=doc.createElement("awa");
                    qd9.appendChild(qd10);
                    t=doc.createTextNode(QString().sprintf("%.2f",awa));
                    qd10.appendChild(t);

                    qd10=doc.createElement("targetPoi");
                    qd9.appendChild(qd10);
                    QDomElement qd11=doc.createElement("name");
                    qd10.appendChild(qd11);
                    t=doc.createTextNode(route->getPoiList().at((int)roadBook->at(i).at(9))->getName());
                    qd11.appendChild(t);
                    qd11=doc.createElement("coordinates");
                    qd10.appendChild(qd11);
                    t=doc.createTextNode(QString().sprintf("%.14f,%.14f,0.0",
                                                           roadBook->at(i).at(1),
                                                           roadBook->at(i).at(2)));
                    qd11.appendChild(t);
                }
            }
#endif
        }

        qd5=doc.createElement("Point");
        qd1.appendChild(qd5);
        QDomElement qd6=doc.createElement("coordinates");
        qd5.appendChild(qd6);
        t=doc.createTextNode(QString().sprintf("%.14f,%.14f,0.0",
                                               pList.at(i)->getLongitude(),
                                               pList.at(i)->getLatitude()));
        qd6.appendChild(t);
    }
    QDomElement qd1=doc.createElement("Placemark");
    d1.appendChild(qd1);
    QDomElement qd2=doc.createElement("name");
    qd1.appendChild(qd2);
    t=doc.createTextNode(route->getName()+" path");
    qd2.appendChild(t);
    QDomElement qd3=doc.createElement("gx:Track");
    qd1.appendChild(qd3);
    if(route->getFrozen())
    {
        QList<vlmPoint> * listPoint=route->getLine()->getPoints();
        for (int i=0;i<listPoint->count();++i)
        {
            QDomElement qd4=doc.createElement("when");
            qd3.appendChild(qd4);
            t=doc.createTextNode(QDateTime().fromTime_t(listPoint->at(i).eta).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
            qd4.appendChild(t);

        }
        for (int i=0;i<listPoint->count();++i)
        {
            QDomElement qd4=doc.createElement("gx:coord");
            qd3.appendChild(qd4);
            t=doc.createTextNode(QString().sprintf("%.14f %.14f 0.0 ",
                                                   listPoint->at(i).lon,
                                                   listPoint->at(i).lat));
            qd4.appendChild(t);
        }
    }
    else
    {
        QList<QList<double> > * roadBook=route->getRoadMap();
        for (int i=0;i<roadBook->count();++i)
        {
            if(roadBook->at(i).at(4)==-1) continue;
            QDomElement qd4=doc.createElement("when");
            qd3.appendChild(qd4);
            t=doc.createTextNode(QDateTime().fromTime_t(qRound(roadBook->at(i).at(0))).toUTC().toString("yyyy-MM-ddThh:mm:ssZ"));
            qd4.appendChild(t);

        }
        for (int i=0;i<roadBook->count();++i)
        {
            if(roadBook->at(i).at(4)==-1) continue;
            QDomElement qd4=doc.createElement("gx:coord");
            qd3.appendChild(qd4);
            t=doc.createTextNode(QString().sprintf("%.14f %.14f 0.0 ",
                                    roadBook->at(i).at(13),
                                    roadBook->at(i).at(14)));
            qd4.appendChild(t);
        }
    }
    if(!toClipboard)
    {
        QTextStream out;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
            return ;
        out.setDevice(&file);
        doc.save(out,4);
    }
    else
    {
        QString buf;
        QTextStream out(&buf);
        doc.save(out,4);
        out<<endl;
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
        clipboard->setText(buf);
    }
    delete waitBox;
}

void myCentralWidget::exportRouteFromMenuGPX(ROUTE * route,QString fileName,bool POIonly)
{
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
    list.append("<?xml version=\"1.0\"?>");
    list.append("<gpx");
    list.append(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
    list.append(" xmlns=\"http://www.topografix.com/GPX/1/0\"");
    list.append(" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">");
    list.append("<rte>");
    list.append("<name>"+route->getName()+"</name>");
    stream<<list.join("\n")<<endl;
    if(route->getStartFromBoat())
    {
        list.clear();
        QString latitude=QString::number(route->getStartLat());
        QString longitude=QString::number(route->getStartLon());
        list.append("<rtept lat=\""+latitude+"\" lon=\""+longitude+"\">");
        QDateTime time;
        time.setTime_t(route->getStartDate());
        time.toUTC();
        time.setTimeSpec(Qt::UTC);
        list.append( "<time>"+time.toString("yyyy-MM-ddThh:mm:ssZ")+"</time>");
        list.append( "</rtept>");
        stream<<list.join("\n")<<endl;
    }
    if (POIonly)
    {
        QList<POI*> poiList=route->getPoiList();
        QListIterator<POI*> i(poiList);
        while(i.hasNext())
        {
            list.clear();
            POI * poi=i.next();
            QString latitude=QString::number(poi->getLatitude());
            QString longitude=QString::number(poi->getLongitude());
            list.append("<rtept lat=\""+latitude+"\" lon=\""+longitude+"\">");
            QDateTime time;
            time.setTime_t(route->getStartDate());
            time=time.toUTC();
            time.setTimeSpec(Qt::UTC);
            list.append( "<time>"+time.toString("yyyy-MM-ddThh:mm:ssZ")+"</time>");
            list.append( "</rtept>");
            stream<<list.join("\n")<<endl;
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
            QString latitude=QString::number(poi.lat);
            QString longitude=QString::number(poi.lon);
            list.append("<rtept lat=\""+latitude+"\" lon=\""+longitude+"\">");
            QDateTime time;
            time.setTime_t(route->getStartDate());
            time=time.toUTC();
            time.setTimeSpec(Qt::UTC);
            list.append( "<time>"+time.toString("yyyy-MM-ddThh:mm:ssZ")+"</time>");
            list.append( "</rtept>");
            stream<<list.join("\n")<<endl;
        }
    }
    stream<<"</rte>"<<endl;
    stream<<"</gpx>"<<endl;
    routeFile.close();
}

void myCentralWidget::withdrawRouteFromBank(QString routeName,QList<QVariant> details)
{
    bool ortho=true;
    ROUTE * route=addRoute();
    if (routeName.isEmpty() || !freeRouteName(routeName.trimmed(),route))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Ce nom est deja utilise ou invalide"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        deleteRoute(route);
        return;
    }
    QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Import de routes"),
                              tr("Import en cours, veuillez patienter..."));
    waitBox->setStandardButtons(QMessageBox::NoButton);
    waitBox->show();
    QApplication::processEvents();
    route->setName(routeName);
    update_menuRoute();
    route->setBoat(mainW->getSelectedBoat());
    route->setStartFromBoat(false);
    route->setStartTimeOption(3);
    route->setColor(QColor(227,142,42,255));
    route->setImported();
    route->setFrozen(true);
    route->setTemp(true);
    QMap<int,QPointF> points;
    for(int n=0;n<details.count();++n)
    {
        QVariant detail=details.at(n);
        QVariantList FTpoint=detail.toList();
        time_t eta=FTpoint.at(0).toInt();
        double lon=FTpoint.at(1).toDouble()/1000.00;
        double lat=FTpoint.at(2).toDouble()/1000.00;
        points.insert(eta,QPointF(lon,lat));
    }
    QDateTime start=QDateTime().fromTime_t(points.begin().key());
    route->setStartTime(start);
    QMapIterator<int,QPointF> p(points);
    int nPoi=0;
    while(p.hasNext())
    {
        ++nPoi;
        int eta=p.next().key();
        double lon=p.value().x();
        double lat=p.value().y();
        QString poiName=route->getName()+QString().sprintf("%.5i",nPoi);
        POI * poi = slot_addPOI(poiName,0,lat,lon,-1,false,false,mainW->getSelectedBoat());
        if(ortho)
            poi->setNavMode(2);
        poi->setRoute(route);
        poi->setRouteTimeStamp(eta);
    }
    route->setHidePois(true);
    route->setImported();
    route->setTemp(false);
    route->setFrozen2(false);//calculate only once and relock immediately
    route->setFrozen2(true);
    delete waitBox;

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
        delete route_editor;
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
        delete route_editor;
        /* Simplifying may remove POIs, which may cause a crash if we
         * got here from a POI context menu and that POI gets removed.
         * therefore we delay the processing until Qt is idle. */
        routeSimplify = route;
        QTimer::singleShot (0, this, SLOT (slot_routeTimer()));
    }
    //route->slot_recalculate();
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
void myCentralWidget::treatRoute(ROUTE* route)
{
    update_menuRoute();
    route->slot_recalculate();
    QApplication::processEvents();
    if((route->getSimplify() || route->getOptimize()) && !route->isBusy())
    {
        this->abortRequest=false;
        bool simplify=route->getSimplify();
        bool optimize=route->getOptimize();
        bool poiShown=route->getHidePois();
        route->setHidePois(false);
        bool detectCoast=route->getDetectCoasts();
        route->setDetectCoasts(false);
        route->setSimplify(false);
        route->setOptimize(false);
        if(route->getFrozen() || (simplify && !route->getHas_eta()))
            QMessageBox::critical(0,QString(QObject::tr("Simplification/Optimisation de route")),QString(QObject::tr("Cette operation est impossible pour une route figee ou une route sans ETA")));
        else if(route->getUseVbvmgVlm())
            QMessageBox::critical(0,QString(QObject::tr("Simplification/Optimisation de route")),QString(QObject::tr("Cette operation est impossible si le mode de calcul VBVMG est celui de VLM")));
        else
        {
            int poiCt=route->getPoiList().count();
            time_t ref_eta=route->getEta();
            if(simplify)
            {
                doSimplifyRoute(route);
                int nbDel=poiCt-route->getPoiList().count();
                int diff=(ref_eta-route->getEta())/60;
                QString result;
                if(diff<0)
                    result=QString().setNum(-diff)+tr(" minutes perdues, ");
                else
                    result=QString().setNum(diff)+tr(" minutes gagnees(!), ");
                result+=QString().setNum(nbDel)+tr(" POIs supprimes sur ")+QString().setNum(poiCt);
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
                QMessageBox mb(0);
                mb.setText(result);
                mb.setWindowTitle(QObject::tr("Resultat de la simplification"));
                mb.setIcon(QMessageBox::Information);
                QPushButton *optim = mb.addButton(tr("Optimiser"),QMessageBox::YesRole);
                mb.addButton(QMessageBox::Close);
                mb.exec();
                if(mb.clickedButton()==optim)
                    optimize=true;
            }
            if(optimize)
            {
                POI*    lastReachedPoi  = route->getLastReachedPoi();
                time_t  ref_eta2        = lastReachedPoi ? lastReachedPoi->getRouteTimeStamp() : route->getEta();
                QString ref_lastPoiName = lastReachedPoi ? lastReachedPoi->getName() : (QString ("<em>%1</em>").arg (tr("aucun")));
                poiCt=route->getPoiList().count();
                QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Optimisation en cours"),
                                          tr("Veuillez patienter..."));
                waitBox->setStandardButtons(QMessageBox::Abort);
                connect(waitBox->button(QMessageBox::Abort),SIGNAL(clicked()),this,SLOT(slot_abortRequest()));
                this->abortRequest=false;
                connect(waitBox,SIGNAL(rejected()),this,SLOT(slot_abortRequest()));
                waitBox->show();
                //bad trick because setFixWidth() is not working
                QSpacerItem* dummy = new QSpacerItem(300,0,QSizePolicy::Minimum, QSizePolicy::Expanding);
                QGridLayout * lay= (QGridLayout*)waitBox->layout();
                lay->addItem(dummy,lay->rowCount(),0,1,lay->columnCount());
                bool hasWP=false;
                for (int poiN=0;poiN<route->getPoiList().count()-1;++poiN)
                {
                    if(route->getPoiList().at(poiN)->getIsWp())
                    {
                        hasWP=true;
                        route->getPoiList().at(poiN)->setWasWP(true);
                        break;
                    }
                }
                for (int maxLoop=0;maxLoop<10;++maxLoop)
                {
                    if(abortRequest) break;
                    POI*    lastReachedPoi = route->getLastReachedPoi();
                    if(lastReachedPoi == NULL) break;
                    time_t  ref_eta3       = lastReachedPoi->getRouteTimeStamp();
                    double  ref_remain     = route->getRemain();
                    int     nPois          = route->getPoiList().count();
                    for (int poiN = route->getStartFromBoat() ? 0 : 1;poiN<route->getPoiList().count()-1;++poiN)
                    {
                        if(abortRequest) break;
                        POI*    poi = route->getPoiList().at(poiN);
                        if (!poi->getHas_eta()) break;
                        if(poi->getNotSimplificable()) continue;
                        poi->slot_finePosit(true);
                    }
                    if(abortRequest) break;
                    if ((route->getLastReachedPoi() == lastReachedPoi)
                        && ((lastReachedPoi->getRouteTimeStamp() > ref_eta3)
                        || ((lastReachedPoi->getRouteTimeStamp() == ref_eta3) && (route->getRemain() > ref_remain))))
                        qWarning()<<"wrong optimization!!";
                    if (route->getHas_eta() && simplify) {
                        time_t  ref_eta4 = route->getEta();
                        doSimplifyRoute(route,true);
                        if(route->getEta()>ref_eta4)
                            qWarning()<<"wrong simplification!!";
                    }
                    if ((lastReachedPoi == route->getLastReachedPoi())
                        && (ref_eta3    == lastReachedPoi->getRouteTimeStamp())
                        && (ref_remain  <= (route->getRemain() + 0.001)) // Should probably be configurable...
                        && (nPois       == route->getPoiList().count()))
                        break;
#if 0
                    qWarning()<<maxLoop<<QDateTime().fromTime_t(ref_eta3).toUTC().toString("dd/MM/yy hh:mm:ss")<<
                                QDateTime().fromTime_t(route->getEta()).toUTC().toString("dd/MM/yy hh:mm:ss")<<
                                nPois<<"/"<<route->getPoiList().count();
#endif
                }
                if(hasWP)
                {
                    for (int poiN=0;poiN<route->getPoiList().count()-1;++poiN)
                    {
                        if(route->getPoiList().at(poiN)->getWasWP())
                        {
                            hasWP=false;
                            route->getPoiList().at(poiN)->setWasWP(false);
                            route->getPoiList().at(poiN)->slot_setWP();
                            break;
                        }
                    }
                }
                int     nbDel = poiCt-route->getPoiList().count();
                QString result;
                if (route->getHas_eta()) {
                    int diff   = (ref_eta2-route->getEta())/60;
                    if(diff<0)
                        result = QString().setNum(-diff)+tr(" minutes perdues");
                    else
                        result = QString().setNum(diff)+tr(" minutes gagnees");
                    result+=", "+QString().setNum(nbDel)+tr(" POIs supprimes sur ")+QString().setNum(poiCt);
                    QDateTime before=before.fromTime_t(ref_eta2);
                    before=before.toUTC();
                    before.setTimeSpec(Qt::UTC);
                    QDateTime after=after.fromTime_t(route->getEta());
                    after=after.toUTC();
                    after.setTimeSpec(Qt::UTC);
                    result=result+"<br>"+tr("ETA avant optimisation: ")+before.toString("dd/MM/yy hh:mm:ss");
                    result=result+"<br>"+tr("ETA apres optimisation: ")+after.toString("dd/MM/yy hh:mm:ss");
                } else {
                    result             = tr("Optimisation terminee");

                    QDateTime   before = QDateTime::fromTime_t (ref_eta2).toUTC();
                    before.setTimeSpec (Qt::UTC);
                    result             = result + "<br>" + (QString (tr("Dernier POI avant optimisation: %1 (ETA: %2)"))
                                                            .arg (ref_lastPoiName, before.toString ("dd/MM/yy hh:mm:ss")));

                    POI*    lastReachedPoi = route->getLastReachedPoi();
                    if (lastReachedPoi != NULL) {
                        QDateTime   after = QDateTime::fromTime_t (lastReachedPoi->getRouteTimeStamp()).toUTC();
                        after.setTimeSpec (Qt::UTC);
                        result = result + "<br>" + (QString (tr("Dernier POI apres optimisation: %1 (ETA: %2)"))
                                                    .arg (lastReachedPoi->getName(), after.toString ("dd/MM/yy hh:mm:ss")));
                    } else {
                        result = result + "<br>" + (QString (tr("Dernier POI apres optimisation: %1 (ETA: %2)"))
                                                    .arg (QString ("<em>%1</em>").arg (tr("aucun")), "--/--/-- --:--:--"));
                    }
                }
                waitBox->close();
                delete waitBox;
                QMessageBox::information(0,QString(QObject::tr("Resultat de l'optimisation")),result);
            }
            route->setDetectCoasts(detectCoast);
            if(detectCoast)
                route->slot_recalculate();
        }
        route->setHidePois(poiShown);
    }
}
void myCentralWidget::slot_abortRequest()
{
    this->abortRequest=true;
}

void myCentralWidget::doSimplifyRoute(ROUTE * route, bool fast)
{
    bool strongSimplify=Settings::getSetting("strongSimplify",1).toInt()==1;
    route->setSimplify(true);
    int firstPOI=1;
    if(route->getStartFromBoat())
        firstPOI=0;
    QList<POI*> pois=route->getPoiList();
    int ref_nbPois=pois.count();
    time_t ref_eta=route->getEta();
    double ref_remain=route->getRemain();
    int nbDel=0;
    int phase=1;
    QProgressDialog p("","",1,ref_nbPois-2);
    if(!fast)
    {
        p.setWindowTitle(tr("Simplification en cours"));
        p.setAutoClose(false);
        this->abortRequest=false;
        p.setCancelButtonText(tr("Abandonner"));
        connect(&p,SIGNAL(canceled()),this,SLOT(slot_abortRequest()));
        p.setLabelText(tr("Phase ")+QString().setNum(phase));
        p.setMinimumDuration (0);
        p.setFixedWidth(300);
    }
    else
        p.close();
    bool notFinished=true;
    time_t bestEta=ref_eta;
    double bestRemain=ref_remain;

    while(notFinished && !abortRequest)
    {
        notFinished=false;
        pois=route->getPoiList();
        if(!fast)
        {
            p.setMaximum(pois.count()-2);
            p.setValue(0);
        }
        for (int n=firstPOI;n<pois.count()-2;++n)
        {
            if(abortRequest) break;
            POI *poi=pois.at(n);
            if(poi->getNotSimplificable()) continue;
            poi->setRoute(NULL);
            QApplication::processEvents();
            if(!route->getHas_eta())
                poi->setRoute(route);
            else if (route->getEta()<bestEta || (route->getEta()==bestEta && (strongSimplify || route->getRemain()<=bestRemain)))
            {
                bestEta=route->getEta();
                bestRemain=route->getRemain();
                notFinished=true;
                slot_delPOI_list(poi);
                delete poi;
                ++nbDel;
            }
            else
                poi->setRoute(route);
            if(!fast)
                p.setValue(n);
            QApplication::processEvents();
        }
        if(abortRequest) break;
        pois=route->getPoiList();
        ++phase;
        if(!fast)
        {
            p.setLabelText(tr("Phase ")+QString().setNum(phase));
            p.setMaximum(pois.count()-2);
            p.setValue(0);
            p.setValue(pois.count()-2);
        }
        else if(!notFinished) break;
        for (int n=pois.count()-2;n>=firstPOI;--n)
        {
            if(abortRequest) break;
            POI *poi=pois.at(n);
            if(poi->getNotSimplificable()) continue;
            poi->setRoute(NULL);
            QApplication::processEvents();
            if(!route->getHas_eta())
                poi->setRoute(route);
            else if (route->getEta()<bestEta || (route->getEta()==bestEta && (strongSimplify || route->getRemain()<=bestRemain)))
            {
                bestEta=route->getEta();
                bestRemain=route->getRemain();
                notFinished=true;
                slot_delPOI_list(poi);
                poi->deleteLater();
                ++nbDel;
            }
            else
                poi->setRoute(route);
            if(!fast)
                p.setValue(n);
            QApplication::processEvents();
        }
        if(fast)
        {
            if(notFinished)
                continue;
            else
                break;
        }
        if(abortRequest) break;
        pois=route->getPoiList();
        ++phase;
        p.setLabelText(tr("Phase ")+QString().setNum(phase));
        p.setMaximum(pois.count()-2);
        p.setValue(0);

        for (int n=firstPOI;n<pois.count()-3;++n)
        {
            if(abortRequest) break;
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
            else if (route->getEta()<bestEta || (route->getEta()==bestEta && (strongSimplify || route->getRemain()<=bestRemain)))
            {
                bestEta=route->getEta();
                bestRemain=route->getRemain();
                notFinished=true;
                slot_delPOI_list(poi1);
                poi1->deleteLater();
                slot_delPOI_list(poi2);
                poi2->deleteLater();
                nbDel=nbDel+2;
                n=firstPOI-1;
                pois=route->getPoiList();
                p.setMaximum(pois.count()-2);
                p.setValue(0);
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
        if(abortRequest) break;
        ++phase;
        p.setLabelText(tr("Phase ")+QString().setNum(phase));
        pois=route->getPoiList();
        p.setMaximum(pois.count()-2);
        p.setValue(0);

        for (int n=firstPOI;n<pois.count()-4;++n)
        {
            if(abortRequest) break;
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
            else if (route->getEta()<bestEta || (route->getEta()==bestEta && (strongSimplify || route->getRemain()<=bestRemain)))
            {
                bestEta=route->getEta();
                bestRemain=route->getRemain();
                notFinished=true;
                slot_delPOI_list(poi1);
                poi1->deleteLater();
                slot_delPOI_list(poi2);
                poi2->deleteLater();
                slot_delPOI_list(poi3);
                poi3->deleteLater();
                nbDel=nbDel+3;
                n=firstPOI-1;
                pois=route->getPoiList();
                p.setMaximum(pois.count()-2);
                p.setValue(0);
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
    if(!fast)
        p.close();
    route->setSimplify(false);
    route->slot_recalculate();
}

void myCentralWidget::setPilototo(QList<POI *> poiList)
{
    if(!poiList.isEmpty())
    {
        mainW->setPilototoFromRoute(poiList);
    }
}

void myCentralWidget::slot_editRoutage(ROUTAGE * routage,bool createMode,POI *endPOI)
{
    DialogRoutage *routage_editor=new DialogRoutage(routage,this,endPOI);
    if(routage_editor->exec()!=QDialog::Accepted)
    {
        delete routage_editor;
        if(createMode || routage->getIsNewPivot())
        {
            bool b=routage->getIsNewPivot();
            delete routage;
            routage_list.removeAll(routage);
            routage=NULL;
            nbRoutage--;
            if(b)
                update_menuRoutage();
        }
    }
    else
    {
        delete routage_editor;
        update_menuRoutage();
        if(routage && (createMode || routage->getIsNewPivot()))
            routage->calculate();
        else if(routage && routage->getI_iso() && !routage->getI_done() && !routage->isConverted())
            routage->calculateInverse();
        else if(routage && routage->getI_iso() && routage->getI_done() && !routage->isConverted())
            routage->showIsoRoute();
    }
}
void myCentralWidget::deleteRoute(ROUTE * route)
{
    if(this->compassRoute==route)
        this->setCompassFollow(NULL);
    route_list.removeAll(route);
    update_menuRoute();
    route->deleteLater();
}
void myCentralWidget::slot_deleteRoute()
{
    QAction *sender=(QAction*)QObject::sender();
    ROUTE *route=reinterpret_cast<class ROUTE *>(qvariant_cast<void*>(sender->data()));
    if(route==NULL) return;
    myDeleteRoute(route);
}
void myCentralWidget::myDeleteRoute(ROUTE * route)
{
    if(route->isBusy()) return ;
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
            poi->deleteLater();
        }
    }
    deleteRoute(route);
}
void myCentralWidget::slot_deleteRoutage()
{
    QAction *sender=(QAction*)QObject::sender();
    ROUTAGE *routage=reinterpret_cast<class ROUTAGE *>(qvariant_cast<void*>(sender->data()));
    if(routage==NULL) return;
    if(routage->isRunning()) return;
    int rep = QMessageBox::question (0,
            tr("Detruire le routage : %1?").arg(routage->getName()),
            tr("La destruction d'un routage est definitive."),
            QMessageBox::Yes | QMessageBox::Cancel);
    if (rep == QMessageBox::Cancel) return;
    deleteRoutage(routage);
}

void myCentralWidget::deleteRoutage(ROUTAGE * routage, ROUTE * route)
{
    if(routage)
    {
        routage_list.removeAll(routage);
        update_menuRoutage();
        if(route!=NULL)
        {
            routeSimplify=route;
            routeSimplify->setSimplify(true);
            connect(routage,SIGNAL(destroyed()),this,SLOT(slot_routeTimer()));
        }
        routage->deleteLater();
        routage=NULL;
    }
}
void myCentralWidget::slot_routeTimer()
{
    QApplication::processEvents();
    treatRoute(routeSimplify);
}

void myCentralWidget::assignPois()
{
    //qWarning() << "AssignPOI "  << route_list.count() << " routes, " << poi_list.count() << " pois";
    QList<bool> frozens;
    QListIterator<ROUTE*> r (route_list);
    while(r.hasNext())
    {
        ROUTE * route=r.next();
        frozens.append(route->getFrozen());
        route->setFrozen(true);
        route->setTemp(true);
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
        route->setTemp(false);
        if(frozens.at(n))
        {
            route->setFrozen(false);
        }
        route->setFrozen(frozens.at(n));
        ++n;
    }
    //qWarning()<<"finished assigning POIs to Routes";
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
    bool hasRoute=!route_list.isEmpty();
    menuBar->mnRoute_edit->setEnabled(hasRoute);
    menuBar->mnRoute_delete->setEnabled(hasRoute);
    menuBar->mnRoute_export->setEnabled(hasRoute);
    menuBar->mnCompassCenterRoute->setEnabled(hasRoute);

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

    bool hasRoutage=!routage_list.isEmpty();
    menuBar->mnRoutage_edit->setEnabled(hasRoutage);
    menuBar->mnRoutage_delete->setEnabled(hasRoutage);

    QListIterator<ROUTAGE*> i (routage_list);
    while(i.hasNext())
        menuBar->addMenuRoutage(i.next());
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
    if(currentPlayer && !currentPlayer->getWrong() && boat_list)
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
        qWarning() << "Deactivate current player";
        if(boat_list && currentPlayer->getType()==BOAT_VLM)
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
            if(player->getWrong()) return;
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
                //qWarning()<<"activating"<<boat->getName();
                ++nn;
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
            //qWarning()<<"emitting accountListUpdated";
            emit accountListUpdated();
            //qWarning()<<"dealing with vlmBoard";
            mainW->getBoard()->playerChanged(player);
            //qWarning()<<"reselected="<<reselected;
            if(reselected)
            {
                mainW->slotSelectBoat(boat_list->at(thisOne));
                boat_list->at(thisOne)->setSelected(true);
                mainW->getBoard()->boatUpdated(boat_list->at(thisOne));
            }
            //qWarning()<<"emitting shRouBis";
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
    emit writeBoatData(player_list,race_list,appFolder.value("userFiles")+"boatAcc.dat");
}

void myCentralWidget::slot_readBoatData(void)
{
    // on vide la liste

    emit readBoatData(appFolder.value("userFiles")+"boatAcc.dat",true);
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
    emit readBoatData(appFolder.value("userFiles")+"boatAcc.dat",false);
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
    else if (act == mb->acView_CurrentColors)
        mode = Terrain::drawCurrent;
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
    emit writePOIData(route_list,poi_list,appFolder.value("userFiles")+"poi.dat");
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
            poi->deleteLater();
        }
        deleteRoute(route_list.first());
    }
    while (!poi_list.isEmpty())
    {
        POI * poi=poi_list.first();
        slot_delPOI_list(poi);
        poi->deleteLater();
    }
    loadPOI();
    QMessageBox::information(this,tr("Chargement des POIs et des routes"),tr("Chargement reussi"));
}

void myCentralWidget::slot_POIimport(void)
{
    emit importZyGrib();
}
void myCentralWidget::slot_POIimportGeoData(void)
{
    xmlPOI->importGeoData();
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
        //qWarning()<<"drawing NSZ";
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
void myCentralWidget::slotFax_open()
{
    bool newFax=false;
    if(!fax)
    {
        newFax=true;
        fax=new faxMeteo(proj,this);
    }
    dialogFaxMeteo * dFax=new dialogFaxMeteo(fax,this);
    if(dFax->exec()!=QDialog::Accepted && newFax)
    {
        delete fax;
        fax=NULL;
    }
    else if(fax->getFileName().isEmpty())
    {
        delete fax;
        fax=NULL;
    }
    delete dFax;
}
void myCentralWidget::slotFax_close()
{
    if (fax)
        delete fax;
    fax=NULL;
}
void myCentralWidget::slotImg_open()
{
    bool newKap=false;
    if(!kap)
    {
        newKap=true;
        kap=new loadImg(proj,this);
    }
    dialogLoadImg * dKap=new dialogLoadImg(kap,this);
    if(dKap->exec()!=QDialog::Accepted && newKap)
    {
        delete kap;
        kap=NULL;
    }
    else if(kap->getMyImgFileName().isEmpty())
    {
        delete kap;
        kap=NULL;
    }
    delete dKap;
}
void myCentralWidget::slotImg_close()
{
    if (kap)
        delete kap;
    kap=NULL;
}
