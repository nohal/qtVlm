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
#include "DialogRoute.h"
#include "boatVLM.h"
#include "boatReal.h"
#include "Polar.h"
#include "Util.h"
#include "settings.h"
#include <QCache>
#define USE_VBVMG_VLM

ROUTE::ROUTE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow)
            : QObject()

{
    this->proj=proj;
    this->name=name;
    this->myscene=myScene;
    this->grib=grib;
    this->parent=parentWindow;
    this->color=Settings::getSetting("routeLineColor", Qt::yellow).value<QColor>();
    this->width=Settings::getSetting("routeLineWidth", 2.0).toDouble();
    this->startFromBoat=true;
    this->startTimeOption=1;
    this->line=new vlmLine(proj,myScene,Z_VALUE_ROUTE);
    line->setParent(this);
    this->frozen=false;
    this->superFrozen=false;
    this->detectCoasts=true;
    this->optimizing=false;
    this->optimizingPOI=false;
    this->busy=false;
    this->startTime= QDateTime::currentDateTime().toUTC();
    this->has_eta=false,
    this->eta=0;
    this->remain=0;
    this->myBoat=NULL;
    this->boatLogin="";
    this->my_poiList.clear();
    this->fastVmgCalc=false;
    this->startPoiName="";
    this->hasStartEta=false;
    this->startEta=0;
    this->hidePois=false;
    this->imported=false;
    this->multVac=1;
    this->useVbvmgVlm=false;
    this->setNewVbvmgVlm(false);
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(width);
    this->initialized=false;
    connect (parent,SIGNAL(boatPointerHasChanged(boat*)),this,SLOT(slot_boatPointerHasChanged(boat*)));
    this->simplify=false;
    this->hidden=false;
    this->temp=false;
    if(!parentWindow->get_shRoute_st())
        slot_shShow();
    else
        slot_shHidden();
    this->speedLossOnTack=1;
    this->autoRemove=Settings::getSetting("autoRemovePoiFromRoute",0).toInt()==1;
    this->autoAt=Settings::getSetting("autoFillPoiHeading",0).toInt()==1;
    this->pilototo=false;
    this->initialDist=0;
    this->roadMapInterval=1;
    this->precalculateTan();
}

ROUTE::~ROUTE()
{
    qWarning() << "Deleting route: " << name;


    if(line)
    {
        if(!parent->getAboutToQuit())
            delete line;
    }
    delete tanPos;
    delete tanNeg;
    delete hypotPos;
    delete hypotNeg;
}

void ROUTE::setBoat(boat *curBoat)
{
    this->myBoat=curBoat;
    if(myBoat!=NULL && myBoat->getType()==BOAT_VLM)
        this->boatLogin=((boatVLM*)myBoat)->getBoatPseudo();
    else
        this->boatLogin="";
    if(frozen)
    {
        setFrozen(false);
        setFrozen(true);
    }
}
void ROUTE::setWidth(double width)
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
void ROUTE::slot_recalculate(boat * boat)
{
    QTime timeTotal;
    timeTotal.start();
    QTime timeDebug;
    int timeD=0;
    int nbLoop=0;
    if(parent->getAboutToQuit()) return;
    if(temp) return;
    if(busy)
    {
        //busy=false; /*recursion*/
        return;
    }
    if(boat!=NULL && this->myBoat!=boat) return;
    if(!myBoat || myBoat==NULL  || !myBoat->getStatus()) return;
    if(this->hidden) return;
    if (frozen && initialized)
    {
        interpolatePos();
        return;
    }
    if(initialized && (frozen || superFrozen)) return;
    line->deleteAll();
    line->setHasInterpolated(false);
    line->setLinePen(pen);
    line->setCoastDetection(false);
    line->slot_showMe();
    roadMap.clear();
    if(my_poiList.count()==0) return;
    busy=true;
    qSort(my_poiList.begin(),my_poiList.end(),POI::myLessThan);
    if(!this->optimizing && !this->optimizingPOI && autoRemove && this->startFromBoat)
    {
        bool foundWP=false;
        int n=0;
        for (n=0;n<this->my_poiList.count();++n)
        {
            if(my_poiList.at(n)->getIsWp())
            {
                foundWP=true;
                break;
            }
        }
        if(foundWP)
        {
            POI * wp=my_poiList.at(n);
            while(my_poiList.first()!=wp)
            {
                my_poiList.first()->setMyLabelHidden(false);
                my_poiList.first()->setRoute(NULL);
            }
            if(my_poiList.count()==0)
            {
                busy=false;
                return;
            }
        }
    }
    bool firstPoint;
    double lastTwa=0;
    eta=0;
    has_eta=false;
    time_t now;
    if(myBoat  && myBoat!=NULL && myBoat->getPolarData() && grib)
    {
        initialized=true;
        switch(startTimeOption)
        {
            case 1:
                if(myBoat->getType()!=BOAT_VLM)
                    eta=((boatReal*)myBoat)->getLastUpdateTime();
                else
                    eta=((boatVLM*)myBoat)->getPrevVac()/*+((boatVLM*)myBoat)->getVacLen()*/;
                now = (QDateTime::currentDateTime()).toUTC().toTime_t();
#warning find a better way to identify a boat that has not yet started
/*cas du boat inscrit depuis longtemps mais pas encore parti*/
                //if(eta < now - 2*myBoat->getVacLen() && myBoat->getType()==BOAT_VLM)
                if(myBoat->getLoch()<0.01 && myBoat->getType()==BOAT_VLM)
                    eta=now;
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
        //qWarning()<<"??"<<my_poiList.first()->getName();
        QListIterator<POI*> i (my_poiList);
        QString tip;
        double lon,lat;
        if(startFromBoat)
        {
            lon=myBoat->getLon();
            lat=myBoat->getLat();
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
            p.isPOI=true;
            line->addVlmPoint(p);
        }
        double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,previous_remaining_distance,cap1,cap2,diff1,diff2;
        double wind_angle,wind_speed,cap,angle;
        cap=-1;
        time_t maxDate=grib->getMaxDate();
        QString previousPoiName="";
        time_t previousEta=0;
        time_t lastEta=0;
        time_t gribDate=grib->getCurrentDate();
        if(this->my_poiList.isEmpty())
            initialDist=0;
        else
        {
            orth.setPoints(lon, lat, my_poiList.last()->getLongitude(),my_poiList.last()->getLatitude());
            initialDist=orth.getDistance();
        }
        double poiNb=-1;
        while(i.hasNext())
        {
            ++poiNb;
            int nbToReach=0;
            bool badEta=false;
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
                tip=tr("<br>Route: ")+name;
                tip=tip+"<br>Estimated ETA: No grib loaded" ;
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
            //qWarning()<<poi->getName()<<this->my_poiList.at(0)->getName();
            remaining_distance=orth.getDistance();
            time_t Eta=0;
            if(has_eta)
            {
                //if(!busy) this->slot_recalculate();
                if(parent->getAboutToQuit()) return;
                do
                {
                    if(imported)
                        eta=poi->getRouteTimeStamp();
                    else
                        eta= eta + myBoat->getVacLen()*multVac;
                    Eta=eta;
                    if(((grib->getInterpolatedValue_byDates(lon, lat,
                                              eta,&wind_speed,&wind_angle,INTERPOLATION_DEFAULT)
                            && eta<=maxDate) || imported))
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
                                {
#if 0
                                    if(useVbvmgVlm && !this->fastVmgCalc && !parent->getIsStartingUp())
                                    {
                                        line->slot_showMe();
                                        QApplication::processEvents();
                                        double h1,h2,w1,w2,t1,t2,d1,d2;
                                        this->do_vbvmg_context(remaining_distance,cap,wind_speed,wind_angle,&h1,&h2,&w1,&w2,&t1,&t2,&d1,&d2);
                                        angle=A180(w1);
                                        //qWarning()<<"cap="<<cap<<"h1="<<h1<<"twa="<<angle;
                                        cap=h1;
                                    }
#endif
                                    if(useVbvmgVlm && !this->fastVmgCalc && !parent->getIsStartingUp())
                                    {
                                        if(++nbLoop>100)
                                        {
                                            nbLoop=0;
                                            line->slot_showMe();
                                            QApplication::processEvents();
                                        }
                                        double h1,h2,w1,w2,t1,t2,d1,d2;
#if 0
                                        this->do_vbvmg_context(remaining_distance,cap,wind_speed,wind_angle,&h1,&h2,&w1,&w2,&t1,&t2,&d1,&d2);
                                        double w1Save=w1;
                                        double w2Save=w2;
                                        double h1Save=h1;
#endif
                                        timeDebug.start();
                                        this->do_vbvmg_buffer(remaining_distance,cap,wind_speed,wind_angle,&h1,&h2,&w1,&w2,&t1,&t2,&d1,&d2);
                                        timeD+=timeDebug.elapsed();
#if 0
                                        //qWarning()<<w1<<w2;
                                        if(qRound(w1*1000)!=qRound(w1Save*1000) || qRound(h1*1000)!=qRound(h1Save*1000))
                                        {
                                            qWarning()<<"error in new vbvmg vlm"<<h1<<w1<<w2<<"should be"<<h1Save<<w1Save<<w2Save;
                                            this->do_vbvmg_buffer(remaining_distance,cap,wind_speed,wind_angle,&h1,&h2,&w1,&w2,&t1,&t2,&d1,&d2);
                                        }
#endif
                                        angle=A180(w1);
                                        //qWarning()<<"cap="<<cap<<"h1="<<h1<<"twa="<<angle;
                                        cap=h1;
                                    }
                                    else
                                    {
                                        angle=A180(cap-wind_angle);
                                        if(qAbs(angle)<myBoat->getBvmgUp(wind_speed))
                                        {
                                            angle=myBoat->getBvmgUp(wind_speed);
                                            cap1=A360(wind_angle+angle);
                                            cap2=A360(wind_angle-angle);
                                            diff1=myDiffAngle(cap,cap1);
                                            diff2=myDiffAngle(cap,cap2);
                                            if(diff1<diff2)
                                                cap=cap1;
                                            else
                                                cap=cap2;
                                        }
                                        else if(qAbs(angle)>myBoat->getBvmgDown(wind_speed))
                                        {
                                            angle=myBoat->getBvmgDown(wind_speed);
                                            cap1=A360(wind_angle+angle);
                                            cap2=A360(wind_angle-angle);
                                            diff1=myDiffAngle(cap,cap1);
                                            diff2=myDiffAngle(cap,cap2);
                                            if(diff1<diff2)
                                                cap=cap1;
                                            else
                                                cap=cap2;
                                        }
                                    }
                                    break;
                                }

                                case 1: //BVMG
                                    if(fastVmgCalc)
                                        myBoat->getPolarData()->getBvmg((cap-wind_angle),wind_speed,&angle);
                                    else
                                        myBoat->getPolarData()->bvmgWind((cap-wind_angle),wind_speed,&angle);
    #if 0
                                    if(qRound(angle*100.00)!=qRound(angleDebug*100.00))
                                        qWarning()<<"angle="<<angle<<" angleDebug="<<angleDebug;
    #endif
                                    cap=A360(angle+wind_angle);
                                    break;
                                case 2: //ORTHO
                                    angle=A180(cap-wind_angle);
                                    break;
                            }

                            newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,angle);
                            if (firstPoint)
                            {
                                firstPoint=false;
                            }
                            else if (speedLossOnTack!=1)
                            {
                                if ((angle>0 && lastTwa<0)||(angle<0 && lastTwa>0))
                                {
                                    //qWarning()<<"reducing polar because of tack/gybe";
                                    newSpeed=newSpeed*speedLossOnTack;
                                }
                            }
                            lastTwa=angle;
                            distanceParcourue=newSpeed*myBoat->getVacLen()*multVac/3600.00;
                            if(!imported && nbToReach==0)
                            {
                                if(qRound(distanceParcourue*100)>=qRound(remaining_distance*100))
                                {
                                    badEta=true;
                                    eta=eta-myBoat->getVacLen()*multVac;
                                    Eta=eta;
                                    if(!this->getSimplify() && poi==my_poiList.last())
                                    {
                                        QList<double> roadPoint;
                                        roadPoint.append((double)Eta); // 0
                                        roadPoint.append(0); // 1
                                        roadPoint.append(0); // 2
                                        roadPoint.append(0); //3
                                        roadPoint.append(-1); //4
                                        roadPoint.append(0); //5
                                        roadPoint.append(0); //6
                                        roadPoint.append(0); //7
                                        roadPoint.append(0); //8
                                        roadPoint.append(-1); //9
                                        roadPoint.append(0);
                                        roadMap.append(roadPoint);
                                    }
                                    break;
                                }
                            }
                            Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                        }
                        orth.setStartPoint(res_lon, res_lat);
                        remaining_distance=orth.getDistance();
                        lon=res_lon;
                        lat=res_lat;
                        //qWarning() << "" << eta << ";" << wind_angle << ";" << wind_speed << ";" << newSpeed << ";" << distanceParcourue << ";" << remaining_distance;
                        ++nbToReach;
                        if (!optimizing)
                        {
                            if(lastEta<gribDate && Eta>=gribDate && gribDate>start+1000)
                            {
                                line->setInterpolated(lon,lat);
                                line->setHasInterpolated(true);
                                if(parent->getCompassFollow()==this)
                                    parent->centerCompass(lon,lat);
                            }
                            vlmPoint p(lon,lat);
                            p.eta=Eta;
                            line->addVlmPoint(p);
                            if(!this->getSimplify())
                            {
                                QList<double> roadPoint;
                                roadPoint.append((double)(Eta-myBoat->getVacLen())); // 0
                                roadPoint.append(poi->getLongitude()); // 1
                                roadPoint.append(poi->getLatitude()); // 2
                                roadPoint.append(cap); //3
                                roadPoint.append(newSpeed); //4
                                roadPoint.append(distanceParcourue); //5
                                roadPoint.append(wind_angle); //6
                                roadPoint.append(wind_speed); //7
                                roadPoint.append(A180(cap-wind_angle)); //8
                                roadPoint.append(poiNb); //9
                                roadPoint.append(remaining_distance); //10
                                roadMap.append(roadPoint);
                            }
                            lastEta=Eta;
                        }
                    }
                    else
                    {
                        has_eta=false;
                        orth.setPoints(res_lon,res_lat,my_poiList.last()->getLongitude(),my_poiList.last()->getLatitude());
                            remain=orth.getDistance();
                    }
//                    if(poi->getIsWp())
//                        qWarning()<<"Eta:"<<QDateTime().fromTime_t(Eta).toUTC().toString("dd MMM-hh:mm")<<"remaining dist"<<remaining_distance<<"distanceParcourue:"<<distanceParcourue;
                    if(!imported &&(qRound(remaining_distance*100)<qRound(distanceParcourue*100) /* ||
                       qRound(previous_remaining_distance*100)<qRound(distanceParcourue*100)*/))
                    {
                        if(!this->getSimplify() && poi==my_poiList.last())
                        {
                            QList<double> roadPoint;
                            roadPoint.append((double)Eta); // 0
                            roadPoint.append(0); // 1
                            roadPoint.append(0); // 2
                            roadPoint.append(0); //3
                            roadPoint.append(-1); //4
                            roadPoint.append(0); //5
                            roadPoint.append(0); //6
                            roadPoint.append(0); //7
                            roadPoint.append(0); //8
                            roadPoint.append(-1); //9
                            roadPoint.append(0); //10
                            roadMap.append(roadPoint);
                        }
                        break;
                    }
                } while (has_eta && !imported);
            }
            if(this->autoAt)
            {
                poi->setWph(qRound(cap*100)/100.0);
            }
//            qWarning()<<"Distance Parcourue="<<distanceParcourue<<" remaining_distance="<<remaining_distance<<" previous_rd="<<previous_remaining_distance;
//            qWarning()<<"newSpeed="<<newSpeed<<" wind_speed="<<wind_speed<<" angle="<<angle;
            line->setLastPointIsPoi();
            tip=tr("<br>Route: ")+name;
            //time_t Eta=eta-myBoat->getVacLen();
//            if(badEta)
//                Eta=eta-myBoat->getVacLen();
            if(!has_eta)
            {
                tip=tip+tr("<br>ETA: Non joignable avec ce fichier GRIB");
                poi->setRouteTimeStamp(-1);
            }
            else if(Eta-start<=0)
            {
                tip=tip+tr("<br>ETA: deja atteint");
                poi->setRouteTimeStamp(Eta);
            }
            else
            {
                tip=tip+"<br>"+tr("Note: la date indiquee correspond a la desactivation du WP");
                time_t Start=start;
                if(startTimeOption==1)
                    Start=QDateTime::currentDateTimeUtc().toTime_t();
                //qWarning()<<"eta arrivee "<<eta;
                double days=(Eta-Start)/86400.0000;
                if(qRound(days)>days)
                    days=qRound(days)-1;
                else
                    days=qRound(days);
                double hours=(Eta-Start-days*86400)/3600.0000;
                if(qRound(hours)>hours)
                    hours=qRound(hours)-1;
                else
                    hours=qRound(hours);
                double mins=qRound((Eta-Start-days*86400-hours*3600)/60.0000);
                QString tt;
                QDateTime tm;
                tm.setTimeSpec(Qt::UTC);
                tm.setTime_t(Start);
                //qWarning()<<"tm="<<tm.toString();
                switch(startTimeOption)
                {
                    case 1:
                            tt="<br>"+tr("ETA a partir de maintenant")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                    case 2:
                            tt="<br>"+tr("ETA depuis la date Grib")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                    case 3:
                            tt="<br>"+tr("ETA depuis la date fixe")+" ("+tm.toString("dd MMM-hh:mm")+"):<br>";
                            break;
                }
                tip=tip+tt+QString::number((int)days)+" "+tr("jours")+" "+QString::number((int)hours)+" "+tr("heures")+" "+
                    QString::number((int)mins)+" "+tr("minutes");
                poi->setRouteTimeStamp(Eta);
                if(poi==this->my_poiList.last())
                    eta=Eta;
            }
            poi->setTip(tip);
//            lon=poi->getLongitude();
//            lat=poi->getLatitude();
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
                previousEta=Eta;
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
    setHidePois(this->hidePois);
    if(this->detectCoasts && !optimizingPOI)
    {
        line->setCoastDetection(true);
        line->setMap(parent->get_gshhsReader());
    }
    else
        line->setCoastDetection(false);
    this->slot_shShow();
    line->slot_showMe();
    busy=false;
//    qWarning()<<"VBVMG-VLM calculation time:"<<timeD;
//    qWarning()<<"Route total calculation time:"<<timeTotal.elapsed();
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
double ROUTE::A180(double angle)
{
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    return angle;
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

void ROUTE::slot_edit()
{
    emit editMe(this);
}

void ROUTE::slot_shShow()
{
    bool toBeHidden=this->hidden || (Settings::getSetting("autoHideRoute",1).toInt()==1 && (this->myBoat==NULL || !this->myBoat->getIsSelected()));
    if(toBeHidden)
    {
        slot_shHidden();
        return;
    }
    if(line)
    {
        line->show();
    }
}

void ROUTE::slot_shHidden()
{
    bool toBeHidden=this->hidden || (Settings::getSetting("autoHideRoute",1).toInt()==1 && (this->myBoat==NULL || !this->myBoat->getIsSelected()));
    if(line)
        line->hide();
    if(toBeHidden)
        setHidePois(hidePois);
}
void ROUTE::slot_boatPointerHasChanged(boat * acc)
{
    if(acc->getType()==BOAT_VLM)
    {
        if(((boatVLM*)acc)->getBoatPseudo()==this->boatLogin)
            this->setBoat(acc);
    }
    else
    {
        if(myBoat->getType()!=BOAT_VLM)
            this->setBoat(acc);
    }
}
void ROUTE::setHidePois(bool b)
{
    this->hidePois=b;
    QListIterator<POI*> i (my_poiList);
    bool toBeHidden=this->hidden || (Settings::getSetting("autoHideRoute",1).toInt()==1 && (this->myBoat==NULL || !this->myBoat->getIsSelected()));
    while(i.hasNext())
    {
        POI * poi=i.next();
        if ((poi==my_poiList.first() || poi==my_poiList.last()) && !toBeHidden)
            poi->setMyLabelHidden(false);
        else
        {
            if(!toBeHidden)
                poi->setMyLabelHidden(this->hidePois);
            else
                poi->setMyLabelHidden(true);
        }
    }
}
void ROUTE::do_vbvmg_context(double dist,double wanted_heading,
                             double w_speed,double w_angle,
                             double *heading1, double *heading2,
                             double *wangle1, double *wangle2,
                             double *time1, double *time2,
                             double *dist1, double *dist2) {
   double alpha, beta;
   double speed, speed_t1, speed_t2, l1, l2, d1, d2;
   double angle, maxangle, t, t1, t2, t_min;
   //double wanted_heading;
   //double w_speed, w_angle;
   double tanalpha, d1hypotratio;
   double b_alpha, b_beta, b_t1, b_t2, b_l1, b_l2;
   //double b1_alpha, b1_beta;
   double speed_alpha, speed_beta;
   double vmg_alpha, vmg_beta;
   wanted_heading=degToRad(wanted_heading);
   w_angle=degToRad(w_angle);
   int i,j, min_i, min_j, max_i, max_j;

   b_t1 = b_t2 = b_l1 = b_l2 = b_alpha = b_beta = beta = 0.0;

   maxangle = wanted_heading;


   /* first compute the time for the "ortho" heading */
   speed=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(w_angle-wanted_heading)));
   //speed = find_speed(aboat, w_speed, w_angle - wanted_heading);
   if (speed > 0.0) {
     t_min = dist / speed;
   } else {
     t_min = 365.0*24.0; /* one year :) */
   }

 #if DEBUG
   printf("VBVMG: Wind %.2fkts %.2f\n", w_speed, radToDeg(w_angle));
   printf("VBVMG Direct road: heading %.2f time %.2f\n",
          radToDeg(wanted_heading), t_min);
   printf("VBVMG Direct road: wind angle %.2f\n",
          radToDeg(w_angle-wanted_heading));
 #endif /* DEBUG */

   angle = w_angle - wanted_heading;
   if (angle < -PI ) {
     angle += TWO_PI;
   } else if (angle > PI) {
     angle -= TWO_PI;
   }
   if (angle < 0.0) {
     min_i = 1;
     min_j = -89;
     max_i = 90;
     max_j = 0;
   } else {
     min_i = -89;
     min_j = 1;
     max_i = 0;
     max_j = 90;
   }

   for (i=min_i; i<max_i; i++) {
     alpha = degToRad((double)i);
     tanalpha = tan(alpha);
     d1hypotratio = hypot(1, tan(alpha));
     speed_t1=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-alpha)));
     //speed_t1 = find_speed(aboat, w_speed, angle-alpha);
     if (speed_t1 <= 0.0) {
       continue;
     }
     for (j=min_j; j<max_j; j++) {
       beta = degToRad((double)j);
       d1 = dist * (tan(-beta) / (tanalpha + tan(-beta)));
       l1 =  d1 * d1hypotratio;
       t1 = l1 / speed_t1;
       if ((t1 < 0.0) || (t1 > t_min)) {
         continue;
       }
       d2 = dist - d1;
       speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
       //speed_t2 = find_speed(aboat, w_speed, angle-beta);
       if (speed_t2 <= 0.0) {
         continue;
       }
       l2 =  d2 * hypot(1, tan(-beta));
       t2 = l2 / speed_t2;
       if (t2 < 0.0) {
         continue;
       }
       t = t1 + t2;
       if (t < t_min) {
         t_min = t;
         b_alpha = alpha;
         b_beta  = beta;
         b_l1 = l1;
         b_l2 = l2;
         b_t1 = t1;
         b_t2 = t2;
       }
     }
   }
 #if DEBUG
   printf("VBVMG: alpha=%.2f, beta=%.2f\n", radToDeg(b_alpha), radToDeg(b_beta));
 #endif /* DEBUG */

#if 0 /*set 1 to get 0.1 precision*/
     b1_alpha = b_alpha;
     b1_beta = b_beta;
     for (i=-9; i<=9; i++) {
       alpha = b1_alpha + degToRad(((double)i)/10.0);
       tanalpha = tan(alpha);
       d1hypotratio = hypot(1, tan(alpha));
       speed_t1=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-alpha)));
       //speed_t1 = find_speed(aboat, w_speed, angle-alpha);
       if (speed_t1 <= 0.0) {
         continue;
       }
       for (j=-9; j<=9; j++) {
         beta = b1_beta + degToRad(((double)j)/10.0);
         d1 = dist * (tan(-beta) / (tanalpha + tan(-beta)));
         l1 =  d1 * d1hypotratio;
         t1 = l1 / speed_t1;
         if ((t1 < 0.0) || (t1 > t_min)) {
           continue;
         }
         d2 = dist - d1;
         speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
         //speed_t2 = find_speed(aboat, w_speed, angle-beta);
         if (speed_t2 <= 0) {
           continue;
         }
         l2 =  d2 * hypot(1, tan(-beta));
         t2 = l2 / speed_t2;
         if (t2 < 0.0) {
           continue;
         }
         t = t1 + t2;
         if (t < t_min) {
           t_min = t;
           b_alpha = alpha;
           b_beta  = beta;
           b_l1 = l1;
           b_l2 = l2;
           b_t1 = t1;
           b_t2 = t2;
         }
       }
     }
 #if DEBUG
     printf("VBVMG: alpha=%.2f, beta=%.2f\n", radToDeg(b_alpha),
            radToDeg(b_beta));
 #endif /* DEBUG */
#endif
   speed_alpha=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-b_alpha)));
   //speed_alpha = find_speed(aboat, w_speed, angle-b_alpha);
   vmg_alpha = speed_alpha * cos(b_alpha);
   speed_beta=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-b_beta)));
   //speed_beta = find_speed(aboat, w_speed, angle-b_beta);
   vmg_beta = speed_beta * cos(b_beta);

 #if DEBUG
     printf("VBVMG: speedalpha=%.2f, speedbeta=%.2f\n", speed_alpha, speed_beta);
     printf("VBVMG: vmgalpha=%.2f, vmgbeta=%.2f\n", vmg_alpha, vmg_beta);
     printf("VBVMG: headingalpha %.2f, headingbeta=%.2f\n",
            radToDeg(fmod(wanted_heading + b_alpha, TWO_PI)),
            radToDeg(fmod(wanted_heading + b_beta, TWO_PI)));
 #endif /* DEBUG */

   if (vmg_alpha > vmg_beta) {
     *heading1 = fmod(wanted_heading + b_alpha, TWO_PI);
     *heading2 = fmod(wanted_heading + b_beta, TWO_PI);
     *time1 = b_t1;
     *time2 = b_t2;
     *dist1 = b_l1;
     *dist2 = b_l2;
   } else {
     *heading2 = fmod(wanted_heading + b_alpha, TWO_PI);
     *heading1 = fmod(wanted_heading + b_beta, TWO_PI);
     *time2 = b_t1;
     *time1 = b_t2;
     *dist2 = b_l1;
     *dist1 = b_l2;
   }
   if (*heading1 < 0 ) {
     *heading1 += TWO_PI;
   }
   if (*heading2 < 0 ) {
     *heading2 += TWO_PI;
   }

   *wangle1 = fmod(*heading1 - w_angle, TWO_PI);
   if (*wangle1 > PI ) {
     *wangle1 -= TWO_PI;
   } else if (*wangle1 < -PI ) {
     *wangle1 += TWO_PI;
   }
   *wangle2 = fmod(*heading2 - w_angle, TWO_PI);
   if (*wangle2 > PI ) {
     *wangle2 -= TWO_PI;
   } else if (*wangle2 < -PI ) {
     *wangle2 += TWO_PI;
   }
 #if 0
   QString s;
   qWarning()<<s.sprintf("VBVMG: wangle1=%.2f, wangle2=%.2f\n", radToDeg(*wangle1),
          radToDeg(*wangle2));
   qWarning()<<s.sprintf("VBVMG: heading1 %.2f, heading2=%.2f\n", radToDeg(*heading1),
          radToDeg(*heading2));
   qWarning()<<s.sprintf("VBVMG: dist=%.2f, l1=%.2f, l2=%.2f, ratio=%.2f\n", dist, *dist1,
          *dist2, (b_l1+b_l2)/dist);
   qWarning()<<s.sprintf("VBVMG: t1 = %.2f, t2=%.2f, total=%.2f\n", *time1, *time2, t_min);
   qWarning()<<s.sprintf("VBVMG: heading %.2f\n", radToDeg(*heading1));
   qWarning()<<s.sprintf("VBVMG: wind angle %.2f\n", radToDeg(*wangle1));
 #endif /* DEBUG */
   *heading1=radToDeg(*heading1);
   *heading2=radToDeg(*heading2);
   *wangle1=radToDeg(*wangle1);
   *wangle2=radToDeg(*wangle2);
 }
void ROUTE::do_vbvmg_buffer(double dist,double wanted_heading,
                             double w_speed,double w_angle,
                             double *heading1, double *heading2,
                             double *wangle1, double *wangle2,
                             double *time1, double *time2,
                             double *dist1, double *dist2)
{
    double alpha, beta;
    double speed, speed_t1, speed_t2, l1, l2, d1, d2;
    double angle, maxangle, t, t1, t2, t_min;
    double tanalpha, d1hypotratio;
    double b_alpha, b_beta, b_t1, b_t2, b_l1, b_l2;
    double speed_alpha, speed_beta;
    double vmg_alpha, vmg_beta;
    wanted_heading=degToRad(wanted_heading);
    w_angle=degToRad(w_angle);
    int i,j, min_i, min_j, max_i, max_j;

    b_t1 = b_t2 = b_l1 = b_l2 = b_alpha = b_beta = beta = 0.0;

    maxangle = wanted_heading;
    QCache<int,double> fastSpeed;
    fastSpeed.setMaxCost(180);


    /* first compute the time for the "ortho" heading */
    speed=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(w_angle-wanted_heading)));
    //speed = find_speed(aboat, w_speed, w_angle - wanted_heading);
    if (speed > 0.0)
    {
        t_min = dist / speed;
    }
    else
    {
        t_min = 365.0*24.0; /* one year :) */
    }


    angle = w_angle - wanted_heading;
    if (angle < -PI )
    {
        angle += TWO_PI;
    }
    else if (angle > PI)
    {
        angle -= TWO_PI;
    }
    double guessAngle=A180(radToDeg(angle));
    if (angle < 0.0)
    {
        min_i = 1;
        min_j = -89;
        max_i = 90;
        max_j = 0;
    }
    else
    {
        min_i = -89;
        min_j = 1;
        max_i = 0;
        max_j = 90;
    }
    for (i=min_i; i<max_i; ++i)
    {
        alpha = degToRad((double)i);
        double guessTwa=A180(radToDeg(angle-alpha));
        if(newVbvmgVlm && (qAbs(guessTwa)<20 || qAbs(guessTwa>175))) continue;
        if(i>0)
        {
            tanalpha = tanPos->at(i);
            d1hypotratio = hypotPos->at(i);
        }
        else
        {
            tanalpha = tanNeg->at(-i);
            d1hypotratio = hypotNeg->at(-i);
        }
        speed_t1=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-alpha)));
        //speed_t1 = find_speed(aboat, w_speed, angle-alpha);
        if (speed_t1 <= 0.0)
        {
            continue;
        }
        int MinJ,MaxJ;
        if(!this->newVbvmgVlm)
        {
            MinJ=min_j;
            MaxJ=max_j;
        }
        else
        {
            int guessInt=qRound(guessAngle+guessTwa);
            MinJ=qMax(guessInt-5,min_j);
            MaxJ=qMin(guessInt+5,max_j);
        }
        for (j=MinJ; j<MaxJ; ++j)
        {
            beta = degToRad((double)j);
            if(-j>0)
            {
//                d1 = dist * (tan(-beta) / (tanalpha + tan(-beta)));
                d1 = dist * tanPos->at(-j) / (tanalpha + tanPos->at(-j));
            }
            else
                d1 = dist * tanNeg->at(j) / (tanalpha + tanNeg->at(j));
            l1 =  d1 * d1hypotratio;
            t1 = l1 / speed_t1;
            if ((t1 < 0.0) || (t1 > t_min))
            {
                continue;
            }
            d2 = dist - d1;
#if 1
            if(fastSpeed.contains(j))
                speed_t2=*fastSpeed.object(j);
            else
            {
                speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
                fastSpeed.insert(j,new double(speed_t2));
            }
#else
            speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
#endif
            //speed_t2 = find_speed(aboat, w_speed, angle-beta);
            if (speed_t2 <= 0.0)
            {
                continue;
            }
            if(-j>0)
            {
                //l2 =  d2 * hypot(1, tan(-beta));
                l2 = d2 * hypotPos->at(-j);
            }
            else
                l2 = d2 * hypotNeg->at(j);
            t2 = l2 / speed_t2;
            if (t2 < 0.0)
            {
                continue;
            }
            t = t1 + t2;
            if (t < t_min)
            {
                t_min = t;
                b_alpha = alpha;
                b_beta  = beta;
                b_l1 = l1;
                b_l2 = l2;
                b_t1 = t1;
                b_t2 = t2;
            }
        }
        if(newVbvmgVlm)
        {
            for (j=qMax(-i-5,min_j); j<qMin(-i+5,max_j); ++j)
            {
                beta = degToRad((double)j);
    //            if((angle-alpha>0 && angle-beta>0) ||
    //               (angle-alpha<0 && angle-beta<0)) continue;
                if(-j>0)
                {
    //                d1 = dist * (tan(-beta) / (tanalpha + tan(-beta)));
                    d1 = dist * tanPos->at(-j) / (tanalpha + tanPos->at(-j));
                }
                else
                    d1 = dist * tanNeg->at(j) / (tanalpha + tanNeg->at(j));
                l1 =  d1 * d1hypotratio;
                t1 = l1 / speed_t1;
                if ((t1 < 0.0) || (t1 > t_min))
                {
                    continue;
                }
                d2 = dist - d1;
#if 1
            if(fastSpeed.contains(j))
                speed_t2=*fastSpeed.object(j);
            else
            {
                speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
                fastSpeed.insert(j,new double(speed_t2));
            }
#else
            speed_t2=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-beta)));
#endif
                //speed_t2 = find_speed(aboat, w_speed, angle-beta);
                if (speed_t2 <= 0.0)
                {
                    continue;
                }
                if(-j>0)
                {
                    //l2 =  d2 * hypot(1, tan(-beta));
                    l2 = d2 * hypotPos->at(-j);
                }
                else
                    l2 = d2 * hypotNeg->at(j);
                t2 = l2 / speed_t2;
                if (t2 < 0.0)
                {
                    continue;
                }
                t = t1 + t2;
                if (t < t_min)
                {
                    t_min = t;
                    b_alpha = alpha;
                    b_beta  = beta;
                    b_l1 = l1;
                    b_l2 = l2;
                    b_t1 = t1;
                    b_t2 = t2;
                }
            }
        }
    }
    speed_alpha=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-b_alpha)));
    //speed_alpha = find_speed(aboat, w_speed, angle-b_alpha);
    vmg_alpha = speed_alpha * cos(b_alpha);
    speed_beta=myBoat->getPolarData()->getSpeed(w_speed,A180(radToDeg(angle-b_beta)));
    //speed_beta = find_speed(aboat, w_speed, angle-b_beta);
    vmg_beta = speed_beta * cos(b_beta);

    if (vmg_alpha > vmg_beta)
    {
        *heading1 = fmod(wanted_heading + b_alpha, TWO_PI);
        *heading2 = fmod(wanted_heading + b_beta, TWO_PI);
        *time1 = b_t1;
        *time2 = b_t2;
        *dist1 = b_l1;
        *dist2 = b_l2;
    }
    else
    {
        *heading2 = fmod(wanted_heading + b_alpha, TWO_PI);
        *heading1 = fmod(wanted_heading + b_beta, TWO_PI);
        *time2 = b_t1;
        *time1 = b_t2;
        *dist2 = b_l1;
        *dist1 = b_l2;
    }
    if (*heading1 < 0 )
    {
        *heading1 += TWO_PI;
    }
    if (*heading2 < 0 )
    {
        *heading2 += TWO_PI;
    }

    *wangle1 = fmod(*heading1 - w_angle, TWO_PI);
    if (*wangle1 > PI )
    {
        *wangle1 -= TWO_PI;
    }
    else if (*wangle1 < -PI )
    {
        *wangle1 += TWO_PI;
    }
    *wangle2 = fmod(*heading2 - w_angle, TWO_PI);
    if (*wangle2 > PI )
    {
        *wangle2 -= TWO_PI;
    } else if (*wangle2 < -PI )
    {
        *wangle2 += TWO_PI;
    }
    fastSpeed.clear();
    *heading1=radToDeg(*heading1);
    *heading2=radToDeg(*heading2);
    *wangle1=radToDeg(*wangle1);
    *wangle2=radToDeg(*wangle2);
}
void ROUTE::precalculateTan()
{
    tanPos=new QList<double>;
    tanNeg=new QList<double>;
    hypotPos=new QList<double>;
    hypotNeg=new QList<double>;
    tanPos->append(0);
    tanNeg->append(0);
    hypotPos->append(0);
    hypotNeg->append(0);
    for (double i=1;i<90;++i)
    {
        double tanG=tan(degToRad(i));
        tanPos->append(tanG);
        hypotPos->append(hypot(1,tanG));
        tanG=tan(degToRad(-i));
        tanNeg->append(tanG);
        hypotNeg->append(hypot(1,tanG));
    }
}
