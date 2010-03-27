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
    this->startTime= QDateTime::currentDateTime().toUTC();
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(width);
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
    if(frozen || superFrozen) return;
    line->deleteAll();
    line->setLinePen(pen);
    if(my_poiList.count()==0) return;
    busy=true;
    time_t eta=0;
    time_t now;
    if(boat && boat->getPolarData() && grib && boat!=NULL)
    {
        switch(startTimeOption)
        {
            case 1:
                eta=boat->getPrevVac();
                now = (QDateTime::currentDateTime()).toUTC().toTime_t();
                if(eta < now - boat->getVacLen()) /*cas du boat inscrit depuis longtemps mais pas encore parti*/
                    eta=now;
                break;
            case 2:
                eta=grib->getCurrentDate();
                break;
            case 3:
                eta=startTime.toUTC().toTime_t();
                break;
        }
        time_t start=eta;
        bool has_eta=true;
        Orthodromie orth(0,0,0,0);
        qSort(my_poiList.begin(),my_poiList.end(),POI::myLessThan);
        QListIterator<POI*> i (my_poiList);
        QString tip;
        float lon,lat;
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
        line->addPoint(lat,lon);
        float newSpeed,distanceParcourue,remaining_distance,angle,res_lon,res_lat,previous_remaining_distance,cap,cap1,cap2,diff1,diff2;
        double wind_angle,wind_speed;
        time_t mult_vac=1;
        time_t maxDate=grib->getMaxDate();
        while(i.hasNext())
        {
            POI * poi = i.next();
            if(!grib->isOk())
            {
                tip="<br>Estimated ETA: No grib loaded" ;
                poi->setRouteTimeStamp(-1);
                poi->setTip(tip);
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
                    if(grib->getInterpolatedValue_byDates((double) lon,(double) lat,
                                              eta,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA) && eta<=maxDate)
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
                        newSpeed=boat->getPolarData()->getSpeed(wind_speed,angle);
                        distanceParcourue=newSpeed*boat->getVacLen()*mult_vac/3600.00;
                        Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                        orth.setStartPoint(res_lon, res_lat);
                        remaining_distance=orth.getDistance();
                        if(remaining_distance>previous_remaining_distance) break;
                        lon=res_lon;
                        lat=res_lat;
                        eta= eta + boat->getVacLen()*mult_vac;
                        line->addPoint(lat,lon);
                    }
                    else
                        has_eta=false;
                } while (remaining_distance>distanceParcourue/2.000 && has_eta);
            }
            if(!has_eta)
            {
                tip="<br>Estimated ETA: Unreachable within Grib" ;
                poi->setRouteTimeStamp(-1);
            }
            else
            {
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
                poi->setRouteTimeStamp((int)eta);
            }
            poi->setTip(tip);
            lon=poi->getLongitude();
            lat=poi->getLatitude();
        }
    }
    line->slot_showMe();
    busy=false;
}
float ROUTE::A360(float hdg)
{
    if(hdg>=360) hdg=hdg-360;
    if(hdg<0) hdg=hdg+360;
    return hdg;
}
float ROUTE::myDiffAngle(float a1,float a2)
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
