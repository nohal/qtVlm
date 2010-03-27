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
#include "boatAccount.h"
#include "Projection.h"

/**************************/
/* Init & Clean           */
/**************************/

POI::POI(QString name, int type, float lat, float lon,
                 Projection *proj, QWidget *main, myCentralWidget *parentWindow,
                 float wph,int tstamp,bool useTstamp,boatAccount *boat)
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
    this->boat=boat;

    useRouteTstamp=false;
    routeTimeStamp=-1;
    route=NULL;
    routeName="";
    this->labelHidden=parentWindow->get_shLab_st();
    qWarning() << "Init POI label: " << labelHidden;

    WPlon=WPlat=-1;
    isWp=false;
    isMoving=false;
    VLMBoardIsBusy=false;

    setZValue(Z_VALUE_POI);
    setFont(QFont("Helvetica",9));
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

    connect(this,SIGNAL(setGribDate(int)),main,SLOT(slotSetGribDate(int)));

    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));
    connect(main,SIGNAL(WPChanged(float,float)),this,SLOT(slot_WPChanged(float,float)));
    connect(main,SIGNAL(boatHasUpdated(boatAccount*)),this,SLOT(slot_updateTip(boatAccount*)));

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
    if(route!=NULL) route->removePoi(this);
}
void POI::setLongitude(float lon)
{
    /*int temp=lon*1000;
    this->lon=(float)temp/1000;*/

    this->lon=lon;

    if(this->lon>180) this->lon=this->lon-360;
    if(this->lon<-180) this->lon=this->lon+360;
}
void POI::setLatitude(float lat)
{
    /*int temp=lat*1000;
    this->lat=(float)temp/1000;*/
    this->lat=lat;
}
void POI::rmSignal(void)
{
    //disconnect(parent, SIGNAL(projectionUpdated()), this, SLOT(slot_updateProjection()) );
    disconnect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    disconnect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));
    disconnect(this,SIGNAL(chgWP(float,float,float)),owner,SLOT(slotChgWP(float,float,float)));

    disconnect(this,SIGNAL(setGribDate(int)),owner,SLOT(slotSetGribDate(int)));

    disconnect(owner,SIGNAL(paramVLMChanged()),this,SLOT(slot_paramChanged()));
    disconnect(owner,SIGNAL(WPChanged(float,float)),this,SLOT(slot_WPChanged(float,float)));
    disconnect(owner,SIGNAL(boatHasUpdated(boatAccount*)),this,SLOT(slot_updateTip(boatAccount*)));

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

    ac_compassLine = new QAction(tr("Tirer un cap"),popup);
    popup->addAction(ac_compassLine);
    connect(ac_compassLine,SIGNAL(triggered()),owner,SLOT(slotCompassLine()));

    ac_routeList = new QMenu(tr("Routes"));
    connect(ac_routeList,SIGNAL(triggered(QAction*)),this,SLOT(slot_routeMenu(QAction*)));
    popup->addMenu(ac_routeList);
}

/**************************/
/* Events                 */
/**************************/

void POI::mousePressEvent(QGraphicsSceneMouseEvent * e)
{
    if (e->button() == Qt::LeftButton && e->modifiers()==Qt::ShiftModifier)
    {
         if(route!=NULL)
             if(route->getFrozen()) return;
         if(VLMBoardIsBusy && this->isWp) return;
         isMoving=true;
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


        return true;
    }
    return false;
}

void POI::mouseReleaseEvent(QGraphicsSceneMouseEvent * e)
{
    if(isMoving)
    {
        double newlon,newlat;
        int new_x=scenePos().x();
        int new_y=scenePos().y()+height/2;
        proj->screen2map(new_x,new_y, &newlon, &newlat);
        setLongitude(newlon);
        setLatitude(newlat);
        Util::computePos(proj,lat, lon, &pi, &pj);
        setPos(pi, pj-height/2);
        isMoving=false;
        setCursor(Qt::PointingHandCursor);
        setName(this->name);
        update();
        if(route!=NULL) route->slot_recalculate();
        if(isWp) slot_setWP();
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
    switch(((myCentralWidget*)parent)->getCompassMode(e->scenePos().x(),e->scenePos().y()))
    {
        case 0:
            /* not showing menu line, default text*/
            ac_compassLine->setText(tr("Tirer un cap"));
            ac_compassLine->setEnabled(false);
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
    }
    else
    {
        ac_setWp->setEnabled(!((MainWindow*)owner)->getBoatLockStatus());
        ac_setGribDate->setEnabled(useTstamp || useRouteTstamp);
        ac_edit->setEnabled(true);
        ac_delPoi->setEnabled(true);
        ac_copy->setEnabled(true);
        ac_routeList->setEnabled(true);
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
    popup->exec(QCursor::pos());
}

/*********************/
/* data manipulation */
/*********************/

void POI::setName(QString name)
{
    this->name=name;
    update_myStr();
    setTip("");
}
void POI::setTip(QString tip)
{
    boatAccount * w_boat;
    tip=tip.replace(" ","&nbsp;");
    if(route==NULL)
        w_boat=boat;
    else
        w_boat=route->getBoat();
    if(w_boat)
    {
        Orthodromie orth2boat(w_boat->getLon(), w_boat->getLat(), lon, lat);
        float   distance=orth2boat.getDistance();
        QString tt=tr("Distance Ortho a partir de ")+w_boat->getLogin()+": "+
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
        if(useRouteTstamp && type==POI_TYPE_WP)
        {
            if(this->routeTimeStamp==0)
                my_str=name + " " + tr("Non joinable avec ce Grib");
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

void POI::setTimeStamp(int date)
{
    this->useTstamp=true;
    this->timeStamp=date;
    update_myStr();

}

void POI::setRouteTimeStamp(int date)
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
    if(compFloat(lat,WPlat) && compFloat(lon,WPlon))
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
    if(this->route!=NULL)
    {
        if(this->route->getFrozen()) return;
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
        setRouteTimeStamp(false);
}

/**************************/
/* Slots                  */
/**************************/

void POI::slot_updateProjection()
{
    Util::computePos(proj,lat, lon, &pi, &pj);
    int dy = height/2;
    setPos(pi, pj-dy);
}
void POI::slot_updateTip(boatAccount * boat)
{
    this->boat=boat;
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
            tr("Détruire la marque"),
            tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
    if (rep == QMessageBox::Yes) {

        emit delPOI_list(this);
        if(route!=NULL)
        {
            route->removePoi(this);
        }
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

void POI::slot_WPChanged(float tlat,float tlon)
{
    WPlat=tlat;
    WPlon=tlon;
    chkIsWP();
    VLMBoardIsBusy=false;
}
QString POI::getRouteName()
{
    return routeName;
}

void POI::slot_routeMenu(QAction* ptr_action)
{
    ROUTE *  ptr_route = (ROUTE *) ptr_action->data().value<void *>();
    setRoute(ptr_route);
}

/***************************/
/* Paint & other qGraphics */
/***************************/

void POI::paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * )
{
    int dy = height/2;
    if(!labelHidden)
    {
        QFontMetrics fm(font());
        pnt->fillRect(9,0, width-10,height-1, QBrush(bgcolor));
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
        pnt->fillRect(0,dy-3,7,7, QBrush(myColor));
        int g = 60;
        pen = QPen(QColor(g,g,g));
        pen.setWidth(1);
        pnt->setPen(pen);
        if(!labelHidden)
            pnt->drawRect(9,0,width-10,height-1);
}

QPainterPath POI::shape() const
{    
    QPainterPath path;
    path.addRect(0,0,width,height);
    return path;
}

QRectF POI::boundingRect() const
{
    return QRectF(0,0,width,height);
}
