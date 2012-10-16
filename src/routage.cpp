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

Original code: virtual-winds.com
***********************************************************************/
#include <cassert>
#include <QDateTime>
#include <QMessageBox>


#include "Orthodromie.h"
#include "Projection.h"
#include "routage.h"
#include "Grib.h"
#include "mycentralwidget.h"
#include "vlmLine.h"
#include "POI.h"
#include "DialogRoutage.h"
#include "boat.h"
#include "Polar.h"
#include "Point.h"
#include "Segment.h"
#include "Triangle.h"
#include "Util.h"
#include "Polygon.h"
#include "route.h"
#include <QDebug>
#include <QtConcurrentMap>
#include "vlmpointgraphic.h"
#include "settings.h"
#include "Terrain.h"
//#include "Terrain.h"
//#define debugCount
//#define traceTime;
inline vlmPoint findPointThreaded(const vlmPoint &point)
{
    vlmPoint pt=point;
    double cap=pt.capOrigin;
    double windAngle=pt.wind_angle;
    double windSpeed=pt.wind_speed;
    double lat=pt.origin->lat;
    double lon=pt.origin->lon;
    cap=Util::A360(cap);
    double angle,newSpeed;
    time_t workEta;
    double res_lon,res_lat;
    double distanceParcourue=0;
    double current_speed=pt.current_speed;
    double current_angle=pt.current_angle;
    for(int a=0;a<=1;++a)
    {
        angle=cap-(double)windAngle;
        if(qAbs(angle)>180)
        {
            if(angle<0)
                angle=360+angle;
            else
                angle=angle-360;
        }
        double limit=pt.routage->getBoat()->getPolarData()->getBvmgUp(windSpeed);
        if(qAbs(angle)<limit && angle!=90) //if too close to wind then use VB-VMG technique
        {
            newSpeed=pt.routage->getBoat()->getPolarData()->getSpeed(windSpeed,limit);
            newSpeed=newSpeed*qAbs(cos(degToRad(limit))/cos(degToRad(qAbs(angle))));
        }
        else
        {
            limit=pt.routage->getBoat()->getPolarData()->getBvmgDown(windSpeed);
            if(qAbs(angle)>limit && angle!=90)
            {
                newSpeed=pt.routage->getBoat()->getPolarData()->getSpeed(windSpeed,limit);
                newSpeed=newSpeed*qAbs(cos(degToRad(limit))/cos(degToRad(qAbs(angle))));
            }
            else
                newSpeed=pt.routage->getBoat()->getPolarData()->getSpeed(windSpeed,angle);
        }
        if(current_speed>0)
        {
            QPointF p=Util::calculateSumVect(cap,newSpeed,Util::A360(current_angle+180.0),current_speed);
            newSpeed=p.x(); //in this case newSpeed is SOG
            cap=p.y(); //in this case cap is COG
        }
        distanceParcourue=newSpeed*pt.routage->getTimeStep()/60.0;
        Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap, &res_lat, &res_lon);
        pt.lon=res_lon;
        pt.lat=res_lat;
        pt.distOrigin=distanceParcourue;
        pt.capOrigin=cap;
        //if(!pt.routage->getUseRouteModule()) break;
        if(a==0)
        {
            double newWindAngle,newWindSpeed;
            if(!pt.routage->getWindIsForced())
            {
                workEta=pt.eta;
                if(pt.routage->getWhatIfUsed() && pt.routage->getWhatIfJour()<=pt.eta)
                    workEta=workEta+pt.routage->getWhatIfTime()*3600;
                if(!pt.routage->getGrib()->getInterpolatedValue_byDates(res_lon,res_lat,
                       workEta+pt.routage->getTimeStep()*60,&newWindSpeed,&newWindAngle,INTERPOLATION_DEFAULT)||workEta+pt.routage->getTimeStep()*60>pt.routage->getGrib()->getMaxDate())
                {
                    pt.isDead=true;
                    return pt;
                }
                newWindAngle=radToDeg(newWindAngle);
                if(pt.routage->getGrib()->getInterpolatedValueCurrent_byDates(res_lon,res_lat,
                       workEta+pt.routage->getTimeStep()*60,&current_speed,&current_angle,INTERPOLATION_DEFAULT))
                {
                    current_angle=radToDeg(current_angle);
                    QPointF p=Util::calculateSumVect(newWindAngle,newWindSpeed,current_angle,current_speed);
                    newWindSpeed=p.x();
                    newWindAngle=p.y();
                }
                else
                {
                    current_speed=-1;
                    current_angle=0;
                }
            }
            else
            {
                newWindAngle=pt.routage->getWindAngle();
                newWindSpeed=pt.routage->getWindSpeed();
            }
            if(pt.routage->getI_iso())
            {
                newWindAngle=Util::A360(newWindAngle+180.0);
                current_angle=Util::A360(current_angle+180.0);
            }
            if(pt.routage->getWhatIfUsed() && pt.routage->getWhatIfJour()<=pt.eta)
                windSpeed=windSpeed*pt.routage->getWhatIfWind()/100.00;
            windAngle=Util::A360((windAngle+newWindAngle)/2.0);
            windSpeed=(windSpeed+newWindSpeed)/2.0;
            if(current_speed!=-1 && pt.current_speed!=-1)
            {
                current_speed=(pt.current_speed+current_speed)/2.0;
                current_angle=Util::A360((current_angle+pt.current_angle)/2.0);
            }
        }
    }
    if(qAbs(pt.lat)>84.0)
    {
        pt.isDead=true;
        return pt;
    }
    double x,y;
    pt.routage->getProj()->map2screenDouble(Util::cLFA(pt.lon,pt.routage->getProj()->getXmin()),pt.lat,&x,&y);
    pt.x=x;
    pt.y=y;
    if(pt.routage->getVisibleOnly() && !pt.routage->getProj()->isInBounderies_strict(pt.x,pt.y))
    {
        pt.isDead=true;
        return pt;
    }
    if(pt.xM1<10e4)
    {
        Triangle t(Point(pt.origin->x,pt.origin->y),
                   Point(pt.x,pt.y),
                   Point(pt.xM1,pt.yM1));
        if (t.orientation()==right_turn)
        {
            pt.isDead=true;
            return pt;
        }
    }
    if(pt.xP1<10e4)
    {
        Triangle t(Point(pt.origin->x,pt.origin->y),
                   Point(pt.x,pt.y),
                   Point(pt.xP1,pt.yP1));
        if (t.orientation()==left_turn)
        {
            pt.isDead=true;
            return pt;
        }
    }
    if(pt.origin->isStart)
        pt.distIso=pt.distStart;
    else
    {
        pt.distIso=ROUTAGE::findDistancePreviousIso(pt, pt.routage->getPreviousIso());
    }


    bool bad=false;
    QPolygonF * previousIso=pt.routage->getPreviousIso();
    QList<QLineF> * previousSegments=pt.routage->getPreviousSegments();
    if(!pt.origin->isStart && previousIso->count()>1)
    {
        QLineF temp1(pt.origin->x,pt.origin->y,pt.x,pt.y);
        QPointF dummy(0,0);
        for (int i=0;i<previousIso->count()-1;++i)
        {
            QLineF s(previousIso->at(i),previousIso->at(i+1));
            if(pt.originNb!=i && pt.originNb!=i+1)
            {
                if(temp1.intersect(s,&dummy)==QLineF::BoundedIntersection)
                {
                    bad=true;
                    break;
                }
                if(temp1.intersect(previousSegments->at(i),&dummy)==QLineF::BoundedIntersection)
                {
                    bad=true;
                    break;
                }
            }
        }
    }
    if (bad)
    {
        pt.isDead=true;
        return pt;
    }
    pt.convertionLat=pt.lat;
    pt.convertionLon=pt.lon;
    return pt;
}

bool rightToLeftFromOrigin(const vlmPoint & P1,const vlmPoint & P2)
{
    Triangle triangle(Point(P1.origin->x,P1.origin->y),
                      Point(P1.x,P1.y),
                      Point(P2.x,P2.y));
    return triangle.orientation()==right_turn;
}

/*threadable functions*/

inline vlmPoint checkCoastCollision(const vlmPoint &point)
{
    vlmPoint newPoint=point;
    double x1,y1,x2,y2;
    x1=newPoint.origin->x;
    y1=newPoint.origin->y;
    x2=newPoint.x;
    y2=newPoint.y;
    newPoint.isDead=(newPoint.routage->getCheckCoast() && (newPoint.routage->getMap() && newPoint.routage->getMap()->crossing(QLineF(x1,y1,x2,y2),QLineF(newPoint.origin->lon,newPoint.origin->lat,newPoint.lon,newPoint.lat))))
                 || (newPoint.routage->getCheckLine() && newPoint.routage->crossBarriere(QLineF(x1,y1,x2,y2)));
    return newPoint;
}
inline QList<vlmPoint> finalEpuration(const QList<vlmPoint> &listPoints)
{
    if(listPoints.count()==0) return listPoints;
    if(listPoints.first().origin->isStart) return listPoints;
    int toBeRemoved=listPoints.at(0).internal_1;
    double initialDist=listPoints.at(0).internal_2;
    if(toBeRemoved<=0) return listPoints;
    QMultiMap<double,QPoint> byCriteres;
    QHash<quint32,double> byIndices;
    QList<bool> deadStatus;
    quint32 s;
    double critere=0;
    for(int n=0;n<listPoints.size()-1;++n)
    {
        if(qAbs(Util::myDiffAngle(listPoints.at(n).capArrival,
                                listPoints.at(n+1).capArrival)) > 90)
            critere=179;
        else
        {
            critere=0;
            QLineF temp1;
            QPointF middle;
            if(listPoints.at(n).originNb!=listPoints.at(n+1).originNb)
            {
                temp1.setPoints(QPointF(listPoints.at(n).origin->x,listPoints.at(n).origin->y),
                         QPointF(listPoints.at(n+1).origin->x,listPoints.at(n+1).origin->y));
                middle=temp1.pointAt(0.5);
            }
            else
                middle=QPointF(listPoints.at(n).origin->x,listPoints.at(n).origin->y);
            temp1.setPoints(QPointF(listPoints.at(n).x,listPoints.at(n).y),
                            QPointF(listPoints.at(n+1).x,listPoints.at(n+1).y));
            QPointF middleBis=temp1.pointAt(0.5);
            temp1.setPoints(middleBis,middle);
            temp1.setLength(initialDist);
            middle=temp1.p2();
            QLineF temp2(middle.x(),middle.y(),listPoints.at(n).x,listPoints.at(n).y);
            QLineF temp3(middle.x(),middle.y(),listPoints.at(n+1).x,listPoints.at(n+1).y);
            critere=qAbs(temp2.angleTo(temp3));
            if(critere>180)
            {
                critere=360-critere;
            }
            if(listPoints.at(n).notSimplificable ||
               listPoints.at(n+1).notSimplificable ||
               listPoints.at(n).origin->notSimplificable ||
               listPoints.at(n+1).origin->notSimplificable)
                    critere=critere*2.0;
        }
        byCriteres.insert(critere,QPoint(n,n+1));
        s=n*10e6+n+1;
        byIndices.insert(s,critere);
        deadStatus.append(false);
    }
    deadStatus.append(false);
    QMutableMapIterator<double,QPoint> d(byCriteres);
    int currentCount=listPoints.count();
    while(toBeRemoved>0 && currentCount>=0)
    {
        d.toFront();
        if(!d.hasNext()) break;
        QPoint couple=d.next().value();
        int badOne=0;
        if(!listPoints.at(couple.x()).notSimplificable &&
           listPoints.at(couple.y()).notSimplificable)
            badOne=couple.x();
        else if(listPoints.at(couple.x()).notSimplificable &&
           !listPoints.at(couple.y()).notSimplificable)
            badOne=couple.y();
        else if(listPoints.at(couple.x()).origin->notSimplificable &&
           !listPoints.at(couple.y()).origin->notSimplificable)
            badOne=couple.y();
        else if(!listPoints.at(couple.x()).origin->notSimplificable &&
           listPoints.at(couple.y()).origin->notSimplificable)
            badOne=couple.x();
        else if(listPoints.at(couple.x()).distIso<listPoints.at(couple.y()).distIso)
            badOne=couple.x();
        else
            badOne=couple.y();
        deadStatus.replace(badOne,true);
        int previous=-1;
        int next=-1;
        if(badOne>0)
            previous=deadStatus.lastIndexOf(false,badOne);
        if(badOne<listPoints.count()-1)
            next=deadStatus.indexOf(false,badOne);
        if(currentCount<=1) break;
        if(previous!=-1 && next!=-1)
        {
            s=previous*10e6+badOne;
            double criterePrevious=byIndices.value(s);
            s=badOne*10e6+next;
            double critereNext=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
            byCriteres.remove(critereNext,QPoint(badOne,next));
            QLineF temp1;
            QPointF middle;
            if(listPoints.at(previous).originNb!=listPoints.at(next).originNb)
            {
                temp1.setPoints(QPointF(listPoints.at(previous).origin->x,listPoints.at(previous).origin->y),
                             QPointF(listPoints.at(next).origin->x,listPoints.at(next).origin->y));
                middle=temp1.pointAt(0.5);
            }
            else
                middle=QPointF(listPoints.at(previous).origin->x,listPoints.at(previous).origin->y);
            temp1.setPoints(QPointF(listPoints.at(previous).x,listPoints.at(previous).y),
                            QPointF(listPoints.at(next).x,listPoints.at(next).y));
            QPointF middleBis=temp1.pointAt(0.5);
            temp1.setPoints(middleBis,middle);
            temp1.setLength(initialDist);
            middle=temp1.p2();
            QLineF temp2(middle.x(),middle.y(),listPoints.at(previous).x,listPoints.at(previous).y);
            QLineF temp3(middle.x(),middle.y(),listPoints.at(next).x,listPoints.at(next).y);
            double critere=qAbs(temp2.angleTo(temp3));
            if(critere>180)
            {
                critere=360-critere;
            }
            byCriteres.insert(critere,QPoint(previous,next));
            s=previous*10e6+next;
            byIndices.insert(s,critere);
        }
        else if(previous==-1)
        {
            s=badOne*10e6+next;
            double critereNext=byIndices.value(s);
            byCriteres.remove(critereNext,QPoint(badOne,next));
        }
        else if(next==-1)
        {
            s=previous*10e6+badOne;
            double criterePrevious=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
        }
        --toBeRemoved;
        --currentCount;
    }
    QList<vlmPoint> result;
    for (int nn=0;nn<deadStatus.count();++nn)
    {
        if(!deadStatus.at(nn))
            result.append(listPoints.at(nn));
    }
    return result;
}
inline QList<vlmPoint> findRoute(const QList<vlmPoint> & pointList)
{
    if(pointList.isEmpty()) return pointList;
    ROUTAGE * routage=pointList.at(0).routage;
    datathread dataThread;
    dataThread.Boat=routage->getBoat();
    if(routage->getI_iso())
        dataThread.Eta=routage->getI_eta();
    else
        dataThread.Eta=routage->getEta();
    dataThread.Eta=pointList.first().origin->eta;
    dataThread.GriB=routage->getGrib();
    dataThread.whatIfJour=routage->getWhatIfJour();
    dataThread.whatIfUsed=routage->getWhatIfUsed();
    dataThread.windIsForced=routage->getWindIsForced();
    dataThread.whatIfTime=routage->getWhatIfTime();
    dataThread.windAngle=routage->getWindAngle();
    dataThread.windSpeed=routage->getWindSpeed();
    dataThread.whatIfWind=routage->getWhatIfWind();
    dataThread.timeStep=routage->getTimeStep();
    dataThread.speedLossOnTack=routage->getSpeedLossOnTack();
    dataThread.i_iso=routage->getI_iso();
    QList<vlmPoint> resultList;
    for (int pp=0;pp<pointList.count();++pp)
    {
        vlmPoint point=pointList.at(pp);
        double cap=point.capOrigin;
        double lon=point.origin->lon;
        double lat=point.origin->lat;
        vlmPoint resultP=point;
        double res_lon=point.lon;
        double res_lat=point.lat;
        double distanceParcourue=point.distOrigin;
        vlmPoint from(lon,lat);
        from.wind_angle=point.origin->wind_angle;
        from.isStart=point.origin->isStart;
        vlmPoint to(res_lon,res_lat);
        double lastLonFound,lastLatFound;
        int realTime=ROUTAGE::calculateTimeRoute(from, to, &dataThread, &lastLonFound, &lastLatFound);
        if(realTime>10e4)
        {
            resultP.isDead=true;
            //resultList.append(resultP);
            continue;
        }
        bool found=false;
        int timeStepSec=point.routage->getTimeStep()*60.0;
        if(realTime==timeStepSec)
        {
            found=true;
        }
        if(!found)
        {
            int oldTime=timeStepSec;
            /*first, trying to be clever*/
            distanceParcourue=distanceParcourue*oldTime/realTime;
            Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap, &res_lat, &res_lon);
            to.lon=res_lon;
            to.lat=res_lat;
            realTime=ROUTAGE::calculateTimeRoute(from, to, &dataThread, &lastLonFound, &lastLatFound);
            if(realTime>10e4)
            {
                resultP.isDead=true;
                //resultList.append(resultP);
                continue;
            }
            if(realTime==timeStepSec)
            {
                found=true;
            }
            if(!found)
            {
                /*find it using Newtown-Raphson method*/
                double x=distanceParcourue;
                double term=0;
                from.capOrigin=cap;
                for (int n=2;n<=21;++n) /*20 tries max*/
                {
                    double y=ROUTAGE::routeFunction(x,from,&lastLonFound,&lastLatFound,&dataThread)-timeStepSec;
                    if(y>10e4)
                        break;
                    if(qAbs(qRound(y))==0)
                    {
                        found=true;
                        double res_lon,res_lat;
                        Util::getCoordFromDistanceAngle(from.lat, from.lon, x, from.capOrigin, &res_lat, &res_lon);
                        to=vlmPoint(res_lon,res_lat);
                        resultP.foundByNewtonRaphson=true;
                        break;
                    }
                    double deriv=ROUTAGE::routeFunctionDeriv(x,from,&lastLonFound,&lastLatFound,&dataThread);
                    if (deriv==0)
                    {
                        break; /*flat spot, there is no solution*/
                    }
                    term=y/deriv;
                    x=qAbs(x-term);
                }
            }
        }
        if(found)
        {
            resultP.convertionLat=to.lat;
            resultP.convertionLon=to.lon;
            resultP.lon=lastLonFound;
            resultP.lat=lastLatFound;
            Orthodromie oo(from.lon,from.lat,resultP.lon,resultP.lat);
            resultP.distOrigin=oo.getDistance();
            resultP.capOrigin=oo.getAzimutDeg();
            resultList.append(resultP);
        }
    }
    return resultList;
}
inline int ROUTAGE::routeFunction(const double &x,const vlmPoint &from, double * lastLonFound, double * lastLatFound, const datathread * dataThread)
{
    double res_lon,res_lat;
    Util::getCoordFromDistanceAngle(from.lat, from.lon, x, from.capOrigin, &res_lat, &res_lon);
    vlmPoint to(res_lon,res_lat);
    return ROUTAGE::calculateTimeRoute(from,to,dataThread,lastLonFound,lastLatFound);
}
inline int ROUTAGE::routeFunctionDeriv(const double &x,const vlmPoint &from, double * lastLonFound, double * lastLatFound, const datathread *dataThread)
{
    double minGap=x/100;
    int   yr,yl;
    double xl,xr;
    for(int n=1;n<200;++n) /*bad trick to avoid flat spots*/
    {
        xr=x+minGap;
        xl=qMax(x/100,x-minGap);
        yr=routeFunction(xr,from,lastLonFound,lastLatFound,dataThread);
        yl=routeFunction(xl,from,lastLonFound,lastLatFound,dataThread);
        if(yr!=yl) break;
        minGap=minGap+x/100;
    }
    return((yr-yl)/(xr-xl));
}
inline int ROUTAGE::calculateTimeRoute(const vlmPoint &routeFrom,const vlmPoint &routeTo, const datathread * dataThread, double * lastLonFound, double * lastLatFound, const int &limit)
{
    double  lastTwa=routeFrom.wind_angle;
    bool    ignoreTackLoss=routeFrom.isStart;
    time_t etaRoute=dataThread->Eta;
    time_t etaLimit=etaRoute*2;
    time_t workEta;
    bool hasLimit=false;
    if(limit!=-1)
    {
        hasLimit=true;
        etaLimit=etaRoute+(limit*2);
    }
    bool has_eta=true;
    Orthodromie orth(0,0,0,0);
    //Orthodromie orth2(0,0,0,0);
    double lon,lat;
    lon=routeFrom.lon;
    lat=routeFrom.lat;
    int vacLen=dataThread->Boat->getVacLen();
    double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,cap1,cap2,diff1,diff2;
    double windAngle,windSpeed,cap,angle;
    time_t maxDate=dataThread->GriB->getMaxDate();
    newSpeed=0;
    distanceParcourue=0;
    res_lon=0;
    res_lat=0;
    windAngle=0;
    windSpeed=0;
    orth.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    //orth2.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    remaining_distance=orth.getDistance();
    //double initialDistance=orth2.getDistance();
    double current_speed=-1;
    double current_angle=0;
    do
    {
        workEta=etaRoute;
        if(dataThread->whatIfUsed && dataThread->whatIfJour<=dataThread->Eta)
            workEta=workEta+dataThread->whatIfTime*3600;
        if(dataThread->windIsForced || (dataThread->GriB->getInterpolatedValue_byDates(lon, lat, workEta,&windSpeed,&windAngle,INTERPOLATION_DEFAULT) && workEta<=maxDate && (!hasLimit || etaRoute<=etaLimit)))
        {
            if(dataThread->windIsForced)
            {
                windSpeed=dataThread->windSpeed;
                windAngle=dataThread->windAngle;
            }
            else
            {
                windAngle=radToDeg(windAngle);
                if (dataThread->GriB->getInterpolatedValueCurrent_byDates(lon, lat, workEta,&current_speed,&current_angle,INTERPOLATION_DEFAULT))
                {
                    current_angle=radToDeg(current_angle);
                    QPointF p=Util::calculateSumVect(windAngle,windSpeed,current_angle,current_speed);
                    windSpeed=p.x();
                    windAngle=p.y();
                }
                else
                {
                    current_speed=-1;
                    current_angle=0;
                }
            }
            if(dataThread->i_iso)
                windAngle=Util::A360(windAngle+180.0);
            if(dataThread->whatIfUsed && dataThread->whatIfJour<=dataThread->Eta)
                windSpeed=windSpeed*dataThread->whatIfWind/100.00;
            cap=orth.getAzimutDeg();
            angle=cap-windAngle;
            if(qAbs(angle)>180)
            {
                if(angle<0)
                    angle=360+angle;
                else
                    angle=angle-360;
            }
            if(qAbs(angle)<dataThread->Boat->getPolarData()->getBvmgUp(windSpeed))
            {
                angle=dataThread->Boat->getPolarData()->getBvmgUp(windSpeed);
                cap1=Util::A360(windAngle+angle);
                cap2=Util::A360(windAngle-angle);
                diff1=Util::myDiffAngle(cap,cap1);
                diff2=Util::myDiffAngle(cap,cap2);
                if(diff1<diff2)
                    cap=cap1;
                else
                    cap=cap2;
            }
            else if(qAbs(angle)>dataThread->Boat->getPolarData()->getBvmgDown(windSpeed))
            {
                angle=dataThread->Boat->getPolarData()->getBvmgDown(windSpeed);
                cap1=Util::A360(windAngle+angle);
                cap2=Util::A360(windAngle-angle);
                diff1=Util::myDiffAngle(cap,cap1);
                diff2=Util::myDiffAngle(cap,cap2);
                if(diff1<diff2)
                    cap=cap1;
                else
                    cap=cap2;
            }
            newSpeed=dataThread->Boat->getPolarData()->getSpeed(windSpeed,angle);
            if(current_speed>0)
            {
                QPointF p=Util::calculateSumVect(cap,newSpeed,Util::A360(current_angle+180.0),current_speed);
                newSpeed=p.x(); //in this case newSpeed is SOG
                cap=p.y(); //in this case cap is COG
            }

            if(!ignoreTackLoss && dataThread->speedLossOnTack!=1)
            {
                if((angle>0 && lastTwa<0) || (angle<0 && lastTwa>0))
                    newSpeed=newSpeed*dataThread->speedLossOnTack;
            }
            else
                ignoreTackLoss=false;
            lastTwa=angle;
            distanceParcourue=newSpeed*vacLen/3600.00;
            Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
            double p_remaining_distance=orth.getDistance();
            orth.setStartPoint(res_lon, res_lat);
            remaining_distance=orth.getDistance();
#if 1 /*note for self*/
            if (remaining_distance<distanceParcourue  ||
                p_remaining_distance<distanceParcourue)
            {
                lon=res_lon;
                lat=res_lat;
                if(dataThread->i_iso)
                    etaRoute-=vacLen;
                else
                    etaRoute+=vacLen;
                break;
            }
#endif
        }
        else
        {
            has_eta=false;
            break;
        }
        //orth2.setEndPoint(res_lon,res_lat);
        lon=res_lon;
        lat=res_lat;
        if(dataThread->i_iso)
            etaRoute-=vacLen;
        else
            etaRoute+=vacLen;
    } while (has_eta);
    if(!has_eta)
    {
        return 10e6;
    }
    if(lastLonFound!=NULL)
    {
        *lastLonFound=lon;
        *lastLatFound=lat;
    }
    return qAbs(etaRoute-dataThread->Eta);
}

ROUTAGE::ROUTAGE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow)
        : QObject()
{
    timerTempo=new QTimer(this);
    timerTempo->setSingleShot(true);
    timerTempo->setInterval(300);
    connect(timerTempo,SIGNAL(timeout()),this,SLOT(slot_calculate()));
    this->proj=proj;
    this->name=name;
    this->myscene=myScene;
    this->isPivot=false;
    fromRoutage=NULL;
    this->isNewPivot=false;
    if(QThread::idealThreadCount()<=1)
        this->useMultiThreading=false;
    else
        this->useMultiThreading=true;
    connect(myScene,SIGNAL(eraseWay()),this,SLOT(eraseWay()));
    this->grib=grib;
    this->parent=parentWindow;
    map=parent->get_gshhsReader();
    QList<QColor> colorsList;
    colorsList.append(Qt::yellow);
    colorsList.append(Qt::blue);
    colorsList.append(Qt::red);
    colorsList.append(Qt::green);
    colorsList.append(Qt::cyan);
    colorsList.append(Qt::magenta);
    colorsList.append(Qt::black);
    colorsList.append(Qt::darkRed);
    colorsList.append(Qt::darkGreen);
    colorsList.append(Qt::darkBlue);
    colorsList.append(Qt::darkCyan);
    colorsList.append(Qt::darkMagenta);
    colorsList.append(Qt::darkYellow);
    int ncolor=parent->getNbRoutage()-1;
    while (ncolor>=colorsList.count())
        ncolor=ncolor-colorsList.count();
    this->color=colorsList.at(ncolor);
    this->width=3;
    this->startTime= QDateTime().fromTime_t(parent->getNextVac()).toUTC();
    this->eta=startTime.toTime_t();
    this->whatIfDate=startTime;
    this->whatIfUsed=false;
    this->whatIfTime=0;
    this->whatIfWind=100;
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(2);
    this->angleRange=Settings::getSetting("angleRange",180).toDouble();
    this->angleStep=Settings::getSetting("angleStep",3).toDouble();
    this->timeStepLess24=Settings::getSetting("timeStepLess24",30).toDouble();
    this->timeStepMore24=Settings::getSetting("timeStepMore24",60).toDouble();
    this->explo=Settings::getSetting("exploNew",40).toDouble();
    this->useRouteModule=Settings::getSetting("useRouteModule",1).toInt()==1;
    this->useConverge=Settings::getSetting("useConverge",1).toInt()==1;
    this->checkCoast=Settings::getSetting("checkCoast",1).toInt()==1;
    this->checkLine=Settings::getSetting("checkLine",1).toInt()==1;
    this->thresholdAlternative=Settings::getSetting("thresholdAlternative",50).toInt();
    this->nbAlternative=Settings::getSetting("nbAlternative",0).toInt();
    this->visibleOnly=Settings::getSetting("visibleOnly",1).toInt()==1;
    this->autoZoom=Settings::getSetting("autoZoom",1).toInt()==1;
    this->zoomLevel=Settings::getSetting("autoZoomLevel",2).toInt();
    this->maxPres=Settings::getSetting("routageMaxPres",70).toDouble();
    this->maxPortant=Settings::getSetting("routageMaxPortant",70).toDouble();
    this->minPres=Settings::getSetting("routageMinPres",0).toDouble();
    this->minPortant=Settings::getSetting("routageMinPortant",0).toDouble();
    this->pruneWakeAngle=Settings::getSetting("routagePruneWake",30).toInt();
    this->colorGrib=Settings::getSetting("routageColorGrib",0).toInt()==1;
    this->showIso=Settings::getSetting("routageShowIso",1).toInt()==1;
    this->wind_angle=0;
    this->wind_speed=20;
    this->windIsForced=false;
    this->done=false;
    this->i_done=false;
    this->i_iso=false;
    this->converted=false;
    this->finalEta=QDateTime();
    this->finalEta.setTimeSpec(Qt::UTC);
    result=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+0.1);
    result->setParent(this);
    way=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+0.1);
    way->setParent(this);
    this->routeFromBoat=true;
    this->aborted=false;
    createPopupMenu();
    connect(parent,SIGNAL(stopCompassLine()),this,SLOT(slot_abort()));
    connect(parent,SIGNAL(updateRoutage()),this,SLOT(slot_gribDateChanged()));
    this->tempPoints.reserve(180*180);
    this->poiPrefix="R";
    this->speedLossOnTack=1;
    highlightedIso=0;
    isoRouteValue=10e6;
}
ROUTAGE::~ROUTAGE()
{
    if(parent->getTerre()->getRoutageGrib()==this)
    {
        parent->getTerre()->setRoutageGrib(NULL);
    }
    while(!isochrones.isEmpty())
        delete isochrones.takeFirst();
    while(!segments.isEmpty())
        delete segments.takeFirst();
    while(!i_isochrones.isEmpty())
        delete i_isochrones.takeFirst();
    while(!i_segments.isEmpty())
        delete i_segments.takeFirst();
    if(result!=NULL)
        delete result;
    while(!isoPointList.isEmpty())
        delete isoPointList.takeFirst();
    delete way;
    if(this->popup && !parent->getAboutToQuit())
        delete popup;
    while(!isoRoutes.isEmpty())
        delete isoRoutes.takeFirst();
    deleteAlternative();
}
void ROUTAGE::setBoat(boat *myBoat)
{
    this->myBoat=myBoat;
}
void ROUTAGE::setWidth(const double &width)
{
    this->width=width;
    if(result)
    {
        QPen penResult;
        penResult.setColor(color);
        penResult.setBrush(color);
        penResult.setWidthF(this->width);
        result->setLinePen(penResult);
    }
}
void ROUTAGE::setColor(const QColor &color)
{
    this->color=color;
    if(result)
    {
        QPen penResult;
        penResult.setColor(color);
        penResult.setBrush(color);
        penResult.setWidthF(this->width);
        result->setLinePen(penResult);
    }
}
void ROUTAGE::calculate()
{
    if(!i_iso)
    {
        Settings::setSetting("angleRange",this->angleRange);
        Settings::setSetting("angleStep",this->angleStep);
        Settings::setSetting("timeStepLess24",this->timeStepLess24);
        Settings::setSetting("timeStepMore24",this->timeStepMore24);
        Settings::setSetting("exploNew",this->explo);
        Settings::setSetting("useRouteModule",useRouteModule?1:0);
        Settings::setSetting("useConverge",useConverge?1:0);
        Settings::setSetting("checkCoast",checkCoast?1:0);
        Settings::setSetting("checkLine",checkLine?1:0);
        Settings::setSetting("thresholdAlternative",thresholdAlternative);
        Settings::setSetting("nbAlternative",nbAlternative);
        Settings::setSetting("visibleOnly",visibleOnly?1:0);
        Settings::setSetting("autoZoom",autoZoom?1:0);
        Settings::setSetting("autoZoomLevel",zoomLevel);
        Settings::setSetting("routageMaxPres",maxPres);
        Settings::setSetting("routageMaxPortant",maxPortant);
        Settings::setSetting("routageMinPres",minPres);
        Settings::setSetting("routageMinPortant",minPortant);
        Settings::setSetting("routagePruneWake",pruneWakeAngle);
        Settings::setSetting("routageColorGrib",colorGrib?1:0);
        Settings::setSetting("routageShowIso",showIso?1:0);
    }
    this->isNewPivot=false;
    this->aborted=false;
    if (!(myBoat && myBoat->getPolarData() && myBoat!=NULL))
    {
        QMessageBox::critical(0,tr("Routage"),tr("Pas de polaire chargee"));
        return;
    }
    if(!grib)
    {
        QMessageBox::critical(0,tr("Routage"),tr("Pas de grib charge"));
        return;
    }
    if(!i_iso)
    {
        if(!isPivot)
            eta=startTime.toUTC().toTime_t()+myBoat->getVacLen();
        else
            eta=startTime.toUTC().toTime_t();
    }
    else
        i_eta=eta;
    if ( eta>grib->getMaxDate() || eta<grib->getMinDate() )
    {
        QMessageBox::critical(0,tr("Routage"),tr("Date de depart choisie incoherente avec le grib"));
        return;
    }
    running=true;
    if(routeFromBoat)
    {
        start.setX(myBoat->getLon());
        start.setY(myBoat->getLat());
    }
    else
    {
        if(isPivot)
        {
            start.setX(pivotPoint.lon);
            start.setY(pivotPoint.lat);
        }
        else
        {
            start.setX(fromPOI->getLongitude());
            start.setY(fromPOI->getLatitude());
        }
    }
    arrival.setX(toPOI->getLongitude());
    arrival.setY(toPOI->getLatitude());
    if(i_iso)
    {
        arrival=start;
        vlmPoint i_start;
        if(arrived)
            i_start=result->getPoints()->at(1);
        else
            i_start=result->getPoints()->first();
        i_eta=i_start.eta;
        start.setX(i_start.lon);
        start.setY(i_start.lat);
        //qWarning()<<QDateTime().fromTime_t(i_eta).toUTC().toString("dd MMM-hh:mm");
    }
    if(autoZoom)
    {
        Orthodromie ortho (start.x(), start.y(), arrival.x(), arrival.y());
        const double    distance = ortho.getDistance();

        double xW, xE, yN, yS, xTmp, yTmp;
        double    ratio = 0.5;
        switch (zoomLevel)
        {
            case 3:
                ratio=0.1;
                break;
            case 2:
                ratio=0.5;
                break;
            case 1:
                ratio=0.8;
                break;
        }
        const double    angle = ortho.getLoxoCap();
        Util::getCoordFromDistanceAngle (start.y(), start.x(), ratio*distance/2, angle+90, &yTmp, &xTmp);
        xW = xE = xTmp;
        yN = yS = yTmp;
        Util::getCoordFromDistanceAngle (start.y(), start.x(), ratio*distance/2, angle-90, &yTmp, &xTmp);
        if (xTmp < xW) xW = xTmp;
        if (xTmp > xE) xE = xTmp;
        if (yTmp < yS) yS = yTmp;
        if (yTmp > yN) yN = yTmp;
        Util::getCoordFromDistanceAngle (arrival.y(), arrival.x(), ratio*distance/2, angle+90, &yTmp, &xTmp);
        if (xTmp < xW) xW = xTmp;
        if (xTmp > xE) xE = xTmp;
        if (yTmp < yS) yS = yTmp;
        if (yTmp > yN) yN = yTmp;
        Util::getCoordFromDistanceAngle (arrival.y(), arrival.x(), ratio*distance/2, angle-90, &yTmp, &xTmp);
        if (xTmp < xW) xW = xTmp;
        if (xTmp > xE) xE = xTmp;
        if (yTmp < yS) yS = yTmp;
        if (yTmp > yN) yN = yTmp;

        if((xW>0 && xE<0) || (xW<0 && xE>0))
        {
            if(qAbs(xW-xE)>180)
            {
                swap(xW,xE);
                if(xW>0)
                    xW-=360;
                else
                    xE-=360;

            }
        }

#if 0
        qWarning() << "Routing from " << start.x() << ", " << start.y() << " to " << arrival.x() << ", " << arrival.y();
        qWarning() << "-- Distance: " << distance;
        qWarning() << "-- North:    " << yN;
        qWarning() << "-- South:    " << yS;
        qWarning() << "-- West:     " << xW;
        qWarning() << "-- East:     " << xE;
#endif
        proj->zoomOnZone(xW,yN,xE,yS);
        connect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_calculate_with_tempo()));
        proj->setScale(proj->getScale()*.9);
        QApplication::processEvents();
    }
    else
        slot_calculate();
}
void ROUTAGE::slot_calculate_with_tempo()
{
    disconnect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_calculate_with_tempo()));
    timerTempo->start();
}

void ROUTAGE::slot_calculate()
{
    disconnect(proj,SIGNAL(projectionUpdated()),this,SLOT(slot_calculate()));
    double   cap;
    QTime timeTotal;
#ifdef traceTime
    QTime tfp;
#endif
    timeTotal.start();
    int refresh=0;
#ifdef traceTime
    int msecs_1=0;
    int msecs_2=0;
    int msecs_21=0;
    int msecs_3=0;
    int msecs_4=0;
    int msecs_5=0;
    int msecs_6=0;
    int msecs_7=0;
    int msecs_8=0;
    int msecs_9=0;
    int msecs_10=0;
    int msecs_11=0;
    int msecs_12=0;
    int msecs_13=0;
    int msecs_14=0;
    int msecs_15=0;
#endif
    int maxLoop=0;
    int nbCaps=0;
    int nbCapsPruned=0;
    QList<POI *> poiList=parent->getPois();
    for(int p=0;p<poiList.count();++p)
    {
        if(poiList.at(p)->getConnectedPoi()!=NULL)
        {
            POI * poi1=poiList.at(p);
            POI * poi2=poiList.at(p)->getConnectedPoi();
            poiList.removeOne(poi2);
            double x1,y1,x2,y2;
            proj->map2screenDouble(Util::cLFA(poi1->getLongitude(),proj->getXmin()),poi1->getLatitude(),&x1,&y1);
            proj->map2screenDouble(Util::cLFA(poi2->getLongitude(),proj->getXmin()),poi2->getLatitude(),&x2,&y2);
            barrieres.append(QLineF(x1,y1,x2,y2));
        }
    }
    //qWarning()<<"barrieres has"<<barrieres.count()<<"line(s)";
    msecsD1=0;
    msecsD2=0;
    debugCross0=0;
    debugCross1=0;
#ifdef traceTime
    QTime time,tDebug;
#endif
    QList<QColor> colorsList;
    colorsList.append(Qt::black);
    colorsList.append(Qt::red);
    colorsList.append(Qt::darkRed);
    colorsList.append(Qt::green);
    colorsList.append(Qt::darkGreen);
    colorsList.append(Qt::blue);
    colorsList.append(Qt::darkBlue);
    colorsList.append(Qt::cyan);
    colorsList.append(Qt::darkCyan);
    colorsList.append(Qt::magenta);
    colorsList.append(Qt::darkMagenta);
    colorsList.append(Qt::yellow);
    colorsList.append(Qt::darkYellow);
    int ncolor=0;
//    for (ncolor=0;ncolor<colorsList.count();++ncolor)
//        colorsList[ncolor].setAlpha(160);
    ncolor=0;
    whatIfJour=whatIfDate.toUTC().toTime_t();
    Orthodromie orth(0,0,0,0);
    orth.setPoints(start.x(),start.y(),arrival.x(),arrival.y());
    loxoCap=orth.getAzimutDeg();
    initialDist=orth.getDistance();
    proj->setFrozen(true);
    iso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    iso->setParent(this);
    vlmPoint point(start.x(),start.y());
    point.convertionLat=point.lat;
    point.convertionLon=point.lon;
    point.isStart=true;
    proj->map2screenDouble(Util::cLFA(start.x(),proj->getXmin()),start.y(),&xs,&ys);
    proj->map2screenDouble(Util::cLFA(arrival.x(),proj->getXmin()),arrival.y(),&xa,&ya);
    point.x=xs;
    point.y=ys;
#if 0
    point.distArrival=initialDist;
    point.distStart=0;
    point.capArrival=orth.getAzimutDeg();
    point.capStart=A360(-orth.getAzimutDeg());
#else
    QLineF tempLine(point.x,point.y,xa,ya);
    point.distStart=0;
    point.capStart=0;
    point.distArrival=tempLine.length();
    point.capArrival=Util::A360(-tempLine.angle()+90);
    point.distOrigin=0;
    initialDist=tempLine.length();
#endif
    approaching=false;
    point.origin=NULL;
    point.routage=this;
    point.capOrigin=Util::A360(loxoCap);
    if(i_iso)
        point.eta=i_eta;
    else
        point.eta=eta;
    iso->addVlmPoint(point);
    if(i_iso)
    {
        i_isochrones.append(iso);
    }
    else
    {
        isochrones.append(iso);
    }
    int nbIso=0;
    if(!i_iso)
        arrived=false;
    QList<vlmPoint> * list;
    //QList<vlmPoint> * previousList;
    vlmLine * currentIso;
    vlmLine * segment;
    QPen penSegment;
    QColor gray=Qt::gray;
    gray.setAlpha(230);
    penSegment.setColor(gray);
    penSegment.setBrush(gray);
    penSegment.setWidthF(0.5);
    //    QPolygonF * isoShape=new QPolygonF();
    //    isoShape->push_back(start);
    routeN=0;
    routeMaxN=0;
    routeTotN=0;
    routeFailedN=0;
    time_t workEta=0;
    NR_n=0;
    NR_success=0;
    time_t maxDate=grib->getMaxDate();
    if(angleRange>=180) angleRange=179;
    arrivalIsClosest=false;
    time_t realEta=eta;
    while(!aborted)
    {
#ifdef traceTime
        tDebug.start();
#endif
        currentIso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
        currentIso->setParent(this);
        list = iso->getPoints();
        int nbNotDead=0;
        double minDist=initialDist*10;
        double distStart=0;
#ifdef traceTime
        tfp.start();
#endif
        for(int n=0;n<list->count();++n)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            if(list->at(n).distArrival<minDist)
            {
                minDist=list->at(n).distArrival;
                distStart=list->at(n).distStart;
                if(!i_iso && distStart>0 && ((eta-this->startTime.toTime_t())*minDist)/distStart < 12*3600)
                    approaching=true;
            }
            double windSpeed,windAngle;
            double current_speed=-1;
            double current_angle=0;
            if(!windIsForced)
            {
                if(i_iso)
                    workEta = i_eta;
                else
                    workEta = eta;
                if(whatIfUsed && whatIfJour<=workEta)
                    workEta=workEta+whatIfTime*3600;
                if(!grib->getInterpolatedValue_byDates((double) list->at(n).lon,(double) list->at(n).lat,
                       workEta,&windSpeed,&windAngle,INTERPOLATION_DEFAULT)||workEta+this->getTimeStep()*60>maxDate)
                {
                    iso->setPointDead(n);
                    continue;
                }
                windAngle=radToDeg(windAngle);
                if(grib->getInterpolatedValueCurrent_byDates((double) list->at(n).lon,(double) list->at(n).lat,
                       workEta,&current_speed,&current_angle,INTERPOLATION_DEFAULT))
                {
                    current_angle=radToDeg(current_angle);
                    QPointF p=Util::calculateSumVect(windAngle,windSpeed,current_angle,current_speed);
                    windSpeed=p.x();
                    windAngle=p.y();
                }
                else
                {
                    current_speed=-1;
                    current_angle=0;
                }
            }
            else
            {
                windAngle=this->wind_angle;
                windSpeed=this->wind_speed;
            }
            if((!i_iso && whatIfUsed && whatIfJour<=eta) || (i_iso && whatIfUsed && whatIfJour<=i_eta))
                windSpeed=windSpeed*whatIfWind/100.00;
            if(i_iso)
            {
                windAngle=Util::A360(windAngle+180.0);
                current_angle=Util::A360(current_angle+180.0);
            }
            ++nbNotDead;
            iso->setPointWind(n,windAngle,windSpeed);
            iso->setPointCurrent(n,current_angle,current_speed);
            double vmg;
            myBoat->getPolarData()->getBvmg(Util::A360(list->at(n).capArrival-windAngle),windSpeed,&vmg);
            iso->setPointCapVmg(n,Util::A360(vmg+windAngle));
        }
#ifdef traceTime
        msecs_5+=tfp.elapsed();
#endif
        if(minDist<distStart)
            arrivalIsClosest=true;
        else
            arrivalIsClosest=false;
        if(nbNotDead==0) break;
        double workAngleStep=0;
        double workAngleRange=0;
        tempPoints.clear();
#ifdef traceTime
        msecsD1=msecsD1+tDebug.elapsed();
        time.start();
#endif
        bool hasTouchCoast=false;
        for(int n=0;n<list->count();++n)
        {
            if(aborted) break;
            if(list->at(n).isDead) continue;
            if(list->at(0).isStart)
            {
                workAngleStep=angleStep;
                workAngleRange=angleRange;
            }
            else
            {
        /*force une convergence logarithmique vers l'arrivee*/
                if(useConverge && arrivalIsClosest /*&& !i_iso*/)
                {
                    workAngleRange=qMax((double)angleRange/2,
                                        qMin((double) angleRange,
                                             (double)angleRange/(1+log(2*(list->at(n).distArrival/minDist)))));
                    workAngleStep=qMax(angleStep/2,workAngleRange/(angleRange/angleStep));
                    workAngleStep=qMax((double)3,workAngleStep); /*this allows less points generated but keep orginal total per iso*/
                }
                else
                {
                    workAngleRange=angleRange;
                    workAngleStep=qMax((double)3,angleStep);
                }
            }
            double windAngle=list->at(n).wind_angle;
            double windSpeed=list->at(n).wind_speed;
            double currentAngle=list->at(n).current_angle;
            double currentSpeed=list->at(n).current_speed;
            QList<double> caps;
            caps.reserve(workAngleRange/workAngleStep);
            calculateCaps(&caps,list->at(n),workAngleStep,workAngleRange);
            QList<vlmPoint> polarPoints;/**/
            bool tryingToFindHole=false;
#if 1 /*calculate angle limits*/
            QLineF limitRight,limitLeft;
            if(n>1)
            {
                limitRight.setPoints(QPointF(list->at(n-2).x,list->at(n-2).y),QPointF(xa,ya));
//                limitRight.setAngle(A360(90-list->at(n).capVmg));
                limitRight.setAngle(Util::A360(90-list->at(n-2).capOrigin));
                limitRight.setLength(list->at(n-2).distIso);
            }
            if(n<list->count()-2)
            {
                limitLeft.setPoints(QPointF(list->at(n+2).x,list->at(n+2).y),QPointF(xa,ya));
//                limitLeft.setAngle(A360(90-list->at(n).capVmg));
                limitLeft.setAngle(Util::A360(90-list->at(n+2).capOrigin));
                limitLeft.setLength(list->at(n+2).distIso);
            }
#endif
            while(true)
            {
                QList<vlmPoint> findPoints;
                findPoints.reserve(caps.count());
                for(int ccc=0;ccc<caps.count();++ccc)
                {
                    ++nbCaps;
    #if 1 /*use angle limits*/
                    if(!tryingToFindHole && !list->at(0).isStart && !list->at(n).notSimplificable)
                    {
                        QLineF temp(list->at(n).x,list->at(n).y,xa,ya);
                        temp.setAngle(Util::A360(90-caps.at(ccc)));
                        temp.setLength(list->at(n).distIso);
                        QPointF dummy;
                        if(n>1)
                        {
                            if(list->at(n).distIso<list->at(n-1).distIso)
                            {
                                if(temp.intersect(limitRight,&dummy)==QLineF::BoundedIntersection)
                                {
                                    ++nbCapsPruned;
                                    continue;
                                }
                            }
                        }
                        if(n<list->count()-2)
                        {
                            if(list->at(n).distIso<list->at(n+1).distIso)
                            {
                                if(temp.intersect(limitLeft,&dummy)==QLineF::BoundedIntersection)
                                {
                                    ++nbCapsPruned;
                                    continue;
                                }
                            }
                        }
                    }

    #endif
                    vlmPoint newPoint(0,0);
                    cap=caps.at(ccc);
                    double twa_x=qAbs(cap-wind_angle);
                    if(qAbs(twa_x)>180)
                    {
                        if(twa_x<0)
                            twa_x=360+twa_x;
                        else
                            twa_x=twa_x-360;
                    }
                    twa_x=qAbs(twa_x);
                    if(twa_x<=90 && windSpeed>this->maxPres) continue;
                    if(twa_x<=90 && windSpeed<this->minPres) continue;
                    if(twa_x>=90 && windSpeed>this->maxPortant) continue;
                    if(twa_x>=90 && windSpeed<this->minPortant) continue;
                    newPoint.routage=this;
                    newPoint.origin=iso->getPoint(n);
                    newPoint.originNb=n;
                    newPoint.wind_angle=windAngle;
                    newPoint.wind_speed=windSpeed;
                    newPoint.current_speed=currentSpeed;
                    newPoint.current_angle=currentAngle;
                    newPoint.capOrigin=cap;
                    if(i_iso)
                        newPoint.eta=i_eta;
                    else
                        newPoint.eta=eta;
                    if(n!=list->count()-1)
                    {
                        if(Util::myDiffAngle(newPoint.origin->capArrival,
                                       list->at(n+1).capArrival)<60)
                        {
                            newPoint.xP1=list->at(n+1).x;
                            newPoint.yP1=list->at(n+1).y;
                        }
                    }
                    if(n!=0)
                    {
                        if(Util::myDiffAngle(newPoint.origin->capArrival,
                                       list->at(n-1).capArrival)<60)
                        {
                            newPoint.xM1=list->at(n-1).x;
                            newPoint.yM1=list->at(n-1).y;
                        }
                    }
                    findPoints.append(newPoint);
                }
#ifdef traceTime
                tfp.start();
#endif
                if(!this->useMultiThreading)
                {
                    for(int pp=0;pp<findPoints.count();++pp)
                    {
                        findPoints.replace(pp,findPointThreaded(findPoints.at(pp)));
                    }
                }
                else
                {
                    findPoints = QtConcurrent::blockingMapped(findPoints, findPointThreaded);
                }
                for (int pp=findPoints.count()-1;pp>=0;--pp)
                {
                    if(findPoints.at(pp).isDead)
                        findPoints.removeAt(pp);
                }
#ifdef traceTime
                msecs_3=msecs_3+tfp.elapsed();
#endif
                bool toBeRestarted=false;
                for(int fp=0;fp<findPoints.count();++fp)
                {
                    vlmPoint newPoint=findPoints.at(fp);
                    if(checkCoast||checkLine)
                    {
/*check crossing with coast*/
#ifdef traceTime
                        tfp.start();
#endif
                        if((checkCoast && map && map->crossing(QLineF(list->at(n).x,list->at(n).y,newPoint.x,newPoint.y),QLineF(list->at(n).lon,list->at(n).lat,newPoint.lon,newPoint.lat)))
                                || (checkLine && crossBarriere(QLineF(list->at(n).x,list->at(n).y,newPoint.x,newPoint.y))))
                        {
#ifdef traceTime
                            msecs_14=msecs_14+tfp.elapsed();
#endif
                            if(!tryingToFindHole)
                            {
                                toBeRestarted=true;
                                tryingToFindHole=true;
                                hasTouchCoast=true;
                                polarPoints.clear();
                                caps.clear();
                                calculateCaps(&caps,list->at(n),1,179);
                                iso->setNotSimplificable(n);
                                break;
                            }
                            continue;
                        }
#ifdef traceTime
                        msecs_14=msecs_14+tfp.elapsed();
#endif
                    }
#ifdef traceTime
                    tfp.start();
#endif
                    QLineF tempLine(newPoint.x,newPoint.y,xs,ys);
                    newPoint.distStart=tempLine.length();
                    newPoint.capStart=Util::A360(-tempLine.angle()+90);
                    tempLine.setP2(QPointF(xa,ya));
                    newPoint.distArrival=tempLine.length();
                    newPoint.capArrival=Util::A360(-tempLine.angle()+90);
                    orth.setPoints(list->at(n).lon,list->at(n).lat,newPoint.lon,newPoint.lat);
                    newPoint.distOrigin=orth.getDistance();
#ifdef traceTime
                    msecs_21=msecs_21+tfp.elapsed();
#endif
                    if(tryingToFindHole)
                        newPoint.notSimplificable=true;
                    else if(!list->at(n).notSimplificable && !list->first().isStart)
                    {
                        if(newPoint.distStart<newPoint.distArrival)
                        {
                            if(newPoint.distStart<list->at(n).distStart) continue;
                        }
                        else
                        {
                            if(newPoint.distArrival>list->at(n).distArrival) continue;
                        }
                    }
                    polarPoints.append(newPoint);
                } /*end looping on caps*/
                if(!toBeRestarted || aborted) break;
                polarPoints.clear();
            }
            if(aborted) break;
#if 1 /* keep only max 80% of initial number of points per polar, based on distIso*/
            if(!tryingToFindHole && !list->at(n).isStart && !list->at(n).notSimplificable)
            {
                int max=(angleRange/angleStep)*0.8;
                if(max<polarPoints.count())
                {
                    QMultiMap<double,vlmPoint> dist;
                    for (int nn=polarPoints.count()-1;nn>=0;--nn)
                    {
                        dist.insert(polarPoints.at(nn).distIso,polarPoints.at(nn));
                    }
//                    QMapIterator<double,vlmPoint> it(dist);
//                    it.toFront();
//                    while(it.hasNext() && polarPoints.count()>max)
                    QMap<double,vlmPoint>::const_iterator it;
                    for(it=dist.constBegin();polarPoints.count()>max && it!=dist.constEnd();++it)
                    {
                        polarPoints.removeOne(it.value());
                    }
                }
            }
#endif
            //qWarning()<<nbIso<<"/"<<n<<"generated"<<polarPoints.count()<<"points";
            if(!polarPoints.isEmpty())
            {
                tempPoints.append(polarPoints);
            }
        }
#ifdef debugCount
        this->countDebug(nbIso,"initial count in tempPoints");
#endif
#ifdef traceTime
        msecs_1=msecs_1+time.elapsed();
#endif
/*1eme epuration: on supprime les segments qui se croisent */
        if(aborted) break;
#ifdef traceTime
        time.restart();
#endif
#if 1
        if(tempPoints.count()>0 && !tempPoints.first().origin->isStart)
             removeCrossedSegments();
#endif
#ifdef debugCount
        this->countDebug(nbIso,"after removeCrossedSegments()");
#endif
#ifdef traceTime
        msecs_2=msecs_2+time.elapsed();
#endif

        if(tempPoints.count()==0)
            break;
#if 1 /*check that the new iso itself does not cross previous segments or iso*/
#ifdef traceTime
        time.restart();
#endif
        if(!tempPoints.at(0).origin->isStart)
        {
            checkIsoCrossingPreviousSegments();
        }
#ifdef debugCount
        this->countDebug(nbIso,"after checkIsoCrossingPreviousSegments()");
#endif
#ifdef traceTime
        msecs_9=msecs_9+time.elapsed();
#endif
#endif
#if 1   /*eliminate points by wake pruning*/
#ifdef traceTime
        time.restart();
#endif
        if(!hasTouchCoast && !tempPoints.first().origin->isStart)
        {
            pruneWake(pruneWakeAngle);
        }
#ifdef debugCount
        this->countDebug(nbIso,"after pruneWake()");
#endif
#ifdef traceTime
        msecs_10=msecs_10+time.elapsed();
#endif
#endif

/*elimination de 50% des points surnumeraires*/
        //qWarning()<<"before first epuration"<<tempPoints.count();
#ifdef traceTime
        time.restart();
#endif
        int limit=(this->angleRange/this->angleStep)+this->explo;
        if(hasTouchCoast) limit=limit*1.5;
        int c=tempPoints.count();
        int toBeRemoved=c-limit;
        //qWarning()<<"before epuration()"<<tempPoints.count();
        if(tempPoints.count()>limit)
        {
            epuration(toBeRemoved*0.5);
        }
#ifdef debugCount
        this->countDebug(nbIso,"After elimination of 50%");
#endif
#ifdef traceTime
        msecs_6=msecs_6+time.elapsed();
#endif
#if 1 /*smoothing iso*/
#ifdef traceTime
        time.restart();
#endif
        if(!tempPoints.isEmpty() && !tempPoints.first().origin->isStart)
        {
            int nbPathSmooth=i_iso?10:2;
            for(int pass=1;pass<=nbPathSmooth;++pass)
            {
                for(int jj=1;jj<tempPoints.count()-1;++jj)
                {
                    if(tempPoints.at(jj).notSimplificable &&
                       !tempPoints.at(jj-1).notSimplificable &&
                       !tempPoints.at(jj+1).notSimplificable)
                        continue;
                    if(tempPoints.at(jj).origin->notSimplificable &&
                       !tempPoints.at(jj-1).origin->notSimplificable &&
                       !tempPoints.at(jj+1).origin->notSimplificable)
                        continue;
                    if(tempPoints.at(jj).distIso<tempPoints.at(jj-1).distIso &&
                       tempPoints.at(jj).distIso<tempPoints.at(jj+1).distIso)
                    {
                        QLineF temp1(tempPoints.at(jj).x,tempPoints.at(jj).y,
                                     tempPoints.at(jj-1).x,tempPoints.at(jj-1).y);
                        QLineF temp2(tempPoints.at(jj).x,tempPoints.at(jj).y,
                                     tempPoints.at(jj+1).x,tempPoints.at(jj+1).y);
                        if(temp1.length()>tempPoints.at(jj).distIso*5.0) continue;
                        if(temp2.length()>tempPoints.at(jj).distIso*5.0) continue;
                        double a=qAbs(temp1.angleTo(temp2));
                        if(a>180) a=360-a;
                        if(a>120) continue;
                        tempPoints.removeAt(jj);
                        --jj;
                    }
                }
            }
        }
#ifdef debugCount
        this->countDebug(nbIso,"after smoothing iso");
#endif
#ifdef traceTime
        msecs_4=msecs_4+time.elapsed();
#endif
#endif
        /*final checking and calculating route between Iso*/
        if(tempPoints.count()==0)
            break;
        somethingHasChanged=true;
#ifdef traceTime
        time.restart();
#endif
        bool routeDone=false;
        int nbLoop=0;
        while (somethingHasChanged && !tempPoints.isEmpty())
        {
            ++nbLoop;
            if(nbLoop>10)
            {
                qWarning()<<"didn't succeed to clean everything in final checking routine";
                break;
            }
            if(nbLoop>maxLoop)
                maxLoop=nbLoop;
            somethingHasChanged=false;
/*recheck that the new iso itself does not cross previous segments*/
            if(!tempPoints.first().origin->isStart && nbLoop<=5)
            {
                checkIsoCrossingPreviousSegments();
            }
#ifdef debugCount
            this->countDebug(nbIso,"after checkIsoCrossingPreviousSegments() -2");
#endif
            if(somethingHasChanged) continue;
/* now that some fast calculations have been made, compute real thing using route*/
            if(this->useRouteModule && !routeDone)
            {
#ifdef traceTime
                QTime t1,t2;
                t1.start();
#endif
                somethingHasChanged=true;
                routeDone=true;
                if(useMultiThreading)
                {
                    QList<QList<vlmPoint> > listList;
                    QList<vlmPoint> tempList;
                    int pp=0;
#if 1
                    int threadCount=QThread::idealThreadCount()*2;
                    for (int t=1;t<=threadCount;++t)
                    {
                        tempList.clear();
                        for (;(double)pp<(double)tempPoints.count()*(double)t/(double)threadCount;++pp)
                        {
                            tempList.append(tempPoints.at(pp));
                        }
                        listList.append(tempList);
                    }
#else
                    for (pp=0;pp<tempPoints.count();++pp)
                    {
                        tempList.clear();
                        tempList.append(tempPoints.at(pp));
                        listList.append(tempList);
                    }
#endif
                    tempList.clear();
                    listList = QtConcurrent::blockingMapped(listList, findRoute);
                    tempPoints.clear();
                    for(pp=0;pp<listList.count();++pp)
                        tempPoints.append(listList.at(pp));
#ifdef debugCount
                    this->countDebug(nbIso,"after calculating route");
#endif
                }
                for(int np=0;np<tempPoints.count();++np)
                {
                    vlmPoint newPoint=tempPoints.at(np);
                    if(!useMultiThreading)
                    {
                        QList<vlmPoint> tempPList;
                        tempPList.append(newPoint);
                        tempPList=findRoute(tempPList);
                        if(tempPList.isEmpty()) continue;
                        newPoint=tempPList.first();
                    }
                    if(this->visibleOnly && !proj->isInBounderies(newPoint.x,newPoint.y))
                        newPoint.isDead=true;
                    if(newPoint.isDead)
                    {
                        tempPoints.removeAt(np);
                        --np;
                        continue;
                    }
                    if(np!=0)
                    {
                        if(tempPoints.at(np-1)==newPoint)
                        {
                            tempPoints.removeAt(np);
                            --np;
                            continue;
                        }
                    }
                    double x,y;
                    proj->map2screenDouble(Util::cLFA(newPoint.lon,proj->getXmin()),newPoint.lat,&x,&y);
                    newPoint.x=x;
                    newPoint.y=y;
#if 1 /*check again if crossing with coast*/
                    if((checkCoast||checkLine) && !this->useMultiThreading)
                    {
#ifdef traceTime
                        t2.start();
#endif
                        double x1,y1,x2,y2;
                        x1=newPoint.origin->x;
                        y1=newPoint.origin->y;
                        x2=newPoint.x;
                        y2=newPoint.y;
                        if((checkCoast && map && map->crossing(QLineF(x1,y1,x2,y2),QLineF(newPoint.origin->lon,newPoint.origin->lat,newPoint.lon,newPoint.lat)))
                            ||( checkLine && crossBarriere(QLineF(x1,y1,x2,y2))))
                        {
#ifdef traceTime
                            msecs_14=msecs_14+t2.elapsed();
#endif
                            tempPoints.removeAt(np);
                            --np;
                            continue;
                        }
#ifdef traceTime
                        msecs_14=msecs_14+t2.elapsed();
#endif
                        if(this->getVisibleOnly() && !proj->isInBounderies_strict(x2,y2))
                        {
                            tempPoints.removeAt(np);
                            --np;
                            continue;
                        }
                    }
#endif
                    if(newPoint.origin->isStart)
                        newPoint.distIso=newPoint.distStart;
                    else
                        newPoint.distIso=ROUTAGE::findDistancePreviousIso(newPoint, &previousIso);
                    tempPoints.replace(np,newPoint);
                }
#ifdef debugCount
                this->countDebug(nbIso,"before checkCoastCollision");
#endif
                if((checkCoast || checkLine) && this->useMultiThreading)
                {
#ifdef traceTime
                    t2.start();
#endif
                    tempPoints = QtConcurrent::blockingMapped(tempPoints, checkCoastCollision);
                    for (int np=0;np<tempPoints.count();++np)
                    {
                        if(tempPoints.at(np).isDead)
                        {
                            tempPoints.removeAt(np);
                            --np;
                            continue;
                        }
                        if(this->getVisibleOnly() && !proj->isInBounderies_strict(tempPoints.at(np).x,tempPoints.at(np).y))
                        {
                            tempPoints.removeAt(np);
                            --np;
                            continue;
                        }
                    }
#ifdef traceTime
                    msecs_14=msecs_14+t2.elapsed();
#endif
                }
#ifdef traceTime
                msecs_12=msecs_12+t1.elapsed();
#endif
#ifdef debugCount
                this->countDebug(nbIso,"epuration before end of final loop");
#endif
                if(tempPoints.count()>0 && !tempPoints.first().origin->isStart)
                {
#ifdef traceTime
                    t1.start();
#endif
                    removeCrossedSegments();
                    checkIsoCrossingPreviousSegments();
#ifdef traceTime
                    msecs_13=msecs_13+t1.elapsed();
#endif
                }
#ifdef debugCount
                this->countDebug(nbIso,"at end of final loop");
#endif
            }
        }
#ifdef traceTime
        msecs_7=msecs_7+time.elapsed();
#endif
        /*epuration finale final part*/
#ifdef traceTime
        time.restart();
#endif
        c=tempPoints.count();
        toBeRemoved=c-limit;
#ifdef debugCount
        this->countDebug(nbIso,"before final epuration");
#endif
        if(tempPoints.count()>limit && !tempPoints.first().origin->isStart)
        {
            epuration(toBeRemoved);
        }
#ifdef debugCount
        this->countDebug(nbIso,"after final epuration");
#endif
        //qWarning()<<"after final epuration"<<tempPoints.count();
#ifdef traceTime
        msecs_6=msecs_6+time.elapsed();
#endif
        previousIso.clear();
#ifdef traceTime
        QTime t2;
#endif
        double x1,y1,x2,y2;
        if(tempPoints.count()>0)
        {
            int mmm=0;
            for (int n=0;n<tempPoints.count();++n)
            {
                if(tempPoints.at(n).isDead) continue;
                if(n!=tempPoints.count()-1 && (checkCoast || checkLine))
                {
#ifdef traceTime
                    t2.start();
#endif
                    x1=tempPoints.at(n).x;
                    y1=tempPoints.at(n).y;
                    x2=tempPoints.at(n+1).x;
                    y2=tempPoints.at(n+1).y;
                    if((checkCoast && map && map->crossing(QLineF(x1,y1,x2,y2),QLineF(tempPoints.at(n).lon,tempPoints.at(n).lat,tempPoints.at(n+1).lon,tempPoints.at(n+1).lat)))
                        || (checkLine && crossBarriere(QLineF(x1,y1,x2,y2))))
                    {
                        vlmPoint temp=tempPoints.at(n);
                        temp.isBroken=true;
                        tempPoints.replace(n,temp);
                    }
#ifdef traceTime
                    msecs_14=msecs_14+t2.elapsed();
#endif


                }
                if(i_iso)
                    tempPoints[n].eta=i_eta-(int)this->getTimeStep()*60.00;
                else
                    tempPoints[n].eta=eta+(int)this->getTimeStep()*60.00;
                currentIso->addVlmPoint(tempPoints[n]);
                if(n>0)
                {
                    if(qAbs(Util::myDiffAngle(tempPoints.at(n).capArrival,
                                                            tempPoints.at(n-1).capArrival)) < 60)
                        previousIso.append(QPointF(tempPoints.at(n).x,tempPoints.at(n).y));
                    else //insert same point not to loose increment
                        previousIso.append(previousIso.last());
                }
                else
                    previousIso.append(QPointF(tempPoints.at(n).x,tempPoints.at(n).y));
                if(!i_iso)
                {
                    vlmPointGraphic * vg=new vlmPointGraphic(this,nbIso+1,mmm,
                                                           tempPoints.at(n).lon,
                                                           tempPoints.at(n).lat,
                                                           this->proj,this->myscene,
                                                           Z_VALUE_ISOPOINT);
                    vg->setParent(this);
                    ++mmm;
#if 0
                    if(tempPoints.at(n).notSimplificable)
                        vg->setDebug("Not Simplicable");
                    else
                        vg->setDebug("Simplicable");
#endif
                    vg->setEta(eta+(int)this->getTimeStep()*60.00);
                    connect(this,SIGNAL(updateVgTip(int,int,QString)),vg,SLOT(slot_updateTip(int,int,QString)));
                    this->isoPointList.append(vg);
                    vg->slot_showMe();
#if 0
                    datathread dataThread;
                    dataThread.Boat=this->getBoat();
                    dataThread.GriB=this->getGrib();
                    dataThread.whatIfJour=this->getWhatIfJour();
                    dataThread.whatIfUsed=this->getWhatIfUsed();
                    dataThread.windIsForced=this->getWindIsForced();
                    dataThread.whatIfTime=this->getWhatIfTime();
                    dataThread.windAngle=this->getWindAngle();
                    dataThread.windSpeed=this->getWindSpeed();
                    dataThread.whatIfWind=this->getWhatIfWind();
                    dataThread.timeStep=this->getTimeStep();
                    dataThread.speedLossOnTack=this->getSpeedLossOnTack();
                    dataThread.i_iso=i_iso;
                    vlmPoint to=tempPoints.at(n);
                    to.lon=tempPoints.at(n).convertionLon;
                    to.lat=tempPoints.at(n).convertionLat;
                    vlmPoint from(to.origin->lon, to.origin->lat);
                    dataThread.Eta=to.origin->eta;
                    int realTime=calculateTimeRoute(from,to,&dataThread);
                    if(realTime!=this->getTimeStep()*60)
                    {
                        qWarning()<<"anomaly"<<n<<nbIso+1<<realTime<<to.foundByNewtonRaphson<<"debug="<<to.debug;
                    }
//                    else
//                        qWarning()<<"OK"<<n<<nbIso+1<<realTime<<to.foundByNewtonRaphson<<"dead="<<to.isDead;

#endif
                }
            }
        }
        else
            break;
        list = currentIso->getPoints();
//        if(nbIso==0)
//            qWarning()<<"iso 0 contains"<<list->count()<<"points";
        iso=currentIso;
        if(ncolor>=colorsList.count()) ncolor=0;
        QColor col=colorsList[ncolor];
        if(i_iso)
            col=Qt::red;
        col.setAlpha(160);
        pen.setColor(col);
        pen.setBrush(col);
        iso->setLinePen(pen);
        ++ncolor;
        previousSegments.clear();
#ifdef traceTime
        time.restart();
#endif
        for (int n=0;n<list->count();++n)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
#if 1
            segment=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
            segment->setParent(this);
            vlmPoint temp=* iso->getOrigin(n);
            temp.isBroken=false;
            segment->addVlmPoint(temp);
            temp=list->at(n);
            temp.isBroken=false;
            segment->addVlmPoint(temp);
#if 0 //debug left-right balancing
            if(temp.debugInt==0)
                penSegment.setColor(Qt::black);
            else if (temp.debugInt==1)
                penSegment.setColor(Qt::blue);
            else if (temp.debugInt==2)
                penSegment.setColor(Qt::red);
            else if (temp.debugInt==3)
                penSegment.setColor(Qt::yellow);
            else if (temp.debugInt==4)
                penSegment.setColor(Qt::magenta);
#endif
            segment->setLinePen(penSegment);
            segment->slot_showMe();
            if(i_iso)
            {
                i_segments.append(segment);
                segment->setHidden(true);
            }
            else
                segments.append(segment);
#endif
            previousSegments.append(QLineF(list->at(n).origin->x,list->at(n).origin->y,list->at(n).x,list->at(n).y));
        }
#ifdef traceTime
        msecs_15=msecs_15+time.elapsed();
#endif
        iso->slot_showMe();
#ifdef traceTime
        time.restart();
#endif
        if(++refresh%4==0)
        {
            QCoreApplication::processEvents();
        }
#ifdef traceTime
        msecs_11+=time.elapsed();
#endif
        ++nbIso;
        if(i_iso)
        {
            i_eta=i_eta-(int)this->getTimeStep()*60.00;
            //qWarning()<<nbIso<<QDateTime().fromTime_t(i_eta).toUTC().toString("dd MMM-hh:mm");
        }
        else
            eta=eta+(int)this->getTimeStep()*60.00;
        if(i_iso)
            i_isochrones.append(iso);
        else
            isochrones.append(iso);
        vlmPoint to(arrival.x(),arrival.y());
#ifdef traceTime
        time.restart();
#endif
        datathread dataThread;
        dataThread.Boat=this->getBoat();
        if(i_iso)
            dataThread.Eta=i_eta;
        else
            dataThread.Eta=eta;
        dataThread.GriB=this->getGrib();
        dataThread.whatIfJour=this->getWhatIfJour();
        dataThread.whatIfUsed=this->getWhatIfUsed();
        dataThread.windIsForced=this->getWindIsForced();
        dataThread.whatIfTime=this->getWhatIfTime();
        dataThread.windAngle=this->getWindAngle();
        dataThread.windSpeed=this->getWindSpeed();
        dataThread.whatIfWind=this->getWhatIfWind();
        dataThread.timeStep=this->getTimeStep();
        dataThread.speedLossOnTack=this->getSpeedLossOnTack();
        dataThread.i_iso=i_iso;
        bool i_arrived=false;
        for (int n=0;n<list->count();++n)
        {
            vlmPoint from=list->at(n);
            orth.setPoints(from.lon,from.lat,to.lon,to.lat);
            if(orth.getDistance()<myBoat->getPolarData()->getMaxSpeed()*1.1*(this->getTimeStep()/60.0))
            {
                if((checkCoast && map && map->crossing(QLineF(list->at(n).x,list->at(n).y,xa,ya),QLineF(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y())))
                   || (checkLine && crossBarriere(QLineF(list->at(n).x,list->at(n).y,xa,ya))))
                    continue;
                int thisTime=calculateTimeRoute(from,to, &dataThread, NULL, NULL, (this->getTimeStep()+1)*60);
                if(thisTime<=this->getTimeStep()*60)
                {
                    arrived=true;
                    i_arrived=true;
                    break;
                }
            }
        }
#ifdef traceTime
        msecs_8=msecs_8+time.elapsed();
#endif
        if (i_iso && i_arrived) break;
        if (!i_iso && arrived)
        {
            list=iso->getPoints();
            int nBest=0;
            int minTime=this->getTimeStep()*10000*60;
            vlmPoint to(arrival.x(),arrival.y());
            datathread dataThread;
            dataThread.Boat=this->getBoat();
            dataThread.Eta=this->getEta();
            dataThread.GriB=this->getGrib();
            dataThread.whatIfJour=this->getWhatIfJour();
            dataThread.whatIfUsed=this->getWhatIfUsed();
            dataThread.windIsForced=this->getWindIsForced();
            dataThread.whatIfTime=this->getWhatIfTime();
            dataThread.windAngle=this->getWindAngle();
            dataThread.windSpeed=this->getWindSpeed();
            dataThread.whatIfWind=this->getWhatIfWind();
            dataThread.timeStep=this->getTimeStep();
            dataThread.speedLossOnTack=this->getSpeedLossOnTack();
            dataThread.i_iso=i_iso;
            for(int n=0;n<list->count();++n)
            {
                if((checkCoast && map && map->crossing(QLineF(list->at(n).x,list->at(n).y,xa,ya),
                        QLineF(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y())))
                    || (checkLine && crossBarriere(QLineF(list->at(n).x,list->at(n).y,xa,ya))))
                    continue;
                vlmPoint from=list->at(n);
                int thisTime=calculateTimeRoute(from,to,&dataThread, NULL, NULL, (this->getTimeStep()+1)*60);
                if(thisTime<minTime)
                {
                    nBest=n;
                    minTime=thisTime;
                }
            }
            realEta=eta+minTime;
            drawResult(list->at(nBest));
            QApplication::processEvents();
            //qWarning()<<"result drawn and stored";
            if(nbAlternative!=0) calculateAlternative();
            break;
        }
        if(nbIso>3000 || nbNotDead<=0)
        {
            break;
        }
    }
    if(!i_iso)
    {
        if(!arrived)
        {
            eta=realEta;
            list=iso->getPoints();
            int nBest=0;
            int minDist=10e5;
            Orthodromie oo(0,0,arrival.x(),arrival.y());
            for(int n=0;n<list->count();++n)
            {
                oo.setStartPoint(list->at(n).lon,list->at(n).lat);
                if(oo.getDistance()<minDist)
                {
                    nBest=n;
                    minDist=oo.getDistance();
                }
            }
            drawResult(list->at(nBest));
        }
    }
    if(routeN==0) routeN=1;
    if(!i_iso)
    {
        QTime tt(0,0,0,0);
        int msecs=timeTotal.elapsed();
        tt=tt.addMSecs(msecs);
        QString info;
        qWarning()<<"Total calculation time:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info="Total calculation time: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
#ifdef traceTime
        QString temp;
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_5);
        qWarning()<<"...Preparation loop:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...Preparation loop: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_1);
        qWarning()<<"...Calculating iso points:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...Calculating iso points: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_3);
        qWarning()<<".........out of which inside findPoint():"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n.........out of which inside findPoint(): "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_14);
        qWarning()<<".........out of which detecting collision with coasts:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n.........out of which detecting collision with coasts: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_21);
        qWarning()<<".........out of which calculating distances and angles:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n.........out of which calculating distances and angles: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_2);
        qWarning()<<"...removing crossed segments:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...removing crossed segments: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_4);
        qWarning()<<"...smoothing isochron:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...smoothing isochron: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_9);
        qWarning()<<"...checking iso not crossing previous segments or iso:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...checking iso not crossing previous segments or iso: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_10);
        qWarning()<<"...pruning by wake:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...pruning by wake: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_6);
        qWarning()<<"...final cleaning:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...final cleaning: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_7);
        qWarning()<<"...Final checking and calculating route between Isos:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'")<<"(maxLoop="<<maxLoop<<")";
        info=info+"\n...Final checking and calculating route between Isos: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_12);
#endif
        int nbCpu=1;
        if(this->useMultiThreading)
            nbCpu=QThread::idealThreadCount();
#ifdef traceTime
        qWarning()<<"........out of which calculating route between Isos:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'")<<"(ran on"<<nbCpu<<"threads)";
        info=info+"\n........out of which calculating route between Isos: "+tt.toString("hh'h'mm'min'ss.zzz'secs'")+" (ran on "+temp.sprintf("%d",nbCpu)+" threads)";
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_13);
        qWarning()<<"........out of which removing crossed route segments:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n........out of which removing crossed route segments: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_8);
        qWarning()<<"...checking if arrived:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...checking if arrived: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_15);
        qWarning()<<"...generating segments"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...generating segments: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs_11);
        qWarning()<<"...processing events"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...processing events: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs-(msecs_1+msecs_2+msecs_4+msecs_5+msecs_6+msecs_7+msecs_8+msecs_9+msecs_10+msecs_11+msecs_15));
        qWarning()<<"...sum of other calculations:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        info=info+"\n...sum of other calculations: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
        if(msecsD1!=0)
        {
            tt.setHMS(0,0,0,0);
            tt=tt.addMSecs(msecsD1);
            qWarning()<<"..............debug time 1:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        }
        if(msecsD2!=0)
        {
            tt.setHMS(0,0,0,0);
            tt=tt.addMSecs(msecsD2);
            qWarning()<<"..............debug time 2:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
        }
        qWarning()<<"nb of caps generated:"<<nbCaps<<"nb of caps ignored:"<<nbCapsPruned;
        qWarning()<<"debugCross0="<<debugCross0<<"debugCross1="<<debugCross1;
#endif
        QMessageBox msgBox;
//        if(this->useRouteModule && !this->useMultiThreading)
//        {
//            info=info+"\n---Route module statistics---";
//            info=info+"\nTotal number of route segments calculated: "+temp.sprintf("%d",routeN);
//            info=info+"\npercentage of correct guesses using rough method: "+temp.sprintf("%.2f",100-((double)(NR_n)/(double)routeN)*100.00)+"%";
//            info=info+"\nAverage number of iterations: "+temp.sprintf("%.2f",(double)routeTotN/(double)routeN);
//            info=info+"\nMax number of iterations made to find a solution: "+temp.sprintf("%d",routeMaxN);
//            info=info+"\nNumber of failures using Route between Iso: "+temp.sprintf("%d",routeFailedN);
//            info=info+"\nNumber of successes using Route between Iso: "+temp.sprintf("%d",routeN-routeFailedN);
//            info=info+"\nNumber of Newton-Raphson calculations: "+temp.sprintf("%d",NR_n);
//            info=info+"\nNumber of Newton-Raphson successful calculations: "+temp.sprintf("%d",NR_success);
//            info=info+"\n-------------------------------";
//        }

#ifdef traceTime
        msgBox.setDetailedText(info);
#endif
        finalEta=QDateTime::fromTime_t(realEta).toUTC();
        msgBox.setIcon(QMessageBox::Information);
        tt.setHMS(0,0,0,0);
        tt=tt.addMSecs(msecs);
        int elapsed=finalEta.toTime_t()-this->startTime.toTime_t();
        QTime eLapsed(0,0,0,0);
        double jours=elapsed/(24*60*60);
        if (qRound(jours)>jours)
            --jours;
        jours=qRound(jours);
        elapsed=elapsed-jours*24*60*60;
        eLapsed=eLapsed.addSecs(elapsed);
        QString jour;
        jour=jour.sprintf("%d",qRound(jours));
        if(arrived)
        {
            msgBox.setText(tr("Date et heure d'arrivee: ")+finalEta.toString("dd MMM-hh:mm")+
                           tr("<br>Arrivee en: ")+jour+tr(" jours ")+eLapsed.toString("hh'h 'mm'min 'ss'secs'")+
                           tr("<br><br>Temps de calcul: ")+tt.toString("hh'h 'mm'min 'ss'secs'"));
        }
        else
        {
            if(aborted)
                msgBox.setText(tr("Routage arrete par l'utilisateur"));
            else
                msgBox.setText(tr("Impossible de rejoindre l'arrivee, desole"));
        }
        QSpacerItem * hs=new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding);
        QGridLayout * layout =(QGridLayout*)msgBox.layout();
        layout->addItem(hs,layout->rowCount(),0,1,layout->columnCount());
        msgBox.exec();
//        if(this->useRouteModule)
//        {
//            qWarning()<<"---Route module statistics---";
//            qWarning()<<"Total number of route segments calculated"<<routeN;
//            qWarning()<<"Average number of iterations"<<(double)routeTotN/(double)routeN;
//            qWarning()<<"Max number of iterations made to find a solution"<<routeMaxN;
//            qWarning()<<"Number of failures using Route between Iso"<<routeFailedN;
//            qWarning()<<"Number of successes using Route between Iso"<<routeN-routeFailedN;
//            qWarning()<<"Number of Newton-Raphson calculations"<<NR_n;
//            qWarning()<<"Number of Newton-Raphson successful calculations"<<NR_success;
//            qWarning()<<"-------------------------------";
//        }
        if (!this->showIso)
            setShowIso(false);
        this->done=true;
        if(this->colorGrib)
            parent->getTerre()->setRoutageGrib(this);
    }
    proj->setFrozen(false);
    if(isConverted() && !i_iso)
    {
        int rep=QMessageBox::Yes;
        if(whatIfUsed && (whatIfTime!=0 || whatIfWind!=100))
        {
            rep = QMessageBox::question (0,
                    tr("Convertir en route"),
                    tr("Ce routage a ete calcule avec une hypothese modifiant les donnees du grib<br>La route ne prendra pas ce scenario en compte<br>Etes vous sur de vouloir le convertir en route?"),
                    QMessageBox::Yes | QMessageBox::No);
        }
        if(rep==QMessageBox::Yes)
        {
            convertToRoute();
            running=false;
            return;
        }
        else
            this->converted=false;
    }
    else if (i_iso)
    {
        this->i_done=true;
        this->showIsoRoute();
    }
    this->slot_gribDateChanged();
    running=false;
}
void ROUTAGE::countDebug(int nbIso, QString s)
{
    if(nbIso!=0) return;
    int count=0;
    int nDead=0;
    for (int n=0;n<tempPoints.count();++n)
    {
        if(tempPoints.at(n).originNb==0)
        {
            ++count;
            if(s.contains("initial"))
            {
                QPen pendebug(Qt::blue);
                pendebug.setWidthF(1);
                vlmLine * debug1=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug1->setParent(this);
                debug1->setLinePen(pendebug);
                debug1->addVlmPoint(tempPoints.at(n));
                debug1->addVlmPoint(*tempPoints.at(n).origin);
                debug1->slot_showMe();
            }
            if(tempPoints.at(n).isDead)
            {
                ++nDead;
            }
        }
    }
    qWarning()<<"remains"<<count<<"points, dead="<<nDead<<s;
}

double ROUTAGE::findDistancePreviousIso(const vlmPoint &P, const QPolygonF * poly)
{
    double cx=P.x;
    double cy=P.y;
    double minDistanceSegment=10e6;

    for(int i=0;i<poly->count()-1;++i)
    {
        double ax=poly->at(i).x();
        double ay=poly->at(i).y();
        double bx=poly->at(i+1).x();
        double by=poly->at(i+1).y();
        double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
        double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
        double r = r_numerator / r_denomenator;
//
//        double px = ax + r*(bx-ax);
//        double py = ay + r*(by-ay);
//
        double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;

        double distanceLine = fabs(s)*sqrt(r_denomenator);

//
// (xx,yy) is the point on the lineSegment closest to (cx,cy)
//
//        double xx = px;
//        double yy = py;
        double distanceSegment=0;
        if ( (r >= 0) && (r <= 1) )
        {
            distanceSegment = distanceLine;
        }
        else
        {
            double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
            double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
            if (dist1 < dist2)
            {
//                    xx = ax;
//                    yy = ay;
                    distanceSegment = sqrt(dist1);
            }
            else
            {
//                    xx = bx;
//                    yy = by;
                    distanceSegment = sqrt(dist2);
            }

        }
        if(distanceSegment<minDistanceSegment)
            minDistanceSegment=distanceSegment;
    }
    return minDistanceSegment;
}
double ROUTAGE::findDistancePoly(const QPointF P, const QPolygonF * poly, QPointF * closest)
{
    double cx=P.x();
    double cy=P.y();
    double minDistanceSegment=10e6;

    for(int i=0;i<poly->count()-1;++i)
    {
        double ax=poly->at(i).x();
        double ay=poly->at(i).y();
        double bx=poly->at(i+1).x();
        double by=poly->at(i+1).y();
        double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
        double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
        double r = r_numerator / r_denomenator;
//
        double px = ax + r*(bx-ax);
        double py = ay + r*(by-ay);
//
        double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;

        double distanceLine = fabs(s)*sqrt(r_denomenator);

//
// (xx,yy) is the point on the lineSegment closest to (cx,cy)
//
        double xx = px;
        double yy = py;
        double distanceSegment=0;
        if ( (r >= 0) && (r <= 1) )
        {
            distanceSegment = distanceLine;
            closest->setX(xx);
            closest->setY(yy);
        }
        else
        {
            double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
            double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
            if (dist1 < dist2)
            {
                    xx = ax;
                    yy = ay;
                    distanceSegment = sqrt(dist1);
            }
            else
            {
                    xx = bx;
                    yy = by;
                    distanceSegment = sqrt(dist2);
            }
            closest->setX(xx);
            closest->setY(yy);
        }
        if(distanceSegment<minDistanceSegment)
            minDistanceSegment=distanceSegment;
    }
    return minDistanceSegment;
}
void ROUTAGE::pruneWake(int wakeAngle)
{
    if (wakeAngle<1) return;
    QList<vlmPoint> *pIso;
    double wakeDir=0;
    if(i_iso)
        pIso=i_isochrones.last()->getPoints();
    else
        pIso=isochrones.last()->getPoints();
    for(int n=tempPoints.count()-1;n>=0;--n)
    {
        double x1,y1,x2,y2;
        x1=tempPoints.at(n).x;
        y1=tempPoints.at(n).y;
        for(int m=0;m<pIso->count();++m)
        {
            if(pIso->at(m).distArrival>tempPoints.at(n).distArrival) continue;
            if(pIso->at(m).isDead) continue;
            double pc1=pIso->at(m).distArrival/initialDist;
            double pc2=tempPoints.at(n).distArrival/initialDist;
            if(pc2-pc1<0.30) continue;
            if(Util::myDiffAngle(pIso->at(m).capArrival,tempPoints.at(n).capArrival)>60) continue;
            QLineF toArrival(pIso->at(m).x,pIso->at(m).y,xa,ya);
            QLineF toArrival2(tempPoints.at(n).x,tempPoints.at(n).y,xa,ya);
            double a=toArrival.angleTo(toArrival2);
            if(a>180) a=360-a;
            if(a>90) continue;
            x2=pIso->at(m).x;
            y2=pIso->at(m).y;
            QPolygonF wake;
            wake.append(QPointF(x2,y2));
            QLineF temp(xa,ya,x1,y1);
            wakeDir=temp.angle();
            QLineF temp1(x2,y2,x1,y1);
            temp1.setAngle(Util::A360(wakeDir+wakeAngle));
            temp1.setLength(10e5);
            wake.append(temp1.p2());
            temp1.setAngle(Util::A360(wakeDir-wakeAngle));
            wake.append(temp1.p2());
            wake.append(QPointF(x2,y2));
            if(wake.containsPoint(QPointF(x1,y1),Qt::OddEvenFill))
            {
                if(this->checkCoast || checkLine)
                {
                    if((!checkCoast || (map &&!map->crossing(QLineF(x1,y1,pIso->at(m).x,pIso->at(m).y),QLineF(tempPoints.at(n).lon,tempPoints.at(n).lat,pIso->at(m).lon,pIso->at(m).lat))))
                        && (!checkLine || !crossBarriere(QLineF(x1,y1,pIso->at(m).x,pIso->at(m).y))))
                    {
                        tempPoints.removeAt(n);
                        break;
                    }
                }
                else
                {
                    tempPoints.removeAt(n);
                    break;
                }
            }
        }
    }
}
double ROUTAGE::findTime(const vlmPoint * pt, QPointF P, double * cap)
{
    double angle,newSpeed,lon,lat,windSpeed,windAngle;
    lon=P.x();
    lat=P.y();
    windSpeed=pt->wind_speed;
    windAngle=pt->wind_angle;
    Orthodromie orth(pt->lon,pt->lat,lon,lat);
    *cap=orth.getAzimutDeg();
    angle=*cap-(double)windAngle;
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    if(qAbs(angle)<myBoat->getPolarData()->getBvmgUp(windSpeed) && angle!=90) //if too close to wind then use BVMG technique
    {
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,myBoat->getPolarData()->getBvmgUp(windSpeed));
//        newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(myBoat->getPolarData()->getBvmgUp(windSpeed),qAbs(angle)))));
        newSpeed=newSpeed*qAbs(cos(degToRad(myBoat->getPolarData()->getBvmgUp(windSpeed)))/cos(degToRad(qAbs(angle))));
    }
    else if(qAbs(angle)>myBoat->getPolarData()->getBvmgDown(windSpeed) && angle!=90)
    {
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,myBoat->getPolarData()->getBvmgDown(windSpeed));
//        newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(qAbs(angle),myBoat->getPolarData()->getBvmgDown(windSpeed)))));
        newSpeed=newSpeed*qAbs(cos(degToRad(myBoat->getPolarData()->getBvmgDown(windSpeed)))/cos(degToRad(qAbs(angle))));
    }
    else
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,angle);
    return orth.getDistance()/newSpeed;
}
double ROUTAGE::mySignedDiffAngle(const double a1,const double a2)
{
    return (Util::A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
}
void ROUTAGE::slot_edit()
{
    emit editMe(this);
}
void ROUTAGE::slot_gribDateChanged()
{
    QPen p=isochrones.at(highlightedIso)->getLinePen();
    p.setWidthF(2);
    isochrones.at(highlightedIso)->setLinePen(p);
    int maxTime=10e5;
    int isoNb=0;
    bool found=false;
    for(int n=0;n<isochrones.count();++n)
    {
        int time=qAbs(isochrones.at(n)->getPoint(0)->eta - grib->getCurrentDate());
        if (time<maxTime)
        {
            maxTime=time;
            isoNb=n;
            found=true;
        }
        if(found && time>maxTime) break;
    }
    highlightedIso=isoNb;
    p=isochrones.at(highlightedIso)->getLinePen();
    p.setWidthF(6);
    isochrones.at(highlightedIso)->setLinePen(p);
}

void ROUTAGE::setShowIso(const bool &b)
{
    this->showIso=b;
    for (int n=0;n<isochrones.count();++n)
    {
        isochrones[n]->setHidden(!showIso);
        isochrones[n]->blockSignals(!b);
    }
    for (int n=0;n<i_isochrones.count();++n)
    {
        i_isochrones[n]->setHidden(!showIso);
        i_isochrones[n]->blockSignals(!b);
    }
    for (int n=0;n<segments.count();++n)
    {
        segments[n]->setHidden(!showIso);
        segments[n]->blockSignals(!b);
    }
    for (int n=0;n<isoPointList.count();++n)
    {
        isoPointList[n]->shown(b);
        isoPointList[n]->blockSignals(!b);
    }
    for (int n=0;n<alternateRoutes.count();++n)
    {
        alternateRoutes[n]->setHidden(!showIso);
        alternateRoutes[n]->blockSignals(!b);
    }
    way->hide();
    if(result && b)
    {
        result->show();
        result->slot_showMe();
    }
}
void ROUTAGE::drawResult(vlmPoint P)
{
    QList<vlmPoint> initialRoad;
    for (int n=0;n<result->getPoints()->count();++n)
    {
        vlmPoint temp=result->getPoints()->at(n);
        temp.isBroken=false;
        initialRoad.append(temp);
    }
    result->deleteAll();
    if(arrived)
    {
        vlmPoint ar(arrival.x(),arrival.y());
        ar.convertionLat=ar.lat;
        ar.convertionLon=ar.lon;
        result->addVlmPoint(ar);
    }
    while (true)
    {
        P.isBroken=false;
        result->addVlmPoint(P);
        if (P.isStart) break;
        P= (*P.origin);
    }
    result->setNotSimplificable(result->count()-1);
    for(int n=1;n<initialRoad.count();++n)
        result->addVlmPoint(initialRoad.at(n));
    pen.setWidthF(width);
    pen.setColor(color);
    pen.setBrush(color);
    result->setLinePen(pen);
    result->slot_showMe();
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(2);
}
void ROUTAGE::setPivotPoint(const int &isoNb,const int &pointNb)
{
    if(isoNb<0 || isoNb>=isochrones.count())
        return;
    if(pointNb<0 || pointNb>=isochrones.at(isoNb)->count())
        return;
    pivotPoint=isochrones.at(isoNb)->getPoints()->at(pointNb);
}

void ROUTAGE::slot_drawWay()
{
    QList<QColor> colorsList;
    colorsList.append(Qt::yellow);
    colorsList.append(Qt::blue);
    colorsList.append(Qt::red);
    colorsList.append(Qt::green);
    colorsList.append(Qt::cyan);
    colorsList.append(Qt::magenta);
    colorsList.append(Qt::black);
    colorsList.append(Qt::darkRed);
    colorsList.append(Qt::darkGreen);
    colorsList.append(Qt::darkBlue);
    colorsList.append(Qt::darkCyan);
    colorsList.append(Qt::darkMagenta);
    colorsList.append(Qt::darkYellow);
    int ncolor=parent->getNbRoutage();
    while (ncolor>=colorsList.count())
        ncolor=ncolor-colorsList.count();
    vlmPoint P=pivotPoint;
    P.notSimplificable=true;
    way->deleteAll();
    while (true)
    {
        P.isBroken=false;
        way->addVlmPoint(P);
        if (P.isStart) break;
        P= (*P.origin);
    }
    pen.setWidthF(width);
    pen.setColor(colorsList[ncolor]);
    pen.setBrush(colorsList[ncolor]);
    way->setLinePen(pen);
    way->slot_showMe();
    way->show();
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(2);
}
void ROUTAGE::eraseWay()
{
    //way->deleteAll();
    way->hide();
}

void ROUTAGE::convertToRoute()
{
    bool routeStartBoat=false;
    ROUTAGE * parentRoutage=this;
    while(parentRoutage->getIsPivot())
    {
        if(parentRoutage->getFromRoutage())
        {
            parentRoutage=parentRoutage->getFromRoutage();
            if(!parent->getRoutageList().contains(parentRoutage))
            {
                QMessageBox::critical(0,tr("Conversion d'un routage en route"),
                                      tr("Un des routages en amont a ete supprime,<br>la conversion est impossible"));
                return;
            }
        }
        else
            break;
    }
    this->converted=false;
    if(parentRoutage->getRouteFromBoat())
    {
        int answ=QMessageBox::question(0,tr("Convertion d'un routage en route"),
                                         tr("Voulez-vous que le point de depart de la route suive le bateau maintenant?"),
                                              QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if(answ==QMessageBox::Cancel) return;
        routeStartBoat=answ==QMessageBox::Yes;
    }
    this->converted=true;
    ROUTE * route=parent->addRoute();
    route->setName(name);
    route->setUseVbVmgVlm(false);
    parent->update_menuRoute();
    route->setBoat(this->myBoat);
    route->setDetectCoasts(this->checkCoast);
    route->setStartTime(parentRoutage->getStartTime());
    if(routeStartBoat)
    {
        route->setStartFromBoat(true);
        route->setStartTimeOption(1);
    }
    else
    {
        route->setStartTimeOption(3);
        route->setStartFromBoat(false);
    }
    route->setColor(this->color);
    route->setSpeedLossOnTack(this->speedLossOnTack);
    //route->setWidth(this->width);
    route->setFrozen(true);
    QList<vlmPoint> * list=result->getPoints();
    for (int n=0;n<list->count();++n)
    {
       if(n!=list->count()-1)
       {
           if(list->at(n)==list->at(n+1)) continue;
       }
       QString poiName;
       poiName.sprintf("%.5i",list->count()-n);
       poiName=poiPrefix+poiName;
       POI * poi = parent->slot_addPOI(poiName,0,list->at(n).convertionLat,list->at(n).convertionLon,-1,false,false,myBoat);
       poi->setRoute(route);
       poi->setNotSimplificable(list->at(n).notSimplificable);
    }
    delete result;
    result=NULL;
    route->setHidePois(true);
    route->setFrozen(false);
    if(routeStartBoat)
    {
        if (!route->getPoiList().isEmpty() && route->getPoiList().at(0)->getRouteTimeStamp()!=-1)
        {
            if(qAbs(route->getPoiList().at(0)->getRouteTimeStamp()-myBoat->getPrevVac())<myBoat->getVacLen()*2.0 || (myBoat->getType()==BOAT_VLM && myBoat->getLoch()<0.1))
            {
                POI * poi = route->getPoiList().at(0);
                poi->setRoute(NULL);
                parent->slot_delPOI_list(poi);
                delete poi;
            }
        }
    }
    parent->deleteRoutage(this);
}
void ROUTAGE::checkIsoCrossingPreviousSegments()
{
    QPointF dummy;
    for(int nn=0;nn<tempPoints.count()-1;++nn)
    {
        if(tempPoints.at(nn).notSimplificable || tempPoints.at(nn+1).notSimplificable)
            continue;
        if(tempPoints.at(nn).origin->notSimplificable || tempPoints.at(nn+1).origin->notSimplificable)
            continue;
        QLineF S1(tempPoints.at(nn).x,tempPoints.at(nn).y,tempPoints.at(nn+1).x,tempPoints.at(nn+1).y);
#if 0 /*better but slower*/
        QLineF s1(tempPoints.at(nn).lon,tempPoints.at(nn).lat,tempPoints.at(nn+1).lon,tempPoints.at(nn+1).lat);
        if(S1.length()>tempPoints.at(nn).distIso)
            continue;
        if(checkCoast && getMap() && getMap()->crossing(S1,s1))
            continue;
        if(checkLine && crossBarriere(S1))
            continue;
#endif
        bool bad=false;
        for(int mm=0;mm<previousSegments.count();++mm)
        {
            if(S1.intersect(previousSegments.at(mm),&dummy)==QLineF::BoundedIntersection)
            {
                bad=true;
                somethingHasChanged=true;
                if(tempPoints.at(nn).distArrival>tempPoints.at(nn+1).distArrival)
                    tempPoints.removeAt(nn);
                else
                    tempPoints.removeAt(nn+1);
                --nn;
                break;
            }
        }
        if(bad) continue;
#if 1
        for(int mm=0;mm<previousIso.count()-1;++mm) /*also check that new Iso does not cross previous iso*/
        {
            QLineF S2(previousIso.at(mm),previousIso.at(mm+1));
            if(S1.intersect(S2,&dummy)==QLineF::BoundedIntersection)
//                    if(fastIntersects(S1,S2))
            {
                if(tempPoints.at(nn).distIso<tempPoints.at(nn+1).distIso)
                    tempPoints.removeAt(nn);
                else
                    tempPoints.removeAt(nn+1);
                --nn;
                break;
            }
        }
#endif
    }
}
void ROUTAGE::epuration(int toBeRemoved)
{
    QList<vlmPoint> rightFromLoxo;
    QList<vlmPoint> leftFromLoxo;
    if(tempPoints.count()<=1) return;
#if 0
    bool rightSide=true;
    for(int n=0;n<tempPoints.count();++n)
    {
        if(rightSide)
        {
            Triangle test(Point(xs,ys),Point(xa,ya),Point(tempPoints.at(n).x,tempPoints.at(n).y));
            if(test.orientation()==left_turn)
            {
                tempPoints[n].debugInt=1;
                rightFromLoxo.append(tempPoints.at(n));
            }
            else
            {
                rightSide=false;
                tempPoints[n].debugInt=2;
                leftFromLoxo.append(tempPoints.at(n));
            }
        }
        else
            leftFromLoxo.append(tempPoints.at(n));
    }
#else
    QPolygonF isoShape;
    for(int n=0;n<tempPoints.count();++n)
    {
        isoShape.append(QPointF(tempPoints.at(n).x,tempPoints.at(n).y));
    }
    isoShape.append(QPointF(tempPoints.first().x,tempPoints.first().y));
    QRectF bounding=isoShape.boundingRect().normalized();
    bool differentDirection=bounding.contains(xa,ya);
    QLineF separation;
    QLineF tempLine(bounding.center(),QPointF(xa,ya));
    if(differentDirection)
        tempLine.setP1(QPointF(xs,ys));
    tempLine.setLength(tempLine.length()*10);
    separation=QLineF(tempLine.p2(),tempLine.p1());
    separation.setLength(separation.length()*10);
    bool rightSide=true;
    for(int n=0;n<tempPoints.count();++n)
    {
        if(rightSide)
        {
            Triangle test(Point(separation.x2(),separation.y2()),
                          Point(separation.x1(),separation.y1()),
                          Point(tempPoints.at(n).x,tempPoints.at(n).y));
            if(test.orientation()==left_turn)
            {
                if(differentDirection)
                    tempPoints[n].debugInt=3;
                else
                    tempPoints[n].debugInt=1;
                rightFromLoxo.append(tempPoints.at(n));
            }
            else
            {
                rightSide=false;
                if(differentDirection)
                    tempPoints[n].debugInt=4;
                else
                    tempPoints[n].debugInt=2;
                leftFromLoxo.append(tempPoints.at(n));
            }
        }
        else
        {
            if(differentDirection)
                tempPoints[n].debugInt=4;
            else
                tempPoints[n].debugInt=2;
            leftFromLoxo.append(tempPoints.at(n));
        }
    }
#endif
    int toBeRemovedRight=0;
    int toBeRemovedLeft=0;
    int balance=qAbs(leftFromLoxo.count()-rightFromLoxo.count());
    if(leftFromLoxo.count()==rightFromLoxo.count())
    {
        toBeRemovedRight=toBeRemoved/2;
        toBeRemovedLeft=toBeRemoved-toBeRemovedRight;
    }
    else if(leftFromLoxo.count()>rightFromLoxo.count())
    {
        if(balance>=toBeRemoved)
        {
            toBeRemovedLeft=toBeRemoved;
            toBeRemovedRight=0;
        }
        else
        {
            toBeRemovedLeft=balance+(toBeRemoved-balance)/2;
            toBeRemovedRight=toBeRemoved-toBeRemovedLeft;
        }
    }
    else
    {
        if(balance>=toBeRemoved)
        {
            toBeRemovedRight=toBeRemoved;
            toBeRemovedLeft=0;
        }
        else
        {
            toBeRemovedRight=balance+(toBeRemoved-balance)/2;
            toBeRemovedLeft=toBeRemoved-toBeRemovedRight;
        }
    }
    if(leftFromLoxo.count()>0)
    {
        leftFromLoxo[0].internal_1=toBeRemovedLeft;
        leftFromLoxo[0].internal_2=initialDist;
    }
    if(rightFromLoxo.count()>0)
    {
        rightFromLoxo[0].internal_1=toBeRemovedRight;
        rightFromLoxo[0].internal_2=initialDist;
    }
    if(this->useMultiThreading)
    {
        QList<QList<vlmPoint> > listList;
        listList.append(rightFromLoxo);
        listList.append(leftFromLoxo);
        listList = QtConcurrent::blockingMapped(listList, finalEpuration);
        tempPoints.clear();
        tempPoints.reserve(listList.at(0).count()+listList.at(1).count());
        tempPoints.append(listList.at(0));
        tempPoints.append(listList.at(1));
    }
    else
    {
        leftFromLoxo=finalEpuration(leftFromLoxo);
        rightFromLoxo=finalEpuration(rightFromLoxo);
        tempPoints.clear();
        tempPoints.reserve(rightFromLoxo.count()+leftFromLoxo.count());
        tempPoints.append(rightFromLoxo);
        tempPoints.append(leftFromLoxo);
    }
}
void ROUTAGE::removeCrossedSegments()
{
    if(tempPoints.isEmpty()) return;
    QMultiMap<double,QPoint> byCriteres;
    QHash<quint32,double> byIndices;
    QList<bool> deadStatus;
    quint32 s;
    double critere=0;
    for(int n=0;n<tempPoints.size()-1;++n)
    {
        bool differentDirection=false;
        if(Util::myDiffAngle(tempPoints.at(n).capArrival,tempPoints.at(n+1).capArrival)>60)
        {
            if(tempPoints.at(n).originNb!=tempPoints.at(n+1).originNb)
            {
                QLineF temp1(tempPoints.at(n).origin->x,tempPoints.at(n).origin->y,
                             tempPoints.at(n).x,tempPoints.at(n).y);
                QLineF temp2(tempPoints.at(n+1).origin->x,tempPoints.at(n+1).origin->y,
                             tempPoints.at(n+1).x,tempPoints.at(n+1).y);
                QPointF dummy;
                if(temp1.intersect(temp2,&dummy)!=QLineF::BoundedIntersection)
                    differentDirection=true;
            }
        }
        if(differentDirection)
            critere=0;
        else
        {
            QLineF temp1(tempPoints.at(n).origin->x,tempPoints.at(n).origin->y,
                         tempPoints.at(n+1).origin->x,tempPoints.at(n+1).origin->y);
            QPointF middle=temp1.pointAt(0.5);
            QLineF temp2(middle.x(),middle.y(),tempPoints.at(n).x,tempPoints.at(n).y);
            QLineF temp3(middle.x(),middle.y(),tempPoints.at(n+1).x,tempPoints.at(n+1).y);
            ++debugCross0;
            critere=temp2.angleTo(temp3);
            if(critere<0) critere+=360;
        }
        byCriteres.insert(critere,QPoint(n,n+1));
        s=n*10e6+n+1;
        byIndices.insert(s,critere);
        deadStatus.append(false);
    }
    deadStatus.append(false);
    QMutableMapIterator<double,QPoint> d(byCriteres);
    int currentCount=tempPoints.count();
    while(currentCount>0)
    {
        d.toBack();
        if(!d.hasPrevious()) break;
        d.previous();
        if(d.key()<180) break;
        QPoint couple=d.value();
        int badOne=0;
        if(tempPoints.at(couple.x()).distIso<tempPoints.at(couple.y()).distIso)
            badOne=couple.x();
        else
            badOne=couple.y();
        deadStatus.replace(badOne,true);
        int previous=-1;
        int next=-1;
        previous=deadStatus.lastIndexOf(false,badOne);
        next=deadStatus.indexOf(false,badOne);
        if(currentCount<=1) break;
        if(previous!=-1 && next!=-1)
        {
            s=previous*10e6+badOne;
            double criterePrevious=byIndices.value(s);
            s=badOne*10e6+next;
            double critereNext=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
            byCriteres.remove(critereNext,QPoint(badOne,next));


            bool differentDirection=false;
            if(Util::myDiffAngle(tempPoints.at(previous).capArrival,tempPoints.at(next).capArrival)>60)
            {
                if(tempPoints.at(previous).originNb!=tempPoints.at(next).originNb)
                {
                    QLineF temp1(tempPoints.at(previous).origin->x,tempPoints.at(previous).origin->y,
                                 tempPoints.at(previous).x,tempPoints.at(previous).y);
                    QLineF temp2(tempPoints.at(next).origin->x,tempPoints.at(next).origin->y,
                                 tempPoints.at(next).x,tempPoints.at(next).y);
                    QPointF dummy;
                    if(temp1.intersect(temp2,&dummy)!=QLineF::BoundedIntersection)
                        differentDirection=true;
                }
            }
            if(differentDirection)
                critere=0;
            else
            {
                QLineF temp1(tempPoints.at(previous).origin->x,tempPoints.at(previous).origin->y,
                             tempPoints.at(next).origin->x,tempPoints.at(next).origin->y);
                QPointF middle=temp1.pointAt(0.5);
                QLineF temp2(middle.x(),middle.y(),tempPoints.at(previous).x,tempPoints.at(previous).y);
                QLineF temp3(middle.x(),middle.y(),tempPoints.at(next).x,tempPoints.at(next).y);
                critere=temp2.angleTo(temp3);
                if(critere<0) critere+=360;
            }


            byCriteres.insert(critere,QPoint(previous,next));
            s=previous*10e6+next;
            byIndices.insert(s,critere);
        }
        else if(previous==-1)
        {
            s=badOne*10e6+next;
            double critereNext=byIndices.value(s);
            byCriteres.remove(critereNext,QPoint(badOne,next));
        }
        else if(next==-1)
        {
            s=previous*10e6+badOne;
            double criterePrevious=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
        }
        --currentCount;
    }
    for (int nn=deadStatus.count()-1;nn>=0;--nn)
    {
        if(deadStatus.at(nn))
            tempPoints.removeAt(nn);
    }
}
void ROUTAGE::calculateCaps(QList<double> *caps, const vlmPoint &point, const double &workAngleStep, const double &workAngleRange)
{
    for(double cc=0;true;cc=cc+workAngleStep)
    {
        if(cc>workAngleRange/2.0)
            cc=workAngleRange/2;
        caps->append(Util::A360(point.capArrival-cc));
        if(cc!=0)
            caps->prepend(Util::A360(point.capArrival+cc));
        if(cc>=workAngleRange/2.0) break;
    }
#if 1
    Point vmg(cos(degToRad(point.capVmg)),sin(degToRad(point.capVmg)));
    Point O(0,0);
    for (int n=0;n<caps->count();++n)
    {
        Point C(cos(degToRad(caps->at(n))),sin(degToRad(caps->at(n))));
        Triangle T(O,C,vmg);
        if(T.orientation()==left_turn)
        {
            caps->insert(n,point.capVmg);
            break;
        }
    }
#endif
    return;
}
void ROUTAGE::showContextMenu(const int &isoNb,const int &pointNb)
{
    if(isoNb>=isochrones.count()) return;
    pivotPoint=isochrones.at(isoNb)->getPoints()->at(pointNb);
    popup->exec(QCursor::pos());
}

void ROUTAGE::slot_createPivot()
{
    //qWarning()<<"creating pivot..";
    if(way->getPoints()->isEmpty() || !way->isVisible())
        slot_drawWay();
    parent->addPivot(this);
}
void ROUTAGE::slot_createPivotM()
{
    //qWarning()<<"creating pivotM..";
    if(way->getPoints()->isEmpty()||!way->isVisible())
        slot_drawWay();
    parent->addPivot(this,true);
}
void ROUTAGE::setFromRoutage(ROUTAGE *fromRoutage, bool editOptions)
{
    this->fromRoutage=fromRoutage;
    this->width=fromRoutage->getWidth();
    this->poiPrefix=fromRoutage->getPoiPrefix();
    pivotPoint=fromRoutage->getPivotPoint();
    this->myBoat=fromRoutage->getBoat();
    this->startTime= startTime.fromTime_t(pivotPoint.eta);
    this->eta=startTime.toTime_t();
    this->whatIfDate=fromRoutage->getWhatIfDate();
    this->whatIfUsed=fromRoutage->getWhatIfUsed();
    this->whatIfTime=fromRoutage->getWhatIfTime();
    this->whatIfWind=fromRoutage->getWhatIfWind();
    this->angleRange=fromRoutage->getAngleRange();
    this->speedLossOnTack=fromRoutage->getSpeedLossOnTack();
    this->angleStep=fromRoutage->getAngleStep();
    this->timeStepMore24=fromRoutage->getTimeStepMore24();
    this->timeStepLess24=fromRoutage->getTimeStepLess24();
    this->explo=fromRoutage->getExplo();
    this->wind_angle=fromRoutage->getWindAngle();
    this->wind_speed=fromRoutage->getWindSpeed();
    this->windIsForced=fromRoutage->getWindIsForced();
    this->useRouteModule=fromRoutage->getUseRouteModule();
    this->checkCoast=fromRoutage->getCheckCoast();
    this->checkLine=fromRoutage->getCheckLine();
    this->useConverge=fromRoutage->useConverge;
    this->pruneWakeAngle=fromRoutage->pruneWakeAngle;
    this->colorGrib=fromRoutage->colorGrib;
    this->routeFromBoat=false;
    this->fromPOI=fromRoutage->getFromPOI();
    this->toPOI=fromRoutage->getToPOI();
    this->autoZoom=fromRoutage->getAutoZoom();
    this->zoomLevel=fromRoutage->getZoomLevel();
    this->minPortant=fromRoutage->getMinPortant();
    this->minPres=fromRoutage->getMinPres();
    this->maxPres=fromRoutage->getMaxPres();
    this->maxPortant=fromRoutage->getMaxPortant();
    isPivot=true;
    ROUTAGE *parentRoutage=this;
    result->deleteAll();
    QList<vlmPoint> initialRoad;
    while(true)
    {
        QList<vlmPoint> parentWay=*parentRoutage->getFromRoutage()->getWay()->getPoints();
        for(int n=1;n<parentWay.count();++n)
        {
            vlmPoint p=parentWay.at(n);
            if(n==parentWay.count()-1)
                p.notSimplificable=true;
            initialRoad.append(p);
        }
        if(!parentRoutage->getFromRoutage() || !parentRoutage->getFromRoutage()->getIsPivot())
        {
            break;
        }
        parentRoutage=parentRoutage->getFromRoutage();
    }
    pivotPoint.notSimplificable=true;
    initialRoad.prepend(pivotPoint);
    for(int n=0;n<initialRoad.count();++n)
    {
        vlmPoint p=initialRoad.at(n);
        p.isBroken=false;
        result->addVlmPoint(p);;
    }
//    result->addVlmPoint(pivotPoint);
    QPen penResult;
    penResult.setColor(color);
    penResult.setBrush(color);
    penResult.setWidthF(this->width);
    result->setLinePen(penResult);
    result->slot_showMe();
    fromRoutage->setShowIso(false);
    fromRoutage->getResult()->hide();
    fromRoutage->getWay()->hide();
    if(editOptions)
    {
        isNewPivot=true;
        emit editMe(this);
    }
    else
        this->calculate();
}
void ROUTAGE::createPopupMenu()
{
    popup = new QMenu(parent);

    ac_pivot = new QAction(tr("Creer un pivot"),popup);
    popup->addAction(ac_pivot);
    ac_pivotM = new QAction(tr("Creer un pivot en changeant les options"),popup);
    popup->addAction(ac_pivotM);
    connect(ac_pivot,SIGNAL(triggered()),this,SLOT(slot_createPivot()));
    connect(ac_pivotM,SIGNAL(triggered()),this,SLOT(slot_createPivotM()));
}
const double ROUTAGE::getTimeStep() const
{
    double step;
    if(!i_iso)
    {
        if(arrived) step = this->timeStepLess24;
        if(approaching || this->eta-this->startTime.toTime_t()<=24*60*60)
            step = this->timeStepLess24;
        else
            step = this->timeStepMore24;
        if(!isPivot && this->isochrones.count()==1)
            step-=myBoat->getVacLen()/60.0;
    }
    else
    {
        if(i_isochrones.count()>=isochrones.count()-1 || i_isochrones.count()<=1)
            step =  timeStepLess24;
        int inc=arrived?1:0;
        step = qAbs((result->getPoints()->at(i_isochrones.count()+inc).eta-
                result->getPoints()->at(i_isochrones.count()+inc-1).eta)/60.0);
    }
    return step;
}
void ROUTAGE::calculateInverse()
{
    i_iso=true;
    //qWarning()<<"launching inversed iso calculation";
    QColor col=Qt::black;
    QPen Pen=pen;
    col.setAlpha(160);
    Pen.setColor(col);
    Pen.setBrush(col);
    foreach(vlmLine *isoc,isochrones)
        isoc->setLinePen(Pen);
    foreach(vlmLine* seg,segments)
        seg->setHidden(true);
    this->calculate();
}
void ROUTAGE::showIsoRoute()
{
    if(this->isConverted()) return;
    deleteAlternative();
    QColor colorCloud=QColor(Qt::lightGray);
    colorCloud.setAlpha(150);
    double goal=(double)(timeStepMore24-isoRouteValue)/(double)timeStepMore24;
    double goalInc=0.2;
    goal-=goalInc;
    while (true)
    {
        goal=qMax(0.0,goal+goalInc);
        if(goal>.99) break;
        //if(goal>.1) break;
        int i=isochrones.count();
        int ii=0;
        QList<vlmPoint> left,right;
        vlmPoint lastInResult=result->getPoints()->last();
        for(int n=arrived?1:0;n<result->count()-1;++n)
        {
            if(result->getPoints()->at(n).isStart)  /*to deal with pivot*/
            {
                lastInResult=result->getPoints()->at(n);
                break;
            }
            --i;
            if(i<1) break;
            ++ii;
            if(ii>=i_isochrones.count()) break;
            //qWarning()<<i<<ii<<isochrones.count()<<i_isochrones.count();
            vlmLine * isochrone=isochrones.at(i);
            vlmLine * i_isochrone=i_isochrones.at(ii);
//            qWarning()<<QDateTime().fromTime_t(isochrone->getPoints()->first().eta).toUTC().toString("dd MMM-hh:mm")<<
//                        QDateTime().fromTime_t(i_isochrone->getPoints()->first().eta).toUTC().toString("dd MMM-hh:mm");
            int indice=isochrone->getPoints()->indexOf(result->getPoints()->at(n));
            if(indice<0)
            {
                qWarning()<<"erreur SIR 1";
                return;
            }
            bool found=false;
            vlmPoint Cross;
            double lon,lat,X,Y;
            Cross=result->getPoints()->at(n);
            proj->map2screenDouble(Util::cLFA(Cross.lon,proj->getXmin()),Cross.lat,&X,&Y);
            int js=0;
            double minDist=10e10;
            for(int s=indice;s<isochrone->getPoints()->count()-1;++s)
            {
                vlmPoint p1=isochrone->getPoints()->at(s);
                vlmPoint p2=isochrone->getPoints()->at(s+1);
                double x1,y1,x2,y2; /*recalculation necessary because zoom has changed*/
                proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                proj->map2screenDouble(Util::cLFA(p2.lon,proj->getXmin()),p2.lat,&x2,&y2);
                QLineF line1(x1,y1,x2,y2);
                for(int is=0;is<i_isochrone->getPoints()->count()-1;++is)
                {
                    vlmPoint ip1=i_isochrone->getPoints()->at(is);
                    vlmPoint ip2=i_isochrone->getPoints()->at(is+1);
                    QLineF line2(ip1.x,ip1.y,ip2.x,ip2.y);
                    QPointF cross;
                    if(line1.intersect(line2,&cross)==QLineF::BoundedIntersection)
                    {
                        QLineF line3(X,Y,cross.x(),cross.y());
                        if(line3.length()<minDist)
                        {
                            minDist=line3.length();
                            proj->screen2mapDouble(cross.x(),cross.y(),&lon,&lat);
                            Cross.lon=lon;
                            Cross.lat=lat;
                            Cross.x=cross.x();
                            Cross.y=cross.y();
                            found=true;
                            js=qMax(0,is-1);
                        }
                        //break;
                    }
                }
                //if(found) break;
            }
            if(qRound(goal*10000)!=0 && found)
            {
                Cross=result->getPoints()->at(n);
                vlmLine * prev_isochrone=isochrones.at(i-1);
                QPolygonF poly,prev_poly,i_poly;
                double  x1,y1;
                for(int s=indice;s<isochrone->getPoints()->count();++s)
                {
                    vlmPoint p1=isochrone->getPoints()->at(s);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    poly.append(QPointF(x1,y1));
                }
                int indicePrev=prev_isochrone->getPoints()->indexOf(result->getPoints()->at(n+1));
                if(indicePrev<0)
                {
                    qWarning()<<"erreur SIR 2";
                    return;
                }
                for(int s=indicePrev;s<prev_isochrone->getPoints()->count();++s)
                {
                    vlmPoint p1=prev_isochrone->getPoints()->at(s);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    prev_poly.append(QPointF(x1,y1));
                }
                for(int s=js;s<i_isochrone->count();++s)
                {
                    vlmPoint p1=i_isochrone->getPoints()->at(s);
                    if(p1.isBroken) break;
                    double x1,y1; /*recalculation necessary because zoom has changed*/
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    i_poly.append(QPointF(x1,y1));
                }
#if 1
                for (int rrr=0;rrr<result->count()-1;++rrr)
                {
                    double x2,y2;
                    vlmPoint p1=result->getPoints()->at(rrr);
                    vlmPoint p2=result->getPoints()->at(rrr+1);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    proj->map2screenDouble(Util::cLFA(p2.lon,proj->getXmin()),p2.lat,&x2,&y2);
                    QLineF rLine(x1,y1,x2,y2);
                    found=false;
                    for(int pp=0;pp<i_poly.count()-1;++pp)
                    {
                        QPointF dummy;
                        QLineF iLine(i_poly.at(pp),i_poly.at(pp+1));
                        if(rLine.intersect(iLine,&dummy)==QLineF::BoundedIntersection)
                        {
                            while(i_poly.count()>pp+2)
                                i_poly.remove(i_poly.count()-1);
                            found=true;
                            break;
                        }
                    }
                    if(found) break;
                }
#endif
                QPen pendebug(Qt::blue);
                pendebug.setWidthF(5);
#if 0
                vlmLine * debug1=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug1->setParent(this);
                debug1->setLinePen(pendebug);
                foreach (QPointF pp,poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug1->addPoint(lat,lon);
                }
                debug1->slot_showMe();
#endif
#if 0
                vlmLine * debug2=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug2->setParent(this);
                pendebug.setColor(Qt::white);
                debug2->setLinePen(pendebug);
                foreach (QPointF pp,i_poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug2->addPoint(lat,lon);
                }
                debug2->slot_showMe();
#endif
#if 0
                vlmLine * debug3=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug3->setParent(this);
                pendebug.setColor(Qt::magenta);
                debug3->setLinePen(pendebug);
                foreach (QPointF pp,prev_poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug3->addPoint(lat,lon);
                }
                debug3->slot_showMe();
#endif
                double root=0.5;
                double precision=0.01;
                if(newtownRaphson(&root,goal,precision,&poly,&prev_poly,&i_poly))
                {
                    QPointF cross=pointAt(&i_poly,root);
                    proj->screen2mapDouble(cross.x(),cross.y(),&lon,&lat);
                    Cross.lon=lon;
                    Cross.lat=lat;
                    Cross.x=cross.x();
                    Cross.y=cross.y();
                    left.append(Cross);
                }
                /*if not found let it draw a line hoping that next i_iso will be ok*/
            }
            else /*if (found)*/
                left.append(Cross);
            js=i_isochrone->getPoints()->count()-1;
            Cross=result->getPoints()->at(n);
            proj->map2screenDouble(Util::cLFA(Cross.lon,proj->getXmin()),Cross.lat,&X,&Y);
            found=false;
            minDist=10e10;
            for(int s=indice;s>0;--s)
            {
                vlmPoint p1=isochrone->getPoints()->at(s);
                vlmPoint p2=isochrone->getPoints()->at(s-1);
                double x1,y1,x2,y2; /*recalculation necessary because zoom has changed*/
                proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                proj->map2screenDouble(Util::cLFA(p2.lon,proj->getXmin()),p2.lat,&x2,&y2);
                QLineF line1(x1,y1,x2,y2);
                for(int is=i_isochrone->getPoints()->count()-1;is>0;--is)
                {
                    vlmPoint ip1=i_isochrone->getPoints()->at(is);
                    vlmPoint ip2=i_isochrone->getPoints()->at(is-1);
                    QLineF line2(ip1.x,ip1.y,ip2.x,ip2.y);
                    QPointF cross;
                    if(line1.intersect(line2,&cross)==QLineF::BoundedIntersection)
                    {
                        QLineF line3(X,Y,cross.x(),cross.y());
                        if(line3.length()<minDist)
                        {
                            minDist=line3.length();
                            proj->screen2mapDouble(cross.x(),cross.y(),&lon,&lat);
                            Cross.lon=lon;
                            Cross.lat=lat;
                            Cross.x=cross.x();
                            Cross.y=cross.y();
                            found=true;
                            js=qMax(0,is);
                        }
                        //break;
                    }
                }
                //if(found) break;
            }
            if(qRound(goal*10000)!=0 && found)
            {
                Cross=result->getPoints()->at(n);
                vlmLine * prev_isochrone=isochrones.at(i-1);
                QPolygonF poly,prev_poly,i_poly;
                double  x1,y1;
                for(int s=indice;s>=0;--s)
                {
                    vlmPoint p1=isochrone->getPoints()->at(s);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    poly.append(QPointF(x1,y1));
                }
                int indicePrev=prev_isochrone->getPoints()->indexOf(result->getPoints()->at(n+1));
                if(indicePrev<0)
                {
                    qWarning()<<"erreur SIR 2-2";
                    return;
                }
                for(int s=indicePrev;s>=0;--s)
                {
                    vlmPoint p1=prev_isochrone->getPoints()->at(s);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    prev_poly.append(QPointF(x1,y1));
                }
#if 1
                int intersection=i_isochrone->getPoints()->count()-1;
                for (int rrr=0;rrr<result->count()-1;++rrr)
                {
                    double x2,y2;
                    vlmPoint p1=result->getPoints()->at(rrr);
                    vlmPoint p2=result->getPoints()->at(rrr+1);
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    proj->map2screenDouble(Util::cLFA(p2.lon,proj->getXmin()),p2.lat,&x2,&y2);
                    QLineF rLine(x1,y1,x2,y2);
                    found=false;
                    for(int pp=0;pp<i_isochrone->getPoints()->count()-1;++pp)
                    {
                        QPointF dummy;
                        p1=i_isochrone->getPoints()->at(pp);
                        p2=i_isochrone->getPoints()->at(pp+1);
                        proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                        proj->map2screenDouble(Util::cLFA(p2.lon,proj->getXmin()),p2.lat,&x2,&y2);
                        QLineF iLine(x1,y1,x2,y2);
                        if(rLine.intersect(iLine,&dummy)==QLineF::BoundedIntersection)
                        {
                            intersection=pp;
                            found=true;
                            break;
                        }
                    }
                    if(found) break;
                }
                if(!found)
                    qWarning()<<"i_poly does NOT cross result(right)!!!!!";
#endif
                for(int s=js;s>=0;--s)
                {
                    vlmPoint p1=i_isochrone->getPoints()->at(s);
                    if(s<intersection) break;
                    if(p1.isBroken)
                    {
                        i_poly.clear();
                    }
                    double x1,y1; /*recalculation necessary because zoom has changed*/
                    proj->map2screenDouble(Util::cLFA(p1.lon,proj->getXmin()),p1.lat,&x1,&y1);
                    i_poly.append(QPointF(x1,y1));
                }
                QPen pendebug(Qt::blue);
                pendebug.setWidthF(5);
#if 0
                if(n==39)
                    pendebug.setColor(Qt::magenta);
                vlmLine * debug1=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug1->setParent(this);
                debug1->setLinePen(pendebug);
                foreach (QPointF pp,poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug1->addPoint(lat,lon);
                }
                debug1->slot_showMe();
#endif
#if 0
                vlmLine * debug2=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug2->setParent(this);
                pendebug.setColor(Qt::white);
                debug2->setLinePen(pendebug);
                foreach (QPointF pp,i_poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug2->addPoint(lat,lon);
                }
                debug2->slot_showMe();
#endif
#if 0
                vlmLine * debug3=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE+10);
                debug3->setParent(this);
                pendebug.setColor(Qt::magenta);
                debug3->setLinePen(pendebug);
                foreach (QPointF pp,prev_poly)
                {
                    proj->screen2mapDouble(pp.x(),pp.y(),&lon,&lat);
                    debug3->addPoint(lat,lon);
                }
                debug3->slot_showMe();
#endif
                double root=0.5;
                double precision=0.01;
                if(newtownRaphson(&root,goal,precision,&poly,&prev_poly,&i_poly))
                {
                    QPointF cross=pointAt(&i_poly,root);
                    proj->screen2mapDouble(cross.x(),cross.y(),&lon,&lat);
                    Cross.lon=lon;
                    Cross.lat=lat;
                    Cross.x=cross.x();
                    Cross.y=cross.y();
                    right.prepend(Cross);
                }
                /*if not found let it draw a line hoping that next i_iso will be ok*/
            }
            else /*if (found)*/
                right.prepend(Cross);
        }
        right.prepend(lastInResult);
        right.append(result->getPoints()->first());
        left.append(right);
        //qWarning()<<"SIR: isoRoute has "<<left.count()<<"points";
        vlmLine * isoRoute=new vlmLine(proj, this->myscene,Z_VALUE_ROUTAGE);
        isoRoute->setParent(this);
        for(int n=0;n<left.count();++n)
            isoRoute->addVlmPoint(left.at(n));
        colorCloud=colorCloud.darker(110);
        QPen Pen(Qt::NoPen);
        Pen.setWidthF(width);
        Pen.setBrush(QBrush(colorCloud));
        isoRoute->setLinePen(Pen);
        isoRoute->setSolid(true);
        isoRoute->slot_showMe();
        isoRoutes.append(isoRoute);
    }
}
QPointF ROUTAGE::pointAt(const QPolygonF * poly,const double ratio)

{
    if(poly->isEmpty())
        qWarning()<<"erreur in pointAt()";
    if(poly->count()==1)
        return poly->first();
    if(ratio>=1)
        return poly->last();
    if(ratio<=0)
        return poly->first();
    QPointF resultPoint;
    double fullLength=0;
    for(int n=0;n<poly->size()-1;++n)
        fullLength+=QLineF(poly->at(n),poly->at(n+1)).length();
    double ratioLength=fullLength*ratio;
    for(int n=0;n<poly->size()-1;++n)
    {
        QLineF line(poly->at(n),poly->at(n+1));
        if(line.length()<ratioLength)
        {
            ratioLength-=line.length();
            continue;
        }
        resultPoint=line.pointAt(ratioLength/line.length());
        break;
    }
    return resultPoint;
}
double ROUTAGE::pointDistanceRatio(double x, double goal, QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly)
{
    if(i_poly->isEmpty())
        qWarning()<<"i_poly is empty";
    if(poly->isEmpty())
        qWarning()<<"poly is empty";
    if(prev_poly->isEmpty())
        qWarning()<<"prev_poly is empty";
    if(i_poly->isEmpty() || poly->isEmpty() || prev_poly->isEmpty())
        return 10e6;
    QPointF point=pointAt(i_poly,x);
#if 0
//    qWarning()<<"i_poly counts"<<i_poly->count();
//    qWarning()<<"poly counts"<<poly->count();
//    qWarning()<<"prev_poly counts"<<prev_poly->count();
    double angle=QLineF(poly->first(),prev_poly->first()).angle();
    QLineF cutter(point,poly->first());
    cutter.setLength(cutter.length()*100.0);
    cutter.setAngle(angle);
    cutter.setPoints(cutter.p2(),cutter.p1());
    cutter.setLength(cutter.length()*100.0);
    double dist1=10e6, dist2=10e3;
    QPointF dummy;
    for (int n=0;n<poly->count()-1;++n)
    {
        if(QLineF(poly->at(n),poly->at(n+1)).intersect(cutter,&dummy)==QLineF::BoundedIntersection)
        {
            dist1=QLineF(dummy,point).length();
            break;
        }
    }
    for (int n=0;n<prev_poly->count()-1;++n)
    {
        if(QLineF(prev_poly->at(n),prev_poly->at(n+1)).intersect(cutter,&dummy)==QLineF::BoundedIntersection)
        {
            dist2=QLineF(dummy,point).length();
            break;
        }
    }
#else
    QPointF dummy;
    double dist1=this->findDistancePoly(point,poly,&dummy);
    double dist2=this->findDistancePoly(point,prev_poly,&dummy);
#endif
    if(dist2==0) return -goal;
    if(dist1+dist2==0) return 10e6;
    return (dist1/(dist1+dist2))-goal;
}
double ROUTAGE::pointDistanceRatioDeriv(double x, double xStep, double goal, bool * status, QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly)
{
    double yr,yl;
    double xl,xr;
    double xMin=0.0;
    double xMax=1.0;
    xl=x-xStep;
    if(xl<xMin)
        xl=x;
    xr=x+xStep;
    if(xr>xMax)
        xr=x;
    if(xr==xl)
    {
        *status=false;
        //qWarning()<<"error in deriv:"<<xr<<x<<xStep;
        return 0;
    }
    yl=pointDistanceRatio(xl,goal,poly,prev_poly,i_poly);
    yr=pointDistanceRatio(xr,goal,poly,prev_poly,i_poly);
    *status=true;
    return((yr-yl)/(xr-xl));
}
bool ROUTAGE::newtownRaphson(double * root, double goal,double precision,QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly)
{
    double stepGap=1000000;
    double x0=0.5;
    double x1=0.5;
    double y0=0,y1=0;
    double xMin=0;
    double xMax=1;
    bool status=false;
    bool haveXneg=false;
    bool haveXpos=false;
    double xStep;
    double xPos=0,xNeg=0, yPos=0, yNeg=0;
    double df0=0;
    int iterations;
    double bestY=10e6,bestX=0;
    int nbRestart=-1;
    QList<double> alternativeGuesses;
    alternativeGuesses<<0.1<<0.2<<0.3<<0.4<<0.6<<0.7<<0.8<<0.9;
    for (iterations=1;iterations<200;++iterations)
    {
        if(x0<xMin)
            x0=xMin;
        if(x0>xMax)
            x0=xMax;
        y0=pointDistanceRatio(x0,goal,poly,prev_poly,i_poly);
        //qWarning()<<x0<<y0;
        if(qAbs(y0)<=precision)
        {
            *root=x0;
            //qWarning()<<"found it (lucky mode) in"<<iterations<<"loop(s)";
            return true;
        }
        if(qAbs(y0)<bestY)
        {
            bestY=qAbs(y0);
            bestX=x0;
        }

/*update data*/
        if(y0>0)
        {
            if(haveXpos)
            {
                if(haveXneg)
                {
                    if(qAbs(x0-xNeg)<qAbs(xPos-xNeg))
                    {
                        xPos=x0;
                        yPos=y0;
                    }
                }
                else if(y0<yPos)
                {
                    xPos=x0;
                    yPos=y0;
                }
            }
            else
            {
                xPos=x0;
                yPos=y0;
                haveXpos=true;
            }
            status=false;
        }
        else if(y0<0)
        {
            if(haveXneg)
            {
                if(haveXpos)
                {
                    if(qAbs(x0-xPos)<qAbs(xPos-xNeg))
                    {
                        xNeg=x0;
                        yNeg=y0;
                    }
                }
                else if(-y0<-yNeg)
                {
                    xNeg=x0;
                    yNeg=y0;
                }
            }
            else
            {
                xNeg=x0;
                yNeg=y0;
                haveXneg=true;
            }
            status=false;
        }
        else
        {
            *root=x0;
            status=true;
        }
/*end of update data*/
        if(status)
        {
            //qWarning()<<"found it in"<<iterations<<"loop(s)";
            return status;
        }
        if(y0==y1 && iterations!=1)
        {
            if(++nbRestart<8)
            {
                //x0=qrand()/(double)RAND_MAX;
                //qWarning()<<"restarting with a random number"<<x0;
                x0=alternativeGuesses[nbRestart];
                //qWarning()<<"restarting with alternate guess:"<<x0;
                continue;
            }
            //qWarning()<<"not found (flat case 1)";
            break;
        }
        y1=y0;
        while(true)
        {
            if(qAbs(x0)<.0000000001)
            {
                if(haveXneg && haveXpos)
                    xStep=qAbs(xPos-xNeg)/stepGap;
                else
                    xStep=(xMax-xMin)/stepGap;
            }
            else
                xStep=qAbs(x0)/stepGap;
            status=true;
            for (int idf=0;idf<5;++idf)
            {
                df0=pointDistanceRatioDeriv(x0,xStep,goal,&status,poly,prev_poly,i_poly);
                if(!status)
                {
                    //double debugXstep=xStep;
                    xStep=xStep/2.0;
                    //qWarning()<<"retrying deriv"<<debugXstep<<xStep;
                }
                else break;
            }
            //qWarning()<<"x0,xStep="<<x0<<xStep;
            if(!status)
            {
                //qWarning()<<"not found (anomaly in derivative function)";
                return status;
            }
            if(qRound(qAbs(df0*100000000))==0) /* hit a flat spot->trouble*/
            {
                if(stepGap>10.0)
                {
                    stepGap=stepGap/10.0;
                    continue;
                }
                else
                {
                    //qWarning()<<"not found (flat case 2)";
                    return false;
                }
            }
            else
                break;
        }
        /*Overshoot a bit to prevent from staying on just one side of root*/
        x1=x0-1.000001*y0/df0;
        double stepSize=qAbs(x1-x0)/qAbs(x0+x1);
        if(stepSize<precision)
        {
            *root=x0;
            //qWarning()<<"found it after"<<iterations<<"loop(s)";
            return true;
        }
        x0=x1;
    }
    if(bestY<precision*20.0 && bestX>=xMin && bestX<=xMax)
    {
        *root=bestX;
        //qWarning()<<"found something not so good but still usable:"<<bestY<<goal+bestY<<"instead of"<<goal;
        return true;
    }
    //qWarning()<<"not found after"<<iterations<<"loops. (Best find:"<<bestY<<goal+bestY<<"instead of"<<goal<<")";
    return false;
}
bool ROUTAGE::crossBarriere(const QLineF &line)
{
    QPointF dummy;
    foreach (QLineF barriere,barrieres)
    {
        if(barriere.intersect(line,&dummy)==QLineF::BoundedIntersection)
            return true;
    }
    return false;
}
void ROUTAGE::calculateAlternative()
{
    while(!this->alternateRoutes.isEmpty())
        delete alternateRoutes.takeFirst();
    Settings::setSetting("thresholdAlternative",thresholdAlternative);
    Settings::setSetting("nbAlternative",nbAlternative);
    if(nbAlternative==0) return;
    if(i_iso || !arrived) return;
    QMessageBox * waitBox = new QMessageBox(QMessageBox::Information,tr("Calcul des routes alternatives"),
                              tr("Veuillez patienter..."));
    waitBox->setStandardButtons(QMessageBox::NoButton);
    waitBox->show();
    QApplication::processEvents();
    vlmPoint to(this->toPOI->getLongitude(),this->toPOI->getLatitude());
    datathread dataThread;
    dataThread.Boat=this->getBoat();
    dataThread.Eta=this->getEta();
    dataThread.GriB=this->getGrib();
    dataThread.whatIfJour=this->getWhatIfJour();
    dataThread.whatIfUsed=this->getWhatIfUsed();
    dataThread.windIsForced=this->getWindIsForced();
    dataThread.whatIfTime=this->getWhatIfTime();
    dataThread.windAngle=this->getWindAngle();
    dataThread.windSpeed=this->getWindSpeed();
    dataThread.whatIfWind=this->getWhatIfWind();
    dataThread.timeStep=this->getTimeStep();
    dataThread.speedLossOnTack=this->getSpeedLossOnTack();
    dataThread.i_iso=i_iso;
    QList<vlmPoint> tempResult;
    for (int r=0;r<result->count();++r)
    {
        if(result->getPoints()->at(r).isStart) break;
        tempResult.append(result->getPoints()->at(r));
    }
    double optionThreshold=(double)thresholdAlternative/100.0;
    int limitNb=qRound((tempResult.count()-1)*optionThreshold);
    //qWarning()<<"(1) limitNb="<<limitNb<<"count="<<tempResult.count()<<optionThreshold;
    if(limitNb<0 || limitNb>=tempResult.count())
    {
        delete waitBox;
        return;
    }
    vlmPoint t=tempResult.at(limitNb);
    QList<vlmPoint> limits;
    limits.append(t);
    //qWarning()<<"Searching for alternative routes";
    QMultiMap<int,vlmPoint> alternateTimes;
    int i=qRound(((double)isochrones.count()-1)*.85);
    if(i<0 || i>=isochrones.count())
    {
        delete waitBox;
        return;
    }
    vlmLine *isoc=isochrones.at(i);
    for (int is=0;is<isoc->getPoints()->count();++is)
    {
        vlmPoint P=isoc->getPoints()->at(is);
        bool bad=false;
        while(true)
        {
            if(P.isStart) break;
            if(P==t)
            {
                bad=true;
                break;
            }
            P=*P.origin;
        }
        if(bad) continue;
        P=isoc->getPoints()->at(is);
        dataThread.Eta=P.eta;
        int thisTime=calculateTimeRoute(P,to,&dataThread, NULL, NULL);
        if(thisTime>10e4) continue;
        alternateTimes.insert(P.eta+thisTime,P);
    }
    //QMap::const_iterator<int,vlmPoint> times(alternateTimes);
    QMapIterator<int,vlmPoint> times(alternateTimes);
    while(times.hasNext())
    {
        times.next();
        vlmPoint P=times.value();
        vlmLine *res=new vlmLine(proj,parent->getScene(),Z_VALUE_ROUTAGE);
        bool bad=false;
        while(true)
        {
            res->addVlmPoint(P);
            if(P.isStart) break;
            if(limits.contains(P))
            {
                bad=true;
                break;
            }
            P=*P.origin;
        }
        if(bad)
        {
            delete res;
            continue;
        }
        alternateRoutes.append(res);
        QPen penA=pen;
        penA.setColor(Qt::white);
        res->setLinePen(penA);
        res->slot_showMe();
        QDateTime tm;
        tm.setTimeSpec(Qt::UTC);
        tm.setTime_t(times.key());
        QString tip=tr("Arrivee (estimation): ")+tm.toString("dd MMM-hh:mm");
        P=times.value();
        int ii=isoc->getPoints()->indexOf(P);
        emit updateVgTip(i,ii,tip);
        if(alternateRoutes.count()>=this->nbAlternative) break;
        limitNb=qRound((res->getPoints()->count()-1)*optionThreshold);
        //qWarning()<<"(2) limitNb="<<limitNb<<"count="<<res->getPoints()->count();
        if(limitNb<0 || limitNb>=res->getPoints()->count()) break;
        vlmPoint t=res->getPoints()->at(limitNb);
        limits.append(t);
    }
    QApplication::processEvents();
    delete waitBox;
}
