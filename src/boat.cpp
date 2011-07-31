/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#include "Util.h"

#include "MainWindow.h"
#include "orthoSegment.h"
#include "vlmLine.h"
#include "Polar.h"
#include "settings.h"
#include "Player.h"
#include "GshhsReader.h"
#include "boat.h"

boat::boat(QString pseudo, bool activated,
           Projection * proj,MainWindow * main,myCentralWidget * parent): QGraphicsWidget()
{
    this->mainWindow=main;
    this->parent=parent;

    polar_list = main->getPolarList();
    connect(this,SIGNAL(getPolar(QString)),polar_list,SLOT(getPolar(QString)));

    this->pseudo=pseudo;
    selected = false;
    polarName="";
    polarData=NULL;
    changeLocked=false;    
    forceEstime=false;
    width=height=0;

    speed=heading=0;
    avg=dnm=loch=ortho=loxo=vmg=0;
    windDir=windSpeed=TWA=0;

    WPLat=WPLon=lat=lon=0;

    setZValue(Z_VALUE_BOAT);
    setFont(QFont(QApplication::font()));
    setData(0,BOAT_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);

    estimeLine = new vlmLine(proj,parent->getScene(),Z_VALUE_ESTIME);
    estimeLine->setRoundedEnd(true);
    estimeTimer=new QTimer(this);
    estimeTimer->setInterval(1000);
    connect(estimeTimer,SIGNAL(timeout()),this,SLOT(slot_estimeFlashing()));
    WPLine = new orthoSegment(proj,parent->getScene(),Z_VALUE_ESTIME);

    connect(parent, SIGNAL(showALL(bool)),this,SLOT(slot_shSall()));
    connect(parent, SIGNAL(hideALL(bool)),this,SLOT(slot_shHall()));

    trace.clear();
    trace_drawing = new vlmLine(proj,parent->getScene(),Z_VALUE_BOAT);
    connect(parent,SIGNAL(startReplay(bool)),trace_drawing,SLOT(slot_startReplay(bool)));
    connect(parent,SIGNAL(replay(int)),trace_drawing,SLOT(slot_replay(int)));

    this->proj = proj;
    this->labelHidden=parent->get_shLab_st();

    //qWarning() << "Boat (" << pseudo << ") created labelHidden=" << labelHidden;

    createPopUpMenu();

    //connect(this,SIGNAL(boatUpdated(boat*,bool,bool)),main,SIGNAL(updateRoute(boat *)));
    connect(parent, SIGNAL(shLab(bool)),this,SLOT(slot_shLab(bool)));


    this->activated=false;
    slot_paramChanged();
    this->country="";
    this->drawFlag=false;
    this->flag=QImage();
    updateBoatData();
    this->activated=activated;
    hide();
    WPLine->hide();
    estimeTimer->stop();
    estimeLine->slot_showMe();
    estimeLine->setHidden(false);
    WPLine->hide();
    this->stopAndGo="0";

    /*if(activated)
        show();
*/

    connect(this,SIGNAL(boatSelected(boat*)),main,SLOT(slotSelectBoat(boat*)));
    connect(this,SIGNAL(boatUpdated(boat*,bool,bool)),main,SLOT(slotBoatUpdated(boat*,bool,bool)));
    connect(this,SIGNAL(boatLockStatusChanged(boat*,bool)),
            main,SLOT(slotBoatLockStatusChanged(boat*,bool)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));


    connect(this,SIGNAL(releasePolar(QString)),main,SLOT(releasePolar(QString)));
}

boat::~boat()
{
    disconnect();
    if(!parent->getAboutToQuit())
    {
        if(polarData)
            emit releasePolar(polarData->getName());
        if(estimeLine)
            delete estimeLine;
        if(WPLine)
            delete WPLine;
        if(this->popup)
            delete popup;
    }
}

void boat::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_select = new QAction("Selectionner",popup);
    popup->addAction(ac_select);
    connect(ac_select,SIGNAL(triggered()),this,SLOT(slot_selectBoat()));

    ac_estime = new QAction("Afficher estime",popup);
    popup->addAction(ac_estime);
    connect(ac_estime,SIGNAL(triggered()),this,SLOT(slot_toggleEstime()));

    ac_compassLine = new QAction(tr("Tirer un cap"),popup);
    popup->addAction(ac_compassLine);
    connect(ac_compassLine,SIGNAL(triggered()),this,SLOT(slotCompassLine()));
    connect (this,SIGNAL(compassLine(double,double)),mainWindow,SLOT(slotCompassLineForced(double,double)));

    ac_twaLine = new QAction(tr("Tracer une estime TWA"),popup);
    popup->addAction(ac_twaLine);
    connect(ac_twaLine,SIGNAL(triggered()),this,SLOT(slotTwaLine()));
}

void boat::slot_paramChanged()
{
    myColor = QColor(Settings::getSetting("qtBoat_color",QColor(Qt::blue).name()).toString());
    selColor = QColor(Settings::getSetting("qtBoat_sel_color",QColor(Qt::red).name()).toString());
    estime_type=Settings::getSetting("estimeType",0).toInt();
    switch(estime_type)
    {
        case 0: /* time */
            estime_param = Settings::getSetting("estimeTime",60).toInt();
            break;
        case 1: /* nb vac */
            estime_param = Settings::getSetting("estimeVac",10).toInt();
            break;
        default: /* dist */
            estime_param = Settings::getSetting("estimeLen",100).toInt();
            break;
    }
    this->updateBoatString();
    if(activated)
    {
        drawEstime();
    }
    update();
}

/**************************/
/* Select / unselect      */
/**************************/

void boat::slot_selectBoat()
{
    if(this->getplayerName()!=parent->getPlayer()->getName())
    {
        selected=false;
        return;
    }
    selected = true;
    trace_drawing->show();
    drawEstime();
    if(this->boat_type==BOAT_REAL) return;
    updateTraceColor();
    emit boatSelected(this);
}

void boat::unSelectBoat(bool needUpdate)
{
    selected = false;
    if(needUpdate)
    {
        drawEstime();
        update();
        updateTraceColor();
    }
    if(this->boat_type==BOAT_REAL)
        this->stopRead();
}

/**************************/
/* data access            */
/**************************/

float boat::getBvmgUp(float ws)
{
    if(polarData) return(polarData->getBvmgUp(ws));
    return -1;
}
float boat::getBvmgDown(float ws)
{
    if(polarData) return(polarData->getBvmgDown(ws));
    return -1;
}

/**************************/
/* Paint & graphics       */
/**************************/

void boat::slot_toggleEstime()
{
    forceEstime=!forceEstime;
    drawEstime();
    update();
}

void boat::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    /*if(!this->isVisible())
        return;*/
    int dy = height/2;
    QPen pen(selected?Qt::darkRed:Qt::darkBlue);
    pnt->setFont(QApplication::font());
    if(!labelHidden)
    {
        if(Settings::getSetting("showFlag",0).toInt()==1 && this->getType()==BOAT_VLM)
        {
            if(flag.isNull())
            {
                if(flag.load("img/flags/"+this->country+".png"))
                {
                    flag=flag.scaled(30,20,Qt::KeepAspectRatio);
                    drawFlag=true;
                    prepareGeometryChange();
                }
                else
                    drawFlag=false;
            }
            else
            {
                if(!drawFlag)
                    prepareGeometryChange();
                drawFlag=true;
            }
        }
        else
        {
            if(drawFlag)
                prepareGeometryChange();
            drawFlag=false;
        }
        if(this->getType()==BOAT_VLM)
        {
            if(this->stopAndGo!="0")
                bgcolor=QColor(239,48,36,150);
            else
                bgcolor = QColor(255,255,255,150);
        }
        pnt->setBrush(QBrush(bgcolor));
        if(!drawFlag)
            pnt->drawRoundRect(9,0, width,height, 50,50);
        else
            pnt->drawRoundRect(21,0, width,height, 50,50);
        pnt->setBrush(Qt::NoBrush);
        pnt->setPen(pen);
        QFontMetrics fm(QApplication::font());
        int w = fm.width("_")+1;
        if(!drawFlag)
            pnt->drawText(w+9,height-height/4,my_str);
        else
            pnt->drawText(w+21,height-height/4,my_str);
    }
    pen.setColor(selected?selColor:myColor);
    pen.setWidth(4);
    pnt->setPen(pen);
    if(!drawFlag)
        pnt->fillRect(0,dy-3,7,7, QBrush(selected?selColor:myColor));
    else
        pnt->drawImage(-11,dy-9,flag);
    int g = 60;
    pen = QPen(QColor(g,g,g));
    pen.setWidth(1);
    pnt->setPen(pen);
    if(!labelHidden)
    {
        if(!drawFlag)
            pnt->drawRoundRect(9,0, width,height, 50,50);
        else
            pnt->drawRoundRect(21,0, width,height, 50,50);
    }

    //drawEstime();
}

void boat::updateTraceColor(void)
{
    if(selected)
    {
        QPen penLine(selColor,1);
        penLine.setWidthF(1);
        trace_drawing->setLinePen(penLine);
        trace_drawing->setPointMode(selColor);
    }
    else
    {
        QPen penLine(myColor,1);
        penLine.setWidthF(1);
        trace_drawing->setLinePen(penLine);
        trace_drawing->setPointMode(myColor);
    }
    trace_drawing->setNbVacPerHour(3600/getVacLen());
    trace_drawing->slot_showMe();
}

void boat::drawEstime(void)
{
    drawEstime(getHeading(),getSpeed());
}

void boat::drawEstime(float myHeading, float mySpeed)
{

    estimeLine->deleteAll();
    WPLine->hideSegment();
    /*should we draw something?*/
    if(isUpdating() || !getStatus())
        return;
    estimeLine->setHidden(false);
    QPen penLine1(QColor(Settings::getSetting("estimeLineColor", QColor(Qt::darkMagenta)).value<QColor>()),1,Qt::SolidLine);
    penLine1.setWidthF(Settings::getSetting("estimeLineWidth", 1.6).toDouble());
    QPen penLine2(QColor(Qt::black),1,Qt::DotLine);
    penLine2.setWidthF(1.2);

    int estime_param_2;
    float estime;

    /* draw estime */
    if(getIsSelected() || getForceEstime())
    {
        double tmp_lat,tmp_lon;


        switch(estime_type)
        {
            case 0: /* time */
                estime = (float)((float)(estime_param/float(60.0000000000)))*mySpeed;
                break;
            case 1: /* nb vac */
                estime_param_2=getVacLen();
                estime = (float)((((float)(estime_param*estime_param_2))/3660.000000000)*mySpeed);
                break;
            default: /* dist */
                estime = estime_param;
                break;
        }

        Util::getCoordFromDistanceAngle(lat,lon,estime,myHeading,&tmp_lat,&tmp_lon);
        GshhsReader *map=parent->get_gshhsReader();
        double I1,J1,I2,J2;
        proj->map2screenFloat(lon,lat,&I1,&J1);
        if(map->getQuality()>=2)
        {
            proj->map2screenFloat(tmp_lon,tmp_lat,&I2,&J2);
            //qWarning("crossing (%.5f,%.5f,%.5f,%.5f) (%.5f,%.5f,%.5f,%.5f)",I1,J1,I2,J2,lon,lat,tmp_lon,tmp_lat);
            //qWarning("estime=%.5f, myHeading=%.5f",estime,myHeading);
            if(estime>0.0001 && map->crossing(QLineF(I1,J1,I2,J2),QLineF(lon,lat,tmp_lon,tmp_lat)))
            {
                estimeTimer->start();
                penLine1.setColor(Qt::red);
            }
            else
                estimeTimer->stop();
        }
        else
            estimeTimer->stop();
        estimeLine->setLinePen(penLine1);
        estimeLine->addPoint(lat,lon);
        estimeLine->addPoint(tmp_lat,tmp_lon);
        estimeLine->slot_showMe();
        //estimeLine->initSegment(i1,j1,i2,j2);
        /* draw ortho to wp */
        if(WPLat != 0 && WPLon != 0)
        {
            WPLine->setLinePen(penLine2);
            proj->map2screenFloat(WPLon,WPLat,&I2,&J2);
            WPLine->initSegment(I1,J1,I2,J2);
        }
    }
}

/**************************/
/* shape & boundingRect   */
/**************************/
void boat::slot_estimeFlashing()
{
    if(!selected)
    {
        estimeTimer->stop();
        estimeLine->setHidden(true);
        WPLine->hide();
        return;
    }
    estimeLine->setHidden(!estimeLine->getHidden());
}

void boat::slotCompassLine()
{
    double i1,j1;
    proj->map2screenFloat(this->lon,this->lat,&i1,&j1);
    emit compassLine(i1,j1);
}
QPainterPath boat::shape() const
{
    QPainterPath path;
    path.addRect(0,0,width,height);
    return path;
}

QRectF boat::boundingRect() const
{
    if(!drawFlag)
        return QRectF(0,0,width+10,height+2);
    else
        return QRectF(-12,height/2-10,width+30,25);
}

/**************************/
/* Data update            */
/**************************/

void boat::updateBoatData()
{
    updateBoatString();
    reloadPolar();
    updatePosition();
    updateHint();
}

void boat::updatePosition(void)
{
    int boat_i,boat_j;
    Util::computePos(proj,lat,lon,&boat_i,&boat_j);
    boat_i-=3;
    boat_j-=(height/2);
    //qWarning() << "upd position: " << x() << "," << y() << " -> " << boat_i << "," << boat_j;
    setPos(boat_i, boat_j);
    drawEstime();
}

void boat::slot_projectionUpdated()
{
    if(activated)
        updatePosition();
}

void boat::setStatus(bool activated)
{
     //qWarning() << "BOAT: " << this->getBoatPseudo() << " setStatus=" << activated;
     this->activated=activated;
     setVisible(activated);
     slot_paramChanged();
     updateBoatData();


     if(!activated)
     {
        WPLine->hide();
        estimeTimer->stop();
        estimeLine->setHidden(true);;
        if(this->boat_type==BOAT_REAL)
            this->stopRead();
     }
     else
         estimeLine->setHidden(false);
}

void boat::playerDeActivated(void)
{
    //qWarning() << "Deactivation of: " << this->getBoatPseudo();
    if(selected)
        this->unSelectBoat(false);
    this->hide();
    setVisible(false);
    trace_drawing->hide();
    WPLine->hide();
    estimeTimer->stop();
    estimeLine->setHidden(true);;
    if(this->boat_type==BOAT_REAL)
        this->stopRead();
}

void boat::setParam(QString pseudo)
{
     this->pseudo=pseudo;
}

void boat::setParam(QString pseudo, bool activated)
{
    setStatus(activated);
    setParam(pseudo);
}

void boat::reloadPolar(bool forced)
{
    //qWarning()<<"inside reloadPolar with forced="<<forced;
    if(forced && polarName.isEmpty())
        polarName=polarData->getName();
    //qWarning()<<"polarName="<<polarName;
    if(polarName.isEmpty()) /* nom de la polaire est vide => pas de chargement */
    {
       //qWarning() << "Polar name empty => nothing to load";
        if(polarData!=NULL) /* doit on faire un release tt de meme? */
        {
            emit releasePolar(polarData->getName());
            polarData=NULL;
        }
        return;
    }

    if(!forced && polarData != NULL && polarName == polarData->getName()) /* reload inutile */
    {
        //qWarning() << "Polar already loaded => nothing to do"<<forced;
        return;
    }

    if(polarData!=NULL)
    {
        //qWarning() << "Releasing polar " << polarData->getName();
        emit releasePolar(polarData->getName()); /* release si une polaire déjà chargée */
    }
    connect(polar_list,SIGNAL(polarLoaded(QString,Polar *)),this,SLOT(polarLoaded(QString,Polar *)));
    //qWarning() << pseudo << " request polar " << polarName;
    emit getPolar(polarName);
}

void boat::polarLoaded(QString polarName,Polar * polarData) {
    disconnect(polar_list,SIGNAL(polarLoaded(QString,Polar *)),this,SLOT(polarLoaded(QString,Polar *)));
    //qWarning() << pseudo << " get polar returned: " << polarName;
    if(this->polarName==polarName)
    {
        this->polarData=polarData;
        /*if(polarData)
            qWarning() << polarData->getName() << " received";
        else
            qWarning() << "void polar received";*/
    }
}

void boat::setLockStatus(bool status)
{
//n'a probablement d interet que pour les boat VLM
    if(status!=changeLocked)
    {
        changeLocked=status;
        emit boatLockStatusChanged(this,status);
    }
}

void boat::slot_updateGraphicsParameters()
{
    if(activated)
    {
        drawEstime();
        showNextGates();
    }
}

/**************************/
/* Events                 */
/**************************/

//void boat::mousePressEvent(QGraphicsSceneMouseEvent * e)
//{
//    if (e->button() != Qt::LeftButton)
//    {
//         e->ignore();
//    }
//}
//
//void  boat::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
//{
//    if(e->button() == Qt::LeftButton)
//    {
//        emit clearSelection();
//        if(!mainWindow->get_selPOI_instruction())
//        {
//            slot_selectBoat();
//            return;
//        }
//    }
//}

void boat::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{
    bool onlyLineOff = false;

    switch(parent->getCompassMode((float)e->scenePos().x(),(float)e->scenePos().y()))
    {
        case 0:
            /* not showing menu line, default text*/
            ac_compassLine->setText(tr("Tirer un cap"));
            ac_compassLine->setEnabled(true);
            break;
        case 1:
            /* showing text for compass line off*/
            ac_compassLine->setText(tr("Arret du cap"));
            ac_compassLine->setEnabled(true);
            onlyLineOff=true;
            break;
        case 2:
        case 3:
            ac_compassLine->setText(tr("Tirer un cap"));
            ac_compassLine->setEnabled(true);
            break;
        }

    if(forceEstime)
        ac_estime->setText(tr("Cacher estime"));
    else
        ac_estime->setText(tr("Afficher estime"));

    if(onlyLineOff)
    {
        ac_select->setEnabled(false);
        ac_estime->setEnabled(false);
    }
    else
    {
        ac_select->setEnabled(!mainWindow->get_selPOI_instruction());
        ac_estime->setEnabled(!selected);
    }

    popup->exec(QCursor::pos());
}
