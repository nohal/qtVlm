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


#include <cassert>

#include <QTimer>
#include <QMessageBox>
#include <QDebug>

#include "POI.h"
#include "Util.h"
#include "MainWindow.h"
#include "settings.h"
#include "Orthodromie.h"
#include "mycentralwidget.h"
#include "route.h"
#include "boatVLM.h"
#include "Projection.h"
#include "DialogFinePosit.h"
#include "dialogpoiconnect.h"

/**************************/
/* Init & Clean           */
/**************************/

POI::POI(QString name, int type, double lat, double lon,
                 Projection *proj, QWidget *main, myCentralWidget *parentWindow,
                 float wph,int tstamp,bool useTstamp,boat *myBoat)
    : QGraphicsWidget()
{
    this->parent = parentWindow;
    this->owner = main;
    this->proj = proj;
    this->name = name;
    setLatitude(lat);
    setLongitude(lon);
    this->wph=wph;
    this->timeStamp=tstamp;
    this->useTstamp=useTstamp;
    this->type=type;
    this->typeMask=1<<type;
    this->myBoat=myBoat;
    this->searchRangeLon=1;
    this->searchRangeLat=1;
    this->searchStep=0.1;
    this->abortSearch=false;
    this->navMode=0;
    this->optimizing=true;
    this->partOfTwa=false;
    this->notSimplificable=false;
    this->connectedPoi=NULL;
    this->lineBetweenPois=NULL;
    this->lineColor=Qt::blue;
    this->lineWidth=2;

    useRouteTstamp=false;
    routeTimeStamp=-1;
    route=NULL;
    routeName="";
    this->labelHidden=parentWindow->get_shLab_st();
    this->myLabelHidden=false;
//    qWarning() << "Init POI label: " << loxoCap<<" name: "<<name;

    WPlon=WPlat=-1;
    isWp=false;
    isMoving=false;
    VLMBoardIsBusy=false;

    setZValue(Z_VALUE_POI);
    setFont(QFont(QApplication::font()));
    setCursor(Qt::PointingHandCursor);
    setData(0,POI_WTYPE);

    fgcolor = QColor(0,0,0);
    int gr = 255;
    bgcolor = QColor(gr,gr,gr,150);
    width=20;
    height=10;

    createPopUpMenu();
    slot_paramChanged();

    connect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));

    connect(this,SIGNAL(chgWP(float,float,float)),main,SLOT(slotChgWP(float,float,float)));

    connect(this,SIGNAL(selectPOI(POI*)),main,SLOT(slot_POIselected(POI*)));

    connect(this,SIGNAL(setGribDate(time_t)),main,SLOT(slotSetGribDate(time_t)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));
    connect(main,SIGNAL(WPChanged(double,double)),this,SLOT(slot_WPChanged(double,double)));
    connect(main,SIGNAL(boatHasUpdated(boat*)),this,SLOT(slot_updateTip(boat*)));
    connect(parent,SIGNAL(stopCompassLine()),this,SLOT(slot_abort()));

    ((MainWindow*)main)->getBoatWP(&WPlat,&WPlon);
    setName(name);
    slot_updateProjection();
    chkIsWP();
    if(!parentWindow->get_shPoi_st())
        show();
    else
        hide();
}

POI::~POI()
{
    if(route!=NULL) this->setRoute(NULL);
    if(lineBetweenPois!=NULL && !parent->getAboutToQuit())
    {
        delete lineBetweenPois;
        this->connectedPoi->setConnectedPoi(NULL);
        this->connectedPoi->setLineBetweenPois(NULL);
    }
    if(popup && !parent->getAboutToQuit())
        delete popup;
}
void POI::setLongitude(double lon)
{
    this->lon=(double)(qRound(lon*1000.0))/1000.0;

    if(this->lon>180) this->lon=this->lon-360;
    if(this->lon<-180) this->lon=this->lon+360;
}
void POI::setLatitude(double lat)
{
    this->lat=(double)(qRound(lat*1000.0))/1000.0;
}
void POI::rmSignal(void)
{
    disconnect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    disconnect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));
    disconnect(this,SIGNAL(chgWP(float,float,float)),owner,SLOT(slotChgWP(float,float,float)));

    disconnect(this,SIGNAL(setGribDate(time_t)),owner,SLOT(slotSetGribDate(time_t)));

    disconnect(owner,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));
    disconnect(owner,SIGNAL(WPChanged(double,double)),this,SLOT(slot_WPChanged(double,double)));
    disconnect(owner,SIGNAL(boatHasUpdated(boat*)),this,SLOT(slot_updateTip(boat*)));

}

void POI::createPopUpMenu(void)
{
    popup = new QMenu(parent);

    ac_edit = new QAction(tr("Editer"),popup);
    popup->addAction(ac_edit);
    connect(ac_edit,SIGNAL(triggered()),this,SLOT(slot_editPOI()));

    ac_delPoi = new QAction(tr("Supprimer"),popup);
    popup->addAction(ac_delPoi);
    connect(ac_delPoi,SIGNAL(triggered()),this,SLOT(slot_delPoi()));

    ac_copy = new QAction(tr("Copier"),popup);
    popup->addAction(ac_copy);
    connect(ac_copy,SIGNAL(triggered()),this,SLOT(slot_copy()));

    ac_setWp = new QAction(tr("Marque->WP"),popup);
    popup->addAction(ac_setWp);
    connect(ac_setWp,SIGNAL(triggered()),this,SLOT(slot_setWP()));

    ac_setGribDate = new QAction(tr("Set Date"),popup);
    popup->addAction(ac_setGribDate);
    connect(ac_setGribDate,SIGNAL(triggered()),this,SLOT(slot_setGribDate()));

    ac_setHorn= new QAction(tr("Activer la corne de brume 10mn avant le passage"),popup);
    popup->addAction(ac_setHorn);
    connect(ac_setHorn,SIGNAL(triggered()),this,SLOT(slot_setHorn()));

    ac_compassLine = new QAction(tr("Tirer un cap"),popup);
    popup->addAction(ac_compassLine);
    connect(ac_compassLine,SIGNAL(triggered()),this,SLOT(slotCompassLine()));
    connect(this,SIGNAL(compassLine(int,int)),owner,SLOT(slotCompassLineForced(int,int)));

    ac_twaLine = new QAction(tr("Tracer une estime TWA"),popup);
    popup->addAction(ac_twaLine);
    connect(ac_twaLine,SIGNAL(triggered()),this,SLOT(slot_twaLine()));

    popup->addSeparator();
    ac_routeList = new QMenu(tr("Routes"));
    connect(ac_routeList,SIGNAL(triggered(QAction*)),this,SLOT(slot_routeMenu(QAction*)));
    popup->addMenu(ac_routeList);


    ac_editRoute = new QAction(tr("Editer la route "),popup);
    ac_editRoute->setData(QVariant(QMetaType::VoidStar, &route));
    popup->addAction(ac_editRoute);
    ac_editRoute->setEnabled(false);;
    connect(ac_editRoute,SIGNAL(triggered()),this,SLOT(slot_editRoute()));

    ac_delRoute = new QAction(tr("Supprimer la route "),popup);
    ac_delRoute->setData(QVariant(QMetaType::VoidStar, &route));
    popup->addAction(ac_delRoute);
    ac_delRoute->setEnabled(false);
    connect(ac_delRoute,SIGNAL(triggered()),parent,SLOT(slot_deleteRoute()));

    ac_finePosit = new QAction(tr("Optimiser le placement sur la route"),popup);
    connect(ac_finePosit,SIGNAL(triggered()),this,SLOT(slot_finePosit()));
    popup->addAction(ac_finePosit);

    ac_modeList = new QMenu(tr("Mode de navigation vers ce POI"));
    QActionGroup * ptr_group = new QActionGroup(ac_modeList);

    ac_modeList1 = new QAction(tr("VB-VMG"),popup);
    ac_modeList1->setCheckable  (true);
    ac_modeList->addAction(ac_modeList1);
    ptr_group->addAction(ac_modeList1);
    ac_modeList1->setData(0);

    ac_modeList2 = new QAction(tr("B-VMG"),popup);
    ac_modeList2->setCheckable  (true);
    ac_modeList->addAction(ac_modeList2);
    ptr_group->addAction(ac_modeList2);
    ac_modeList2->setData(1);

    ac_modeList3 = new QAction(tr("ORTHO"),popup);
    ac_modeList3->setCheckable  (true);
    ac_modeList->addAction(ac_modeList3);
    ptr_group->addAction(ac_modeList3);
    ac_modeList3->setData(2);
    ac_modeList1->setChecked(true);
    connect(ac_modeList,SIGNAL(triggered(QAction*)),this,SLOT(slot_setMode(QAction*)));
    popup->addMenu(ac_modeList);

    popup->addSeparator();
    ac_connect=new QAction(tr("Tracer/Editer une ligne avec un autre POI"),popup);
    connect(ac_connect,SIGNAL(triggered()),this,SLOT(slot_relier()));
    popup->addAction(ac_connect);
}

/**************************/
/* Events                 */
/**************************/

void POI::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && e->modifiers()==Qt::ShiftModifier)
    {
         if(route!=NULL)
             if(route->getFrozen()||route->isBusy()) return;
         if(VLMBoardIsBusy && this->isWp) return;
         previousLon=lon;
         previousLat=lat;
         this->partOfTwa=false;
         isMoving=true;
//         if(route!=NULL)
//            this->isPartOfBvmg=route->isPartOfBvmg(this);
//         else
//            this->isPartOfBvmg=false;
         if(route!=NULL)
             route->setFastVmgCalc(true);
         mouse_x=e->scenePos().x();
         mouse_y=e->scenePos().y();
         setCursor(Qt::BlankCursor);
         update();
     }
    else if(!((MainWindow*)owner)->get_selPOI_instruction())
        e->ignore();
}

bool POI::tryMoving(int x, int y)
{
    if(isMoving)
    {
        int new_x=this->x()+(x-mouse_x);
        int new_y=this->y()+(y-mouse_y);

        setPos(new_x,new_y);
        mouse_x=x;
        mouse_y=y;


        if(route!=NULL && route->isLive() &&!route->isBusy())
        {
            double newlon,newlat;
            new_x=scenePos().x();
            new_y=scenePos().y()+height/2;
            proj->screen2map(new_x,new_y, &newlon, &newlat);
            setLongitude(newlon);
            setLatitude(newlat);
            Util::computePos(proj,lat, lon, &pi, &pj);
            emit poiMoving();
        }
        if(lineBetweenPois!=NULL)
        {
            double newlon,newlat;
            new_x=scenePos().x();
            new_y=scenePos().y()+height/2;
            proj->screen2map(new_x,new_y, &newlon, &newlat);
            setLongitude(newlon);
            setLatitude(newlat);
            Util::computePos(proj,lat, lon, &pi, &pj);
            lineBetweenPois->deleteAll();
            lineBetweenPois->addVlmPoint(vlmPoint(this->lon,this->lat));
            lineBetweenPois->addVlmPoint(vlmPoint(this->connectedPoi->lon,this->connectedPoi->lat));
            lineBetweenPois->slot_showMe();
        }
        return true;
    }
    return false;
}

void POI::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(isMoving)
    {
        double newlon,newlat;
        if(e->modifiers()==Qt::ShiftModifier)
        {
            int new_x=scenePos().x();
            int new_y=scenePos().y()+height/2;
            proj->screen2map(new_x,new_y, &newlon, &newlat);
        }
        else
        {
            newlon=previousLon;
            newlat=previousLat;
        }
        setLongitude(newlon);
        setLatitude(newlat);
        Util::computePos(proj,lat, lon, &pi, &pj);
        setPos(pi, pj-height/2);
        isMoving=false;
        setCursor(Qt::PointingHandCursor);
        setName(this->name);
        update();
        if(isWp && e->modifiers()==Qt::ShiftModifier) slot_setWP();
        if(route!=NULL)
        {
            route->setFastVmgCalc(false);
            route->slot_recalculate();
        }
        if(lineBetweenPois!=NULL)
        {
            lineBetweenPois->deleteAll();
            lineBetweenPois->addVlmPoint(vlmPoint(this->lon,this->lat));
            lineBetweenPois->addVlmPoint(vlmPoint(this->connectedPoi->lon,this->connectedPoi->lat));
            lineBetweenPois->slot_showMe();
        }
        return;
    }

    if(e->pos().x() < 0 || e->pos().x()>width || e->pos().y() < 0 || e->pos().y() > height)
    {
        e->ignore();
        return;
    }
    if (e->button() == Qt::LeftButton)
    {
        emit clearSelection();
        if(((MainWindow*)owner)->get_selPOI_instruction())
            emit selectPOI(this);
        else
        {
//            if(route!=NULL)
//                if(route->getFrozen()) return;
//            emit editPOI(this);
            e->ignore();
        }
    }
}

void POI::contextMenuEvent(QGraphicsSceneContextMenuEvent * e)
{

    bool onlyLineOff = false;
    if(route==NULL || route->getLastPoi()==this || route->getFrozen())
    {
        this->ac_finePosit->setEnabled(false);
    }
    else
    {
        this->ac_finePosit->setEnabled(true);
    }
    if(route==NULL)
        this->ac_setHorn->setEnabled(false);
    else
        this->ac_setHorn->setEnabled(true);


    switch(parent->getCompassMode(e->scenePos().x(),e->scenePos().y()))
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

    if(onlyLineOff || ((MainWindow*)owner)->get_selPOI_instruction()) /* ie only show the Arret du cap line */
    {
        ac_setWp->setEnabled(false);
        ac_setGribDate->setEnabled(false);
        ac_edit->setEnabled(false);
        ac_delPoi->setEnabled(false);
        ac_copy->setEnabled(false);
        ac_routeList->setEnabled(false);
        ac_delRoute->setEnabled(false);
        ac_delRoute->setData(QVariant(QMetaType::VoidStar, &route));
        ac_editRoute->setEnabled(false);
    }
    else
    {
        ac_setWp->setEnabled(!((MainWindow*)owner)->getBoatLockStatus());
        ac_setGribDate->setEnabled(useTstamp || useRouteTstamp);
        ac_edit->setEnabled(true);
        ac_delPoi->setEnabled(true);
        ac_copy->setEnabled(true);
        ac_routeList->setEnabled(true);
        ac_delRoute->setEnabled(true);
        ac_editRoute->setEnabled(true);
        ac_delRoute->setData(QVariant(QMetaType::VoidStar, &route));
        if(route==NULL)
        {
            ac_delRoute->setEnabled(false);
            ac_editRoute->setEnabled(false);
        }
        else
        {
            ac_delRoute->setText(tr("Supprimer la route ")+route->getName());
            ac_delRoute->setEnabled(true);
            ac_editRoute->setText(tr("Editer la route ")+route->getName());
            ac_editRoute->setEnabled(true);
        }
        /*clear current actions */
        ac_routeList->clear();

        QActionGroup * ptr_group = new QActionGroup(ac_routeList);
        QListIterator<ROUTE*> j (parent->getRouteList());
        QAction * ptr;
        ptr=new QAction(tr("Aucune route"),ac_routeList);
        ptr->setCheckable  (true);
        ac_routeList->addAction(ptr);
        ptr_group->addAction(ptr);
        ptr->setData(0);

        int k=0;

        if(route==NULL)
            ptr->setChecked(true);
        else
            ptr->setChecked(false);
        while(j.hasNext())
        {
            ROUTE * ptr_route = j.next();
            ptr=new QAction(ptr_route->getName(),ac_routeList);            
            ptr->setCheckable  (true);
            ptr->setData(-1);
            ac_routeList->addAction(ptr);
            ptr_group->addAction(ptr);
            if(ptr_route == route)
                ptr->setChecked(true);
            else
                ptr->setChecked(false);
            ptr->setData(qVariantFromValue((void *) ptr_route));
            k++;
        }

    }

    /* Modification du nom du bateau */
    boat * ptr=parent->getSelectedBoat();
    if(ptr)
    {
        ac_setWp->setText(tr("Marque->WP : ")+ptr->getBoatPseudo());
    }
    else
        ac_setWp->setEnabled(false);

    popup->exec(QCursor::pos());
}

/*********************/
/* data manipulation */
/*********************/

void POI::setNavMode(int mode)
{
    this->navMode=mode;
    switch(navMode)
    {
        case 0:
            ac_modeList1->setChecked(true);
            break;
        case 1:
            ac_modeList2->setChecked(true);
            break;
        case 2:
            ac_modeList3->setChecked(true);
            break;
        default:
            this->navMode=0;
            ac_modeList1->setChecked(true);
            break;
     }
}
void POI::setName(QString name)
{
    this->name=name;
    update_myStr();
    setTip("");
}
void POI::setTip(QString tip)
{
    boat * w_boat;
    tip=tip.replace(" ","&nbsp;");
    if(route==NULL)
        w_boat=myBoat;
    else
        w_boat=route->getBoat();
    if(w_boat)
    {
        Orthodromie orth2boat(w_boat->getLon(), w_boat->getLat(), lon, lat);
        double   distance=orth2boat.getDistance();
        QString tt=tr("Distance Ortho a partir de ")+w_boat->getBoatPseudo()+": "+
                   Util::formatDistance(distance);
        tt=tt.replace(" ","&nbsp;");
        setToolTip(getTypeStr() + " : " + my_str + "<br>"+tt+tip);
    }
    else
        setToolTip(getTypeStr() + " : "+my_str);
}
void POI::update_myStr(void)
{
    QDateTime tm;
    tm.setTimeSpec(Qt::UTC);
    tm.setTime_t(getTimeStamp());
    if(route==NULL) useRouteTstamp=false;
    if(useRouteTstamp || useTstamp)
        if(useRouteTstamp)
        {
            if(this->routeTimeStamp==0)
                my_str=name + " " + tr("Non joignable avec ce Grib");
            else
                my_str=name + " " + tm.toString("dd MMM-hh:mm");
        }
        else
            my_str=name + " " + tm.toString("dd MMM-hh:mm");
    else
        my_str=this->name;
    QFontMetrics fm(font());
    prepareGeometryChange();
    width = fm.width(my_str) + 10 +2;
    height = qMax(fm.height()+2,10);
}

void POI::setTimeStamp(time_t date)
{
    this->useTstamp=true;
    this->timeStamp=date;
    update_myStr();

}

void POI::setRouteTimeStamp(time_t date)
{
    this->useRouteTstamp=(date!=-1);
    if(useRouteTstamp) useTstamp=false;
    this->routeTimeStamp=date;
    update_myStr();
    //update();
}

QString POI::getTypeStr(int index)
{
    QString type_str[3] = { "POI", "Marque", "Balise" };
    return type_str[index];
}

void POI::chkIsWP(void)
{
    if(compDouble(lat,WPlat) && compDouble(lon,WPlon))
    {
        if(!isWp)
        {
            isWp=true;
            update();
        }
    }
    else
    {
        if(isWp)
        {
            isWp=false;
            update();
        }
    }
}
void POI::setRoute(ROUTE *route)
{
    this->partOfTwa=false;
    if(this->route!=NULL)
    {
        if(this->route->getFrozen()) return;
        //this->notSimplificable=false;
        this->route->removePoi(this);
        this->routeName="";
    }
    this->route=route;
    if(this->route!=NULL)
    {
        this->route->insertPoi(this);
        this->routeName=route->getName();
    }
    else
    {
        //this->notSimplificable=false;
        setRouteTimeStamp(false);
    }
}

/**************************/
/* Slots                  */
/**************************/
void POI::slot_relier()
{
    DialogPoiConnect * box=new DialogPoiConnect(this,parent);
    if(box->exec()==QDialog::Accepted)
    {
        if(connectedPoi==NULL)
        {
            if(lineBetweenPois!=NULL)
            {
                delete lineBetweenPois;
                lineBetweenPois=NULL;
            }
        }
        else
        {
            if(lineBetweenPois!=NULL)
                delete lineBetweenPois;
            lineBetweenPois=new vlmLine(proj,parent->getScene(),Z_VALUE_POI);
            lineBetweenPois->addVlmPoint(vlmPoint(this->lon,this->lat));
            lineBetweenPois->addVlmPoint(vlmPoint(this->connectedPoi->lon,this->connectedPoi->lat));
            connectedPoi->setLineBetweenPois(lineBetweenPois);
            QPen pen(lineColor);
            pen.setWidthF(lineWidth);
            lineBetweenPois->setLinePen(pen);
            lineBetweenPois->slot_showMe();
        }
    }
}

void POI::slot_editRoute()
{
    if (this->route==NULL) return;
    parent->slot_editRoute(route);
}
void POI::slotCompassLine()
{
    int i1,j1;
    proj->map2screen(this->lon,this->lat,&i1,&j1);
    emit compassLine(i1,j1);
}
void POI::slot_setHorn()
{
    QDateTime time=QDateTime::fromTime_t(this->routeTimeStamp).toUTC();
    time.setTimeSpec(Qt::UTC);
//    qWarning()<<"time before sub "<<time;
//    time.addSecs(-600);
    time=(QDateTime::fromTime_t(time.toTime_t()-600)).toUTC();
//    qWarning()<<"time after sub "<<time;
    parent->setHornDate(time);
    parent->setHornIsActivated(true);
    parent->setHorn();
}

void POI::slot_updateProjection()
{
    Util::computePos(proj,lat, lon, &pi, &pj);
    int dy = height/2;
    setPos(pi, pj-dy);
}
void POI::slot_updateTip(boat * myBoat)
{
    this->myBoat=myBoat;
    if(route==NULL) setTip("");
}

void POI::slot_editPOI()
{
    if(route!=NULL)
        if(route->getFrozen()) return;
    emit editPOI(this);
}

void POI::slot_copy()
{
    Util::setWPClipboard(lat,lon,wph);
}

void POI::slot_setWP()
{
    this->partOfTwa=false;
    VLMBoardIsBusy=true;
    emit chgWP(lat,lon,wph);
}

void POI::slot_setGribDate()
{
    if (useRouteTstamp) emit setGribDate(routeTimeStamp);
    else if(useTstamp) emit setGribDate(timeStamp);
}

void POI::slot_delPoi()
{
    if(route!=NULL)
        if(route->getFrozen()) return;
    int rep = QMessageBox::question (parent,
            tr("Detruire la marque"),
            tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
    if (rep == QMessageBox::Yes) {

        if(route!=NULL)
        {
            if(route->isBusy()) return;
            setRoute(NULL);
        }
        emit delPOI_list(this);
        rmSignal();
        close();
    }
}

void POI::slot_paramChanged()
{
    poiColor = QColor(Settings::getSetting("POI_color",QColor(Qt::black).name()).toString());
    mwpColor = QColor(Settings::getSetting("Marque_WP_color",QColor(Qt::red).name()).toString());
    wpColor  = QColor(Settings::getSetting("WP_color",QColor(Qt::darkYellow).name()).toString());
    baliseColor  = QColor(Settings::getSetting("Balise_color",QColor(Qt::darkMagenta).name()).toString());
    update();
}

void POI::slot_WPChanged(double tlat,double tlon)
{
    WPlat=tlat;
    WPlon=tlon;
    chkIsWP();
    VLMBoardIsBusy=false;
    if (this->isWp)
        this->setWph(parent->getSelectedBoat()->getWPHd());
}
QString POI::getRouteName()
{
    return routeName;
}

void POI::slot_setMode(QAction* ptr_action)
{
    this->navMode=ptr_action->data().value<int>();
    this->setName(name);
    update();
    if(route!=NULL)
        route->slot_recalculate();
}
void POI::slot_routeMenu(QAction* ptr_action)
{
    ROUTE *  ptr_route = (ROUTE *) ptr_action->data().value<void *>();
    setRoute(ptr_route);
}
void POI::slot_finePosit()
{
    if (this->route==NULL) return;
    if (this->route->getFrozen()) return;
    if (route->getLastPoi()==this) return;
    if (route->isBusy()) return;
    if (route->getOptimizing()) return;
    if (route->getUseVbvmgVlm())
    {
        QMessageBox::critical(0,tr("Optimisation du placement d'un POI"),
                              tr("Vous ne pouvez pas optimiser en mode VBVMG-VLM"));
        return;
    }
    this->abortSearch=false;
    double savedLon=lon;
    double savedLat=lat;
    DialogFinePosit * dia=new DialogFinePosit(this,parent);
    if(dia->exec()!=QDialog::Accepted)
    {
        delete dia;
        return;
    }
    delete dia;
    double rangeLon=searchRangeLon;
    double rangeLat=searchRangeLat;
    double step=searchStep;
    time_t bestEta=0;
    bool has_bestEta=false;
    double bestRemain=0;
    bool has_bestRemain=false;
    double bestLon=0;
    double bestLat=0;
    double sens=1;
    QString r;
    route->setOptimizing(!this->optimizing);
    route->setOptimizingPOI(true);
    route->setPoiName(this->name);
    POI * best=NULL;
    QDateTime tm;
    route->setFastVmgCalc(true);
    Orthodromie fromBoat(route->getStartLon(),route->getStartLat(),lon,lat);
    float bestDistance=0;
    POI * previousMe=NULL;
    route->slot_recalculate();
    if (route->getHas_eta())
    {
        tm.setTimeSpec(Qt::UTC);
        tm.setTime_t(route->getEta());
        previousMe=new POI(tr("ETA du prochain POI: ")+tm.toString("dd MMM-hh:mm"),0,savedLat,savedLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
    }
    else
        previousMe=new POI(tr("Dist. restante du prochain POI: ")+r.sprintf("%.2f milles",route->getRemain()),
                     0,savedLat,savedLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
    parent->slot_addPOI_list(previousMe);
    setLongitude(lon-rangeLon/2.0);
    setLatitude(lat-rangeLat/2.0);
    QApplication::processEvents();
    while(lon<savedLon+rangeLon/2)
    {
        while((sens==1.0 && lat<savedLat+rangeLat/2.0)||(sens==-1.0 && lat>savedLat-rangeLat/2.0))
        {
            if(this->abortSearch) break;
            route->slot_recalculate();
            if (route->getHas_eta())
            {
                if (!has_bestEta)
                {
                    has_bestEta=true;
                    bestEta=route->getEta();
                    bestLon=lon;
                    bestLat=lat;
                    tm.setTimeSpec(Qt::UTC);
                    tm.setTime_t(bestEta);
                    if(has_bestRemain)
                    {
                        parent->slot_delPOI_list(best);
                        delete best;
                    }
                    best=new POI(tr("Meilleure ETA: ")+tm.toString("dd MMM-hh:mm"),0,bestLat,bestLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
                    parent->slot_addPOI_list(best);
                    fromBoat.setEndPoint(bestLon,bestLat);
                    bestDistance=fromBoat.getDistance();
                }
                else
                {
                    if(bestEta>route->getEta())
                    {
                        bestEta=route->getEta();
                        bestLon=lon;
                        bestLat=lat;
                        parent->slot_delPOI_list(best);
                        delete best;
                        tm.setTimeSpec(Qt::UTC);
                        tm.setTime_t(bestEta);
                        best=new POI(tr("Meilleure ETA: ")+tm.toString("dd MMM-hh:mm"),0,bestLat,bestLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
                        parent->slot_addPOI_list(best);
                        fromBoat.setEndPoint(bestLon,bestLat);
                        bestDistance=fromBoat.getDistance();
                    }
                    else if(bestEta==route->getEta())
                    {
                        fromBoat.setEndPoint(lon,lat);
                        if(fromBoat.getDistance()>bestDistance)
                        {
                            bestEta=route->getEta();
                            bestLon=lon;
                            bestLat=lat;
                            parent->slot_delPOI_list(best);
                            delete best;
                            tm.setTimeSpec(Qt::UTC);
                            tm.setTime_t(bestEta);
                            best=new POI(tr("Meilleure ETA: ")+tm.toString("dd MMM-hh:mm"),0,bestLat,bestLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
                            parent->slot_addPOI_list(best);
                            bestDistance=fromBoat.getDistance();
                        }
                    }
                }
            }
            else
            {
                if (!has_bestEta)
                {
                    if(!has_bestRemain || bestRemain>route->getRemain())
                    {
                        if(has_bestRemain)
                        {
                            parent->slot_delPOI_list(best);
                            delete best;
                        }
                        has_bestRemain=true;
                        bestRemain=route->getRemain();
                        bestLon=lon;
                        bestLat=lat;
                        best=new POI(tr("Meilleure distance restante: ")+r.sprintf("%.2f milles",bestRemain),
                                     0,bestLat,bestLon,this->proj,this->owner,this->parent,0,0,false,this->myBoat);
                        parent->slot_addPOI_list(best);
                    }
                }
            }
            setLatitude(lat+sens*step);
            Util::computePos(proj,lat, lon, &pi, &pj);
            setPos(pi, pj-height/2);
            update();
            QApplication::processEvents();
        }
        if(this->abortSearch) break;
        sens=-sens;
        lon=(double)qRound((lon+step)*1000.0)/1000.0;
    }
    if((!has_bestEta && !has_bestRemain))
    {
        lon=savedLon;
        lat=savedLat;
        if(has_bestEta || has_bestRemain)
        {
            parent->slot_delPOI_list(best);
            delete best;
        }
    }
    else if (this->abortSearch)
    {
        int rep = QMessageBox::question (parent,tr("Abandon du positionnement automatique"), tr("Souhaitez vous conserver la meilleure position deja trouvee?"), QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes)
        {
            parent->slot_delPOI_list(best);
            delete best;
            if(Settings::getSetting("keepOldPoi","0").toInt()==0)
            {
                parent->slot_delPOI_list(previousMe);
                delete previousMe;
            }
            setLongitude(bestLon);
            setLatitude(bestLat);
            if(isWp) slot_setWP();
        }
        else
        {
            lon=savedLon;
            lat=savedLat;
            if(has_bestEta || has_bestRemain)
            {
                parent->slot_delPOI_list(best);
                delete best;
            }
            parent->slot_delPOI_list(previousMe);
            delete previousMe;
        }
    }
    else
    {
        parent->slot_delPOI_list(best);
        delete best;
        if(Settings::getSetting("keepOldPoi","0").toInt()==0)
        {
            parent->slot_delPOI_list(previousMe);
            delete previousMe;
        }
        setLongitude(bestLon);
        setLatitude(bestLat);
        if(isWp) slot_setWP();
    }
    Util::computePos(proj,lat, lon, &pi, &pj);
    setPos(pi, pj-height/2);
    update();
    route->setOptimizing(false);
    route->setOptimizingPOI(false);
    route->setFastVmgCalc(false);
    route->slot_recalculate();
    QApplication::processEvents();
}

/***************************/
/* Paint & other qGraphics */
/***************************/

void POI::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    int dy = height/2;
    if(!labelHidden && !myLabelHidden)
    {
        QFontMetrics fm(font());
        if(route==NULL)
            pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
        else
        {
            switch (this->navMode)
            {
            case 0:
                pnt->fillRect(9,0, width-10,height-1, QBrush(QColor(147,255,147,160)));
                break;
            case 1:
                pnt->fillRect(9,0, width-10,height-1, QBrush(QColor(168,226,255,160)));
                break;
            case 2:
                pnt->fillRect(9,0, width-10,height-1, QBrush(QColor(255,168,198,160)));
                break;
            default:
                pnt->fillRect(9,0, width-10,height-1, QBrush(QColor(255,0,0,60)));
                break;
            }
        }
        pnt->setFont(font());
        pnt->drawText(10,fm.height()-2,my_str);
    }
    QColor myColor;
    if(isWp)
        myColor=mwpColor;
    else
        switch(type)
        {
            case 0:
                myColor=poiColor;
                break;
            case 1:
                myColor=wpColor;
                break;
            case 2:
                myColor=baliseColor;
                break;
            case 3:
                myColor=QColor(Qt::red);
                break;
         }

    QPen pen(myColor);
        pen.setWidth(4);
        pnt->setPen(pen);
        if(!myLabelHidden)
            pnt->fillRect(0,dy-3,7,7, QBrush(myColor));
        int g = 60;
        pen = QPen(QColor(g,g,g));
        pen.setWidth(1);
        pnt->setPen(pen);
        if(!labelHidden && !myLabelHidden)
            pnt->drawRect(9,0,width-10,height-1);
}

QPainterPath POI::shape() const
{    
    QPainterPath path;
    if(this->myLabelHidden)
        path.addRect(0,0,0,0);
    else
        path.addRect(0,0,width,height);
    return path;
}

QRectF POI::boundingRect() const
{
    if(this->myLabelHidden)
        return QRectF(0,0,0,0);
    else
        return QRectF(0,0,width,height);
}
