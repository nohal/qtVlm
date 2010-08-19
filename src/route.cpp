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
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

#include "route.h"

#include "Orthodromie.h"
#include "Projection.h"
#include "Grib.h"
#include "mycentralwidget.h"
#include "vlmLine.h"
#include "POI.h"
#include "Route_Editor.h"
#include "boatAccount.h"
#include "Polar.h"
#include "Util.h"

ROUTE::ROUTE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow)
            : QGraphicsWidget()

{
    this->proj=proj;
    this->name=name;
    this->myscene=myScene;
    this->grib=grib;
    this->parent=parentWindow;
    this->color=Qt::red;
    this->width=2;
    this->startFromBoat=true;
    this->startTimeOption=1;
    this->line=new vlmLine(proj,myScene,Z_VALUE_ROUTE);
    this->frozen=false;
    this->superFrozen=false;
    this->live=true;
    this->optimizing=false;
    this->optimizingPOI=false;
    this->busy=false;
    this->startTime= QDateTime::currentDateTime().toUTC();
    this->has_eta=false,
    this->eta=0;
    this->remain=0;
    this->boat=NULL;
    this->boatLogin="";
    this->my_poiList.clear();
    this->fastVmgCalc=false;
    this->startPoiName="";
    this->hasStartEta=false;
    this->startEta=0;
    this->hidePois=false;
    this->imported=false;
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(width);
    connect (parent,SIGNAL(boatPointerHasChanged(boatAccount*)),this,SLOT(slot_boatPointerHasChanged(boatAccount*)));
    if(!parentWindow->get_shRoute_st())
        slot_shShow();
    else
        slot_shHidden();
}
ROUTE::~ROUTE()
{
    if(line)
    {
        if(!parent->getAboutToQuit())
            delete line;
    }
}
void ROUTE::setBoat(boatAccount *boat)
{
    this->boat=boat;
    if(boat!=NULL)
        this->boatLogin=boat->getLogin();
    else
        this->boatLogin="";
}
void ROUTE::setWidth(float width)
{
    this->width=width;
    pen.setWidthF(width);
    line->setLinePen(pen);
}
void ROUTE::setColor(QColor color)
{
    this->color=color;
    pen.setColor(color);
    pen.setBrush(color);
    line->setLinePen(pen);
}
void ROUTE::insertPoi(POI *poi)
{
    my_poiList.append(poi);
    connect(poi,SIGNAL(poiMoving()),this,SLOT(slot_recalculate()));
    slot_recalculate();
}
void ROUTE::removePoi(POI *poi)
{
    my_poiList.removeAll(poi);
    disconnect(poi,SIGNAL(poiMoving()),this,SLOT(slot_recalculate()));
    slot_recalculate();
}
void ROUTE::slot_recalculate()
{
//    if(!boat->getStatus())
//    {
//        return parent->deleteRoute(this);
//    }
    if(busy)
    {
//        qWarning() << "Route recalculate already busy";
        return;
    }
    if (frozen) interpolatePos();
    if(frozen || superFrozen) return;
    line->deleteAll();
    line->setHasInterpolated(false);
    line->setLinePen(pen);
    if(my_poiList.count()==0) return;
    busy=true;
    eta=0;
    has_eta=false;
    time_t now;
    if(boat && boat->getPolarData() && grib && boat!=NULL)
    {
        switch(startTimeOption)
        {
            case 1:
                eta=boat->getPrevVac();
                now = (QDateTime::currentDateTime()).toUTC().toTime_t();
#warning find a better way to identify a boat that has not yet started
                if(eta < now - 2*boat->getVacLen()) /*cas du boat inscrit depuis longtemps mais pas encore parti*/
                    eta=now;
                //qWarning() << "Depart route: " << now;
                break;
            case 2:
                eta=grib->getCurrentDate();
                break;
            case 3:
                eta=startTime.toUTC().toTime_t();
                break;
        }
        start=eta;
        has_eta=true;
        Orthodromie orth(0,0,0,0);
        qSort(my_poiList.begin(),my_poiList.end(),POI::myLessThan);
        QListIterator<POI*> i (my_poiList);
        QString tip;
        double lon,lat;
        if(startFromBoat)
        {
            lon=boat->getLon();
            lat=boat->getLat();
        }
        else
        {
            POI * poi;
            poi=i.next();
            lon=poi->getLongitude();
            lat=poi->getLatitude();
            tip="<br>Starting point for route "+name;
            poi->setRouteTimeStamp((int)eta);
            poi->setTip(tip);
        }
        startLat=lat;
        startLon=lon;
        if(parent->getCompassFollow()==this)
            parent->centerCompass(lon,lat);
        if(!optimizing && (!optimizingPOI || !hasStartEta))
        {
            vlmPoint p(lon,lat);
            p.eta=eta;
            line->addVlmPoint(p);
        }
        double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,previous_remaining_distance,cap1,cap2,diff1,diff2;
        double wind_angle,wind_speed,cap,angle;
        time_t mult_vac=1;
        time_t maxDate=grib->getMaxDate();
        QString previousPoiName="";
        time_t previousEta=0;
        time_t lastEta=0;
        time_t gribDate=grib->getCurrentDate();
        while(i.hasNext())
        {
            POI * poi = i.next();
            if(optimizingPOI && hasStartEta)
            {
                if(poi->getName()<startPoiName)
                    continue;
                if(poi->getName()==startPoiName)
                {
                    lon=poi->getLongitude();
                    lat=poi->getLatitude();
                    eta=startEta;
                    vlmPoint p(lon,lat);
                    p.eta=eta;
                    line->addVlmPoint(p);
                    continue;
                }
            }
            if(!grib->isOk() && !imported)
            {
                tip="<br>Estimated ETA: No grib loaded" ;
                poi->setRouteTimeStamp(-1);
                poi->setTip(tip);
                has_eta=false;
//                qWarning()<<"no grib loaded for route!!";
                continue;
            }
            newSpeed=0;
            distanceParcourue=0;
            res_lon=0;
            res_lat=0;
            previous_remaining_distance=0;
            wind_angle=0;
            wind_speed=0;
            orth.setPoints(lon, lat, poi->getLongitude(),poi->getLatitude());
            remaining_distance=orth.getDistance();
            if(has_eta)
            {
                do
                {                    
                    if((grib->getInterpolatedValue_byDates(lon, lat,
                                              eta,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA)
                        && eta<=maxDate || imported))
                    {
                        //qWarning() << lon << ";" << lat << ";" << eta << ";" << wind_speed << ";" << wind_angle;
                        previous_remaining_distance=remaining_distance;
                        wind_angle=radToDeg(wind_angle);
                        cap=orth.getAzimutDeg();
                        if(imported)
                        {
                            res_lon=poi->getLongitude();
                            res_lat=poi->getLatitude();
                        }
                        else
                        {
                            switch (poi->getNavMode())
                            {
                                case 0: //VBVMG
                                    angle=cap-wind_angle;
                                    if(qAbs(angle)>180)
                                    {
                                        if(angle<0)
                                            angle=360+angle;
                                        else
                                            angle=angle-360;
                                    }
                                    if(qAbs(angle)<boat->getBvmgUp(wind_speed))
                                    {
                                        angle=boat->getBvmgUp(wind_speed);
                                        cap1=A360(wind_angle+angle);
                                        cap2=A360(wind_angle-angle);
                                        diff1=myDiffAngle(cap,cap1);
                                        diff2=myDiffAngle(cap,cap2);
                                        if(diff1<diff2)
                                            cap=cap1;
                                        else
                                            cap=cap2;
                                    }
                                    else if(qAbs(angle)>boat->getBvmgDown(wind_speed))
                                    {
                                        angle=boat->getBvmgDown(wind_speed);
                                        cap1=A360(wind_angle+angle);
                                        cap2=A360(wind_angle-angle);
                                        diff1=myDiffAngle(cap,cap1);
                                        diff2=myDiffAngle(cap,cap2);
                                        if(diff1<diff2)
                                            cap=cap1;
                                        else
                                            cap=cap2;
                                    }
                                    break;
                                case 1: //BVMG
                                    if(fastVmgCalc)
                                        boat->getPolarData()->getBvmg((cap-wind_angle),wind_speed,&angle);
                                    else
                                        boat->getPolarData()->bvmgWind((cap-wind_angle),wind_speed,&angle);
    #if 0
                                    if(qRound(angle*100.00)!=qRound(angleDebug*100.00))
                                        qWarning()<<"angle="<<angle<<" angleDebug="<<angleDebug;
    #endif
                                    cap=A360(angle+wind_angle);
                                    break;
                                case 2: //ORTHO
                                    angle=cap-wind_angle;
                                    if(qAbs(angle)>180)
                                    {
                                        if(angle<0)
                                            angle=360+angle;
                                        else
                                            angle=angle-360;
                                    }
                                    break;
                            }
                            newSpeed=boat->getPolarData()->getSpeed(wind_speed,angle);
                            distanceParcourue=newSpeed*boat->getVacLen()*mult_vac/3600.00;
                            Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                        }
                        orth.setStartPoint(res_lon, res_lat);
                        remaining_distance=orth.getDistance();
                        if(remaining_distance>previous_remaining_distance)
                        {
//                            if (remaining_distance-previous_remaining_distance>10)
//                                qWarning()<<(remaining_distance-previous_remaining_distance)<<" milles in one vac, not bad";
                            break;
                        }
                        lon=res_lon;
                        lat=res_lat;
                        if(imported)
                            eta=poi->getRouteTimeStamp();
                        else
                            eta= eta + boat->getVacLen()*mult_vac;
                        //qWarning() << "" << eta << ";" << wind_angle << ";" << wind_speed << ";" << newSpeed << ";" << distanceParcourue << ";" << remaining_distance;
                        if (!optimizing)
                        {
                            if(lastEta<gribDate && eta>=gribDate && gribDate>start+1000)
                            {
                                line->setInterpolated(lon,lat);
                                line->setHasInterpolated(true);
                                if(parent->getCompassFollow()==this)
                                    parent->centerCompass(lon,lat);
                            }
                            vlmPoint p(lon,lat);
                            p.eta=eta;
                            line->addVlmPoint(p);
                            lastEta=eta;
                        }
                    }
                    else
                    {
                        has_eta=false;
                        orth.setPoints(res_lon,res_lat,my_poiList.last()->getLongitude(),my_poiList.last()->getLatitude());
                        remain=orth.getDistance();
                    }
                } while (remaining_distance>distanceParcourue/2.000 && has_eta && !imported);
            }
//            qWarning()<<"Distance Parcourue="<<distanceParcourue<<" remaining_distance="<<remaining_distance<<" previous_rd="<<previous_remaining_distance;
//            qWarning()<<"newSpeed="<<newSpeed<<" wind_speed="<<wind_speed<<" angle="<<angle;
            if(!has_eta)
            {
                tip=tr("<br>ETA: Non joignable avec ce fichier GRIB");
                poi->setRouteTimeStamp(-1);
            }
            else
            {
                //qWarning()<<"eta arrivee "<<eta;
                float days=(eta-start)/86400.0000;
                if(qRound(days)>days)
                    days=qRound(days)-1;
                else
                    days=qRound(days);
                float hours=(eta-start-days*86400)/3600.0000;
                if(qRound(hours)>hours)
                    hours=qRound(hours)-1;
                else
                    hours=qRound(hours);
                float mins=qRound((eta-start-days*86400-hours*3600)/60.0000);
                QString tt;
                QDateTime tm;
                tm.setTimeSpec(Qt::UTC);
                tm.setTime_t(start);
                //qWarning()<<"tm="<<tm.toString();
                switch(startTimeOption)
                {
                    case 1:
                            tt="<br>"+tr("ETA depuis la derniere vacation")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                    case 2:
                            tt="<br>"+tr("ETA depuis la date Grib")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                    case 3:
                            tt="<br>"+tr("ETA depuis la date fixe")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                }
                tip=tt+QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                    QString::number((int)mins)+" "+tr("minutes");
                poi->setRouteTimeStamp(eta);
            }
            poi->setTip(tip);
            lon=poi->getLongitude();
            lat=poi->getLatitude();
            if(optimizingPOI)
            {
                if(previousPoiName==poiName)
                    break;
                if(!hasStartEta)
                {
                    startEta=previousEta;
                    startPoiName=previousPoiName;
                }
                previousPoiName=poi->getName();
                previousEta=eta;
            }
        }
    }
    if(!optimizing)
        line->slot_showMe();
    if(optimizingPOI)
    {
        if(startPoiName!="")
            hasStartEta=true;
    }
    busy=false;
    setHidePois(this->hidePois);
}
void ROUTE::interpolatePos()
{
    line->setHasInterpolated(false);
    if(!grib || !grib->isOk())
        return;
    QList<vlmPoint> *list=line->getPoints();
    if (list->count()==0) return;
    time_t lastEta=list->at(0).eta;
    time_t gribDate=grib->getCurrentDate();
    if(parent->getCompassFollow()==this)
        parent->centerCompass(list->at(0).lon,list->at(0).lat);
    if(gribDate<lastEta+1000) return;
    for (int n=1; n<list->count();n++)
    {
        if(lastEta<gribDate && list->at(n).eta>=gribDate)
        {
            line->setInterpolated(list->at(n).lon,list->at(n).lat);
            line->setHasInterpolated(true);
            if(parent->getCompassFollow()==this)
                parent->centerCompass(list->at(n).lon,list->at(n).lat);
            break;
        }
        lastEta=list->at(n).eta;
    }
}
double ROUTE::A360(double hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}

bool ROUTE::isPartOfBvmg(POI * poi)
{
    if (poi->getNavMode()==1) return true;
    if (my_poiList.last()==poi) return false;
    if (my_poiList.at(my_poiList.indexOf(poi)+1)->getNavMode()==1) return true;
    return false;
}
double ROUTE::myDiffAngle(double a1,double a2)
{
    return qAbs(A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
}
void ROUTE::slot_delete()
{
    int rep = QMessageBox::question (parent,
            tr("Détruire la route : %1").arg(name),
            tr("La destruction d'une route est definitive.\n\nVoulez-vous egalement supprimer tous les POIs lui appartenant?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (rep == QMessageBox::Cancel) return;
    QListIterator<POI*> i (my_poiList);
    frozen=false;
    superFrozen=true;
    while(i.hasNext())
    {
        POI * poi = i.next();
        poi->setRoute(NULL);
        poi->setMyLabelHidden(false);
        if(rep==QMessageBox::Yes)
        {
            emit deletePoi(poi);
            delete poi;
        }
    }
    parent->deleteRoute(this);
}
void ROUTE::slot_edit()
{
    emit editMe(this);
}

void ROUTE::slot_shShow()
{
    show();
    if(line)
        line->show();
}

void ROUTE::slot_shHidden()
{
    hide();
    if(line)
        line->hide();
}
void ROUTE::slot_boatPointerHasChanged(boatAccount * acc)
{
    if(acc->getLogin()==this->boatLogin)
        this->setBoat(acc);
}
void ROUTE::setHidePois(bool b)
{
    this->hidePois=b;
    QListIterator<POI*> i (my_poiList);
    while(i.hasNext())
    {
        POI * poi=i.next();
        if (poi==my_poiList.first() || poi==my_poiList.last())
            poi->setMyLabelHidden(false);
        else
            poi->setMyLabelHidden(this->hidePois);
    }
}
