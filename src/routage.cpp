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
//#include "Terrain.h"

bool rightToLeftFromOrigin(const vlmPoint & P1,const vlmPoint & P2)
{
    Triangle triangle(Point(P1.origin->x,P1.origin->y),
                      Point(P1.x,P1.y),
                      Point(P2.x,P2.y));
    return triangle.orientation()==right_turn;
}

/*threadable functions*/

inline vlmPoint checkCoastCollision(const vlmPoint point)
{
    vlmPoint newPoint=point;
    double x1,y1,x2,y2;
    x1=newPoint.origin->x;
    y1=newPoint.origin->y;
    x2=newPoint.x;
    y2=newPoint.y;
    newPoint.isDead=newPoint.routage->getMap()->crossing(QLineF(x1,y1,x2,y2),QLineF(newPoint.origin->lon,newPoint.origin->lat,newPoint.lon,newPoint.lat));
    return newPoint;
}
inline QList<vlmPoint> finalEpuration(const QList<vlmPoint> &listPoints)
{
    if(listPoints.count()==0) return listPoints;
    int toBeRemoved=listPoints.at(0).internal_1;
    double initialDist=listPoints.at(0).internal_2;
    if(toBeRemoved<=0) return listPoints;
    QMultiMap<double,QPoint> byCriteres;
    QHash<QString,double> byIndices;
    QList<bool> deadStatus;
    QString s;
    for(int n=0;n<listPoints.size()-1;n++)
    {
#if 1
        QLineF temp1(listPoints.at(n).origin->x,listPoints.at(n).origin->y,
                     listPoints.at(n+1).origin->x,listPoints.at(n+1).origin->y);
        QPointF middle=temp1.pointAt(0.5);
        temp1.setPoints(QPointF(listPoints.at(n).x,listPoints.at(n).y),
                        QPointF(listPoints.at(n+1).x,listPoints.at(n+1).y));
        QPointF middleBis=temp1.pointAt(0.5);
        temp1.setPoints(middleBis,middle);
        temp1.setLength(initialDist);
        middle=temp1.p2();
        QLineF temp2(middle.x(),middle.y(),listPoints.at(n).x,listPoints.at(n).y);
        QLineF temp3(middle.x(),middle.y(),listPoints.at(n+1).x,listPoints.at(n+1).y);
        double critere=qAbs(temp2.angleTo(temp3));
        if(critere>180)
        {
            critere=360-critere;
        }
#else
        QLineF ecart(listPoints.at(n).x,listPoints.at(n).y,listPoints.at(n+1).x,listPoints.at(n+1).y);
        double critere=ecart.length();
#endif
        byCriteres.insert(critere,QPoint(n,n+1));
        s=s.sprintf("%d;%d",n,n+1);
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
        if(listPoints.at(couple.x()).distIso<listPoints.at(couple.y()).distIso)
            badOne=couple.x();
        else
            badOne=couple.y();
        deadStatus.replace(badOne,true);
        int previous=0;
        int next=0;
        previous=-1;
        previous=deadStatus.lastIndexOf(false,badOne);
        next=deadStatus.indexOf(false,badOne);
        if(currentCount<=1) break;
        if(previous!=-1 && next!=-1)
        {
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
            byCriteres.remove(critereNext,QPoint(badOne,next));
//            double length=QLineF(QPointF(listPoints.at(previous).lon,listPoints.at(previous).lat),QPointF(listPoints.at(next).lon,listPoints.at(next).lat)).length();
#if 1
            QLineF temp1(listPoints.at(previous).origin->x,listPoints.at(previous).origin->y,
                         listPoints.at(next).origin->x,listPoints.at(next).origin->y);
            QPointF middle=temp1.pointAt(0.5);
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
#else
            QLineF ecart(listPoints.at(previous).x,listPoints.at(previous).y,listPoints.at(next).x,listPoints.at(next).y);
            double critere=ecart.length();
#endif
            byCriteres.insert(critere,QPoint(previous,next));
            s=s.sprintf("%d;%d",previous,next);
            byIndices.insert(s,critere);
        }
        else if(previous==-1)
        {
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(critereNext,QPoint(badOne,next));
        }
        else if(next==-1)
        {
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
        }
        toBeRemoved--;
        currentCount--;
    }
    QList<vlmPoint> result;
    for (int nn=0;nn<deadStatus.count();nn++)
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
    dataThread.Eta=routage->getEta();
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
    QList<vlmPoint> resultList;
    for (int pp=0;pp<pointList.count();pp++)
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
        if(realTime==0)
        {
            resultP.isDead=true;
            resultList.append(resultP);
            continue;
        }
        int timeStep=point.routage->getTimeStep();
        int timeStepSec=timeStep*60;
        if(realTime==timeStepSec)
        {
            resultP.lon=lastLonFound;
            resultP.lat=lastLatFound;
            resultList.append(resultP);
            continue;
        }
        int minDiff=qAbs(realTime-timeStepSec);
        double bestDist=distanceParcourue;
        bool found=false;
        int n=1;
        int oldTime=timeStepSec;
        /*first, trying to be clever*/
        double newDist=distanceParcourue*oldTime/realTime;
        Util::getCoordFromDistanceAngle(lat, lon, newDist, cap, &res_lat, &res_lon);
        to.lon=res_lon;
        to.lat=res_lat;
        oldTime=realTime;
        realTime=ROUTAGE::calculateTimeRoute(from, to, &dataThread, &lastLonFound, &lastLatFound);
        if(realTime==timeStepSec)
        {
            resultP.distOrigin=newDist;
            found=true;
        }
        if(!found)
        {
            if(minDiff>qAbs(realTime-timeStepSec))
            {
                minDiff=qAbs(realTime-timeStepSec);
                bestDist=newDist;
            }
    #if 1 /*find it using Newtown-Raphson method*/
            double x=distanceParcourue;
            double term=0;
            from.capOrigin=cap;
            for (n=2;n<=21;n++) /*20 tries max*/
            {
                double y=ROUTAGE::routeFunction(x,from,&lastLonFound,&lastLatFound,&dataThread);
                if(qAbs(y)<=60)
                {
                    found=true;
                    resultP.distOrigin=x;
                    break;
                }
                double deriv=ROUTAGE::routeFunctionDeriv(x,from,&lastLonFound,&lastLatFound,&dataThread);
                if (deriv==0)
                {
                    bestDist=distanceParcourue;
                    break; /*flat spot, there is no solution*/
                }
                term=y/deriv;
                x=qAbs(x-term);
            }
    #endif
        }
        if(found)
        {
            resultP.lon=lastLonFound;
            resultP.lat=lastLatFound;
        }
        else
        {
            resultP.distOrigin=bestDist;
            Util::getCoordFromDistanceAngle(lat, lon, bestDist, cap, &res_lat, &res_lon);
            resultP.lon=res_lon;
            resultP.lat=res_lat;
            resultP.isDead=true; /*no route to point, better delete it(?)*/
        }
        resultList.append(resultP);
    }
    return resultList;
}
inline int ROUTAGE::routeFunction(double x,vlmPoint from, double * lastLonFound, double * lastLatFound, datathread * dataThread)
{
    double res_lon,res_lat;
    Util::getCoordFromDistanceAngle(from.lat, from.lon, x, from.capOrigin, &res_lat, &res_lon);
    vlmPoint to(res_lon,res_lat);
    return ROUTAGE::calculateTimeRoute(from,to,dataThread,lastLonFound,lastLatFound)-dataThread->timeStep*60;
}
inline int ROUTAGE::routeFunctionDeriv(double x,vlmPoint from, double * lastLonFound, double * lastLatFound,datathread *dataThread)
{
    double minGap=x/100;
    int   yr,yl;
    double xl,xr;
    for(int n=1;n<200;n++) /*bad trick to avoid flat spots*/
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
inline int ROUTAGE::calculateTimeRoute(vlmPoint routeFrom,vlmPoint routeTo, datathread * dataThread, double * lastLonFound, double * lastLatFound, int limit)
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
    Orthodromie orth2(0,0,0,0);
    double lon,lat;
    lon=routeFrom.lon;
    lat=routeFrom.lat;
    double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,previous_remaining_distance,cap1,cap2,diff1,diff2;
    double windAngle,windSpeed,cap,angle;
    time_t maxDate=dataThread->GriB->getMaxDate();
    newSpeed=0;
    distanceParcourue=0;
    res_lon=0;
    res_lat=0;
    previous_remaining_distance=0;
    windAngle=0;
    windSpeed=0;
    orth.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    orth2.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    remaining_distance=orth.getDistance();
    double initialDistance=orth2.getDistance();
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
                }
                if(dataThread->whatIfUsed && dataThread->whatIfJour<=dataThread->Eta)
                    windSpeed=windSpeed*dataThread->whatIfWind/100.00;
                previous_remaining_distance=remaining_distance;
                cap=orth.getAzimutDeg();
                angle=cap-windAngle;
                if(qAbs(angle)>180)
                {
                    if(angle<0)
                        angle=360+angle;
                    else
                        angle=angle-360;
                }
                if(qAbs(angle)<dataThread->Boat->getBvmgUp(windSpeed))
                {
                    angle=dataThread->Boat->getBvmgUp(windSpeed);
                    cap1=ROUTAGE::A360(windAngle+angle);
                    cap2=ROUTAGE::A360(windAngle-angle);
                    diff1=ROUTAGE::myDiffAngle(cap,cap1);
                    diff2=ROUTAGE::myDiffAngle(cap,cap2);
                    if(diff1<diff2)
                        cap=cap1;
                    else
                        cap=cap2;
                }
                else if(qAbs(angle)>dataThread->Boat->getBvmgDown(windSpeed))
                {
                    angle=dataThread->Boat->getBvmgDown(windSpeed);
                    cap1=ROUTAGE::A360(windAngle+angle);
                    cap2=ROUTAGE::A360(windAngle-angle);
                    diff1=ROUTAGE::myDiffAngle(cap,cap1);
                    diff2=ROUTAGE::myDiffAngle(cap,cap2);
                    if(diff1<diff2)
                        cap=cap1;
                    else
                        cap=cap2;
                }
                newSpeed=dataThread->Boat->getPolarData()->getSpeed(windSpeed,angle);
                if(!ignoreTackLoss && dataThread->speedLossOnTack!=1)
                {
                    if((angle>0 && lastTwa<0) || (angle<0 && lastTwa>0))
                        newSpeed=newSpeed*dataThread->speedLossOnTack;
                }
                else
                    ignoreTackLoss=false;
                lastTwa=angle;
                distanceParcourue=newSpeed*dataThread->Boat->getVacLen()/3600.00;
                Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                orth.setStartPoint(res_lon, res_lat);
                remaining_distance=orth.getDistance();
            }
            else
            {
                has_eta=false;
                break;
            }
            orth2.setEndPoint(res_lon,res_lat);
            if(orth2.getDistance()>=initialDistance)
            {
                lon=res_lon;
                lat=res_lat;
                etaRoute= etaRoute + dataThread->Boat->getVacLen();
                break;
            }
            lon=res_lon;
            lat=res_lat;
            etaRoute= etaRoute + dataThread->Boat->getVacLen();
        } while (has_eta);
    if(!has_eta)
    {
        if(hasLimit)
            return 10e5;
    }
    if(lastLonFound!=NULL)
    {
        *lastLonFound=lon;
        *lastLatFound=lat;
    }
    return(etaRoute-dataThread->Eta);
}

ROUTAGE::ROUTAGE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow)
        : QObject()
{
    this->proj=proj;
    this->name=name;
    this->myscene=myScene;
    this->isPivot=false;
    this->isNewPivot=false;
    this->autoZoom=true;
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
    colorsList.append(Qt::white);
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
    this->whatIfDate=startTime;
    this->whatIfUsed=false;
    this->whatIfTime=0;
    this->whatIfWind=100;
    pen.setColor(color);
    pen.setBrush(color);
    pen.setWidthF(2);
    this->angleRange=160;
    this->angleStep=3;
    this->timeStep=60;
    this->explo=5;
    this->wind_angle=0;
    this->wind_speed=20;
    this->windIsForced=false;
    this->showIso=true;
    this->done=false;
    this->converted=false;
    this->useRouteModule=true;
    this->finalEta=QDateTime();
    this->finalEta.setTimeSpec(Qt::UTC);
    result=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    result->setParent(this);
    way=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    way->setParent(this);
    this->checkCoast=true;
    this->useConverge=true;
    this->pruneWakeAngle=30;
    this->routeFromBoat=true;
    this->aborted=false;
    createPopupMenu();
    connect(parent,SIGNAL(stopCompassLine()),this,SLOT(slot_abort()));
    connect(parent,SIGNAL(updateRoutage()),this,SLOT(slot_gribDateChanged()));
    this->tempPoints.reserve(180*180);
    this->poiPrefix="R";
    this->speedLossOnTack=1;
    highlightedIso=0;
}
ROUTAGE::~ROUTAGE()
{
    //qWarning() << "Delete of routage: " << name;
    for (int n=0;n<isochrones.count();n++)
        delete isochrones[n];
    for (int n=0;n<segments.count();n++)
        delete segments[n];
    if(result!=NULL)
        delete result;
    for (int n=0;n<isoPointList.count();n++)
        delete isoPointList[n];
    delete way;
    if(this->popup && !parent->getAboutToQuit())
        delete popup;
}
void ROUTAGE::setBoat(boat *myBoat)
{
    this->myBoat=myBoat;
}
void ROUTAGE::setWidth(double width)
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
void ROUTAGE::setColor(QColor color)
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
    eta=startTime.toUTC().toTime_t();
    if ( eta>grib->getMaxDate() || eta<grib->getMinDate() )
    {
        QMessageBox::critical(0,tr("Routage"),tr("Date de depart choisie incoherente avec le grib"));
        return;
    }
    running=true;
    QTime timeTotal;
    QTime tfp;
    timeTotal.start();
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
    int maxLoop=0;
    int msecs_15=0;
    int nbCaps=0;
    int nbCapsPruned=0;
    msecsD1=0;
    msecsD2=0;
    debugCross0=0;
    debugCross1=0;
    QTime time,tDebug;
    QList<QColor> colorsList;
    colorsList.append(Qt::white);
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
    for (ncolor=0;ncolor<colorsList.count();ncolor++)
        colorsList[ncolor].setAlpha(160);
    ncolor=0;
    whatIfJour=whatIfDate.toUTC().toTime_t();
    Orthodromie orth(0,0,0,0);
    double   cap;
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
    orth.setPoints(start.x(),start.y(),arrival.x(),arrival.y());
    loxoCap=orth.getAzimutDeg();
    initialDist=orth.getDistance();
    if(autoZoom)
    {
        double xW=qMin(start.x(),arrival.x());
        double xE=qMax(start.x(),arrival.x());
        qWarning()<<xW<<xE;
        if((xW>0 && xE<0) || (xW<0 && xE>0))
        {
            if(qAbs(xW-xE)>180)
            {
                double xTemp=xW;
                xW=xE;
                xE=xTemp;
                if(xW>0)
                    xW-=360;
                else
                    xE-=360;

            }
        }
        qWarning()<<xW<<xE;
        double yN=qMax(start.y(),arrival.y());
        double yS=qMin(start.y(),arrival.y());
        proj->setUseTempo(false);
        proj->blockSignals(true);
        proj->zoomOnZone(xW,yN,xE,yS);
        proj->blockSignals(false);
        proj->setScale(proj->getScale()*.8);
        proj->setUseTempo(true);
        QApplication::processEvents();
    }
    proj->setFrozen(true);
    iso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    iso->setParent(this);
    vlmPoint point(start.x(),start.y());
    point.isStart=true;
    proj->map2screenFloat(cLFA(start.x()),start.y(),&xs,&ys);
    proj->map2screenFloat(cLFA(arrival.x()),arrival.y(),&xa,&ya);
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
    point.capArrival=A360(-tempLine.angle()+90);
    point.distOrigin=0;
    initialDist=tempLine.length();
#endif
    point.origin=NULL;
    point.routage=this;
    point.capOrigin=A360(loxoCap);
    point.eta=eta;
    iso->addVlmPoint(point);
    isochrones.append(iso);
    int nbIso=0;
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
#if 0
    QLineF loxoLine(xa,ya,point.x,point.y);
    loxoLine.setLength(loxoLine.length()*loxoLine.length());
    double loxoAngle=loxoLine.angle();
    loxoLine.setAngle(loxoAngle+angleRange/2);
    QPolygonF limits;
    limits.append(QPointF(xa,ya));
    limits.append(loxoLine.p2());
    loxoLine.setAngle(loxoAngle-angleRange/2);
    limits.append(loxoLine.p2());
    limits.append(QPointF(xa,ya));
#endif
    while(!aborted)
    {
        tDebug.start();
        currentIso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
        currentIso->setParent(this);
        list = iso->getPoints();
        int nbNotDead=0;
        double minDist=initialDist*10;
        double distStart=0;
        for(int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            if(list->at(n).distArrival<minDist)
            {
                minDist=list->at(n).distArrival;
                distStart=list->at(n).distStart;
            }
            double windSpeed,windAngle;
            if(!windIsForced)
            {
                workEta = eta;
                if(whatIfUsed && whatIfJour<=eta)
                    workEta=workEta+whatIfTime*3600;
                if(!grib->getInterpolatedValue_byDates((double) list->at(n).lon,(double) list->at(n).lat,
                       workEta,&windSpeed,&windAngle,INTERPOLATION_DEFAULT)||workEta+timeStep*60>maxDate)
                {
                    iso->setPointDead(n);
                    continue;
                }
                windAngle=radToDeg(windAngle);
            }
            else
            {
                windAngle=this->wind_angle;
                windSpeed=this->wind_speed;
            }
            if(whatIfUsed && whatIfJour<=eta)
                windSpeed=windSpeed*whatIfWind/100.00;
            nbNotDead++;
            iso->setPointWind(n,windAngle,windSpeed);
            double vmg;
            myBoat->getPolarData()->getBvmg(A360(list->at(n).capArrival-windAngle),windSpeed,&vmg);
            iso->setPointCapVmg(n,A360(vmg+windAngle));
        }
        if(minDist<distStart)
            arrivalIsClosest=true;
        else
            arrivalIsClosest=false;
        if(nbNotDead==0) break;
        double workAngleStep=0;
        double workAngleRange=0;
        tempPoints.clear();
        msecsD1=msecsD1+tDebug.elapsed();
        time.start();
        int lastAnnoyingSegment=0;
        bool hasTouchCoast=false;
        for(int n=0;n<list->count();n++)
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
                if(useConverge && arrivalIsClosest)
                {
                    //workAngleRange=qMax((double)angleRange/2,qMin((double)angleRange,(double)angleRange/(1+log((initialDist/minDist)/2))));
                    workAngleRange=qMax((double)angleRange/2,
                                        qMin((double) angleRange,
                                             (double)angleRange/(1+log(2*(list->at(n).distArrival/minDist)))));
                    workAngleStep=qMax(angleStep/2,workAngleRange/(angleRange/angleStep));
                    workAngleStep=qMax((double)3,workAngleStep); /*this allows less points generated but keep orginal total per iso*/
                    //workAngleStep=angleStep;
                    //qWarning()<<"workAngleRange="<<workAngleRange<<"workAngleStep"<<workAngleStep;
                }
                else
                {
                    workAngleRange=angleRange;
                    workAngleStep=qMax((double)3,angleStep);
                }
            }
            double windAngle=list->at(n).wind_angle;
            double windSpeed=list->at(n).wind_speed;
            QList<double> caps=calculateCaps(list->at(n),workAngleStep,workAngleRange);
            QList<vlmPoint> polarPoints;/**/
            double maxDistIso=0;
            double averageDistIso=0;
            int averageDistIsoN=0;
            bool tryingToFindHole=false;
#if 1 /*calculate angle limits*/
            QLineF limitRight,limitLeft;
            if(n>1)
            {
                limitRight.setPoints(QPointF(list->at(n-2).x,list->at(n-2).y),QPointF(xa,ya));
//                limitRight.setAngle(A360(90-list->at(n).capVmg));
                limitRight.setAngle(A360(90-list->at(n-2).capOrigin));
                limitRight.setLength(list->at(n-2).distIso);
            }
            if(n<list->count()-2)
            {
                limitLeft.setPoints(QPointF(list->at(n+2).x,list->at(n+2).y),QPointF(xa,ya));
//                limitLeft.setAngle(A360(90-list->at(n).capVmg));
                limitLeft.setAngle(A360(90-list->at(n+2).capOrigin));
                limitLeft.setLength(list->at(n+2).distIso);
            }
#endif
            for(int ccc=0;ccc<caps.count();ccc++)
            {
                nbCaps++;
#if 1 /*use angle limits*/
                if(!tryingToFindHole && !list->at(0).isStart)
                {
                    QLineF temp(list->at(n).x,list->at(n).y,xa,ya);
                    temp.setAngle(A360(90-caps.at(ccc)));
                    temp.setLength(list->at(n).distIso);
                    QPointF dummy;
                    if(n>1)
                    {
                        if(list->at(n).distIso<list->at(n-1).distIso)
                        {
                            if(temp.intersect(limitRight,&dummy)==QLineF::BoundedIntersection)
                            {
                                nbCapsPruned++;
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
                                nbCapsPruned++;
                                continue;
                            }
                        }
                    }
                }
#endif
                vlmPoint newPoint(0,0);
                cap=caps.at(ccc);
                tfp.start();
                if(!findPoint(list->at(n).lon, list->at(n).lat, windAngle, windSpeed, cap, &newPoint))
                {
                    msecs_3=msecs_3+tfp.elapsed();
                    continue;
                }
                msecs_3=msecs_3+tfp.elapsed();
                double x,y;
                proj->map2screenFloat(cLFA(newPoint.lon),newPoint.lat,&x,&y);
                newPoint.x=x;
                newPoint.y=y;
                newPoint.routage=this;
#if 0
                if(!limits.containsPoint(QPointF(x,y),Qt::OddEvenFill)) continue;
#endif
#if 1
                if(n!=0)
                {
                    Triangle t(Point(list->at(n).x,list->at(n).y),
                               Point(newPoint.x,newPoint.y),
                               Point(list->at(n-1).x,list->at(n-1).y));
                    if (t.orientation()==right_turn) continue;
                }
                if(n!=list->count()-1)
                {
                    Triangle t(Point(list->at(n).x,list->at(n).y),
                               Point(newPoint.x,newPoint.y),
                               Point(list->at(n+1).x,list->at(n+1).y));
                    if (t.orientation()==left_turn) continue;
                }
#endif
                tfp.start();
                if(list->at(n).isStart)
                    newPoint.distIso=newPoint.distStart;
                else
                    newPoint.distIso=findDistancePreviousIso(newPoint,&previousIso);
                msecs_11=msecs_11+tfp.elapsed();
                if(maxDistIso<newPoint.distIso)
                    maxDistIso=newPoint.distIso;
                averageDistIso=averageDistIso+newPoint.distIso;
                averageDistIsoN++;
                if(checkCoast)
                {
#if 1 /*check crossing with coast*/
                    tfp.start();
                    if(map->crossing(QLineF(list->at(n).x,list->at(n).y,newPoint.x,newPoint.y),QLineF(list->at(n).lon,list->at(n).lat,newPoint.lon,newPoint.lat)))
                    {
                        msecs_14=msecs_14+tfp.elapsed();

                        if(!tryingToFindHole)
                        {
                            tryingToFindHole=true;
                            hasTouchCoast=true;
                            polarPoints.clear();
                            caps=calculateCaps(list->at(n),1,179);
                            ccc=-1;
                            iso->setNotSimplificable(n);
                        }
                        continue;
                    }
                    msecs_14=msecs_14+tfp.elapsed();
#endif
                }
                newPoint.origin=iso->getPoint(n);
                newPoint.originNb=n;
                newPoint.capOrigin=A360(cap);
                tfp.start();
#if 0
                orth.setPoints(start.x(),start.y(),newPoint.lon,newPoint.lat);
                newPoint.distStart=orth.getDistance();
                newPoint.capStart=A360(orth.getAzimutDeg()+180);
                orth.setStartPoint(arrival.x(),arrival.y());
                newPoint.distArrival=orth.getDistance();
                newPoint.capArrival=A360(orth.getAzimutDeg()+180);
                orth.setStartPoint(list->at(n).lon,list->at(n).lat);
                newPoint.distOrigin=orth.getDistance();
#else
                QLineF tempLine(newPoint.x,newPoint.y,xs,ys);
                newPoint.distStart=tempLine.length();
                newPoint.capStart=A360(-tempLine.angle()+90);
                tempLine.setP2(QPointF(xa,ya));
                newPoint.distArrival=tempLine.length();
                newPoint.capArrival=A360(-tempLine.angle()+90);
                orth.setPoints(list->at(n).lon,list->at(n).lat,newPoint.lon,newPoint.lat);
                newPoint.distOrigin=orth.getDistance();
#endif
                msecs_21=msecs_21+tfp.elapsed();
                if(tryingToFindHole)
                    newPoint.notSimplificable=true;
#if 1
                else
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
#endif
                /*check that the line(origin,newPoint) do not cross previous iso or previous segments.*/
                tfp.start();
                bool bad=false;
                if(!list->at(n).isStart && previousIso.count()>1 /*&& !tryingToFindHole*/)
                {
#if 1
                    QLineF temp1(list->at(n).x,list->at(n).y,newPoint.x,newPoint.y);
                    QPointF dummy(0,0);
                    QLineF s(previousIso.at(lastAnnoyingSegment),previousIso.at(lastAnnoyingSegment+1));
                    if(n!=lastAnnoyingSegment && n!=lastAnnoyingSegment+1)
                    {
                        if(temp1.intersect(s,&dummy)==QLineF::BoundedIntersection)
                        {
                            continue;
                        }
#if 1
                        if(temp1.intersect(previousSegments.at(lastAnnoyingSegment),&dummy)==QLineF::BoundedIntersection)
                        {
                            continue;
                        }
#endif
                    }
                    for (int i=0;i<previousIso.count()-1;i++)
                    {
                        if(i==lastAnnoyingSegment) continue;
                        QLineF s(previousIso.at(i),previousIso.at(i+1));
                        if(n!=i && n!=i+1)
                        {
                            if(temp1.intersect(s,&dummy)==QLineF::BoundedIntersection)
                            {
                                bad=true;
                                lastAnnoyingSegment=i;
                                break;
                            }
#if 1
                            if(temp1.intersect(previousSegments.at(i),&dummy)==QLineF::BoundedIntersection)
                            {
                                bad=true;
                                lastAnnoyingSegment=i;
                                break;
                            }
#endif
                        }
                    }
                }
#if 0
                QPolygonF previousShape;
                previousShape.append(QPointF(point.x,point.y));
                previousShape=previousShape+previousIso;
                previousShape.append(QPointF(point.x,point.y));
                if(previousShape.containsPoint(QPointF(newPoint.x,newPoint.y),Qt::OddEvenFill))
                    bad=true;
#endif
#endif
                msecs_5=msecs_5+tfp.elapsed();
                if (bad) continue;
//                {
//                    if(polarPoints.isEmpty())
//                        continue;
//                    else
//                        break;
//                }
#if 0
                if(newPoint.distIso<maxDistIso/3 && !tryingToFindHole && !list->at(n).isStart) continue;
#endif
                polarPoints.append(newPoint);
            }
            if(aborted) break;
            if(averageDistIsoN>0)
                averageDistIso=averageDistIso/averageDistIsoN;
#if 0 /**/
            if(!tryingToFindHole && !list->at(n).isStart && averageDistIsoN>5)
            {
                for(int nn=polarPoints.count()-1;nn>=0;nn--)
                {
                    if(polarPoints.at(nn).distIso<(averageDistIso))
                    {
                        polarPoints.removeAt(nn);
                        continue;
                    }
                    polarPoints[nn].maxDistIso=maxDistIso;
                }
            }
#endif
#if 1 /* keep only max 3/4(?) of initial number of points per polar, based on distIso*/
            if(!tryingToFindHole && !list->at(n).isStart)
            {
                int max=(angleRange/angleStep)*0.75;
                if(max<polarPoints.count())
                {
                    QMultiMap<double,vlmPoint> dist;
                    for (int nn=polarPoints.count()-1;nn>=0;nn--)
                    {
                        if(minDist<initialDist/10)
                            dist.insert(polarPoints.at(nn).distArrival,polarPoints.at(nn));
                        else
                            dist.insert(polarPoints.at(nn).distIso,polarPoints.at(nn));
                    }
                    QMapIterator<double,vlmPoint> it(dist);
                    if(minDist<initialDist/10)
                    {
                        it.toBack();
                        while(it.hasPrevious() && polarPoints.count()>max)
                        {
                            polarPoints.removeOne(it.previous().value());
                        }
                    }
                    else
                    {
                        it.toFront();
                        while(it.hasNext() && polarPoints.count()>max)
                        {
                            polarPoints.removeOne(it.next().value());
                        }
                    }
                }
            }
#endif

            if(!polarPoints.isEmpty())
            {
                //qSort(polarPoints.begin(),polarPoints.end(),rightToLeftFromOrigin);
                tempPoints.append(polarPoints);
            }
        }
        msecs_1=msecs_1+time.elapsed();
/*1eme epuration: on supprime les segments qui se croisent */
        time.restart();
#if 1
        if(tempPoints.count()>0 && !tempPoints.at(0).origin->isStart)
             removeCrossedSegments();
#endif
        msecs_2=msecs_2+time.elapsed();





        if(tempPoints.count()==0)
            break;




#if 1 //Check that no segment is crossing it's own isochron. If this is the case remove worst point
        time.restart();
        if(!tempPoints.at(0).origin->isStart)
        {
            checkSegmentCrossingOwnIso();
        }
        msecs_4=msecs_4+time.elapsed();
#endif
#if 1 /*check that the new iso itself does not cross previous segments or iso*/
        time.restart();
        if(!tempPoints.at(0).origin->isStart)
        {
            checkIsoCrossingPreviousSegments();
        }
        msecs_9=msecs_9+time.elapsed();

#endif
#if 1   /*eliminate points by wake pruning*/
        time.restart();
        if(!hasTouchCoast)
            pruneWake(pruneWakeAngle);
        msecs_10=msecs_10+time.elapsed();
#endif
/*elimination de 70% des points surnumeraires*/
        time.restart();
        int limit=(this->angleRange/this->angleStep)+this->explo;
        if(hasTouchCoast) limit=limit*1.5;
        int c=tempPoints.count();
        int toBeRemoved=c-limit;
        if(tempPoints.count()>limit)
        {
            epuration(toBeRemoved*0.7);
        }
        msecs_6=msecs_6+time.elapsed();
        /*final checking and calculating route between Iso*/
        somethingHasChanged=true;
        time.restart();
        bool routeDone=false;
        int nbLoop=0;
        while (somethingHasChanged)
        {
            nbLoop++;
            if(nbLoop>10)
            {
                qWarning()<<"didn't succeed to clean everything in final checking routine";
                break;
            }
            if(nbLoop>maxLoop)
                maxLoop=nbLoop;
            somethingHasChanged=false;
/*Recheck that no segment is crossing it's own isochron*/
            if(!tempPoints.at(0).origin->isStart)
            {
                checkSegmentCrossingOwnIso();
            }
            if(somethingHasChanged) continue;
/*recheck that the new iso itself does not cross previous segments*/
            if(!tempPoints.at(0).origin->isStart)
            {
                checkIsoCrossingPreviousSegments();
            }
            if(somethingHasChanged) continue;
/* now that some fast calculations have been made, compute real thing using route*/
            if(this->useRouteModule && !routeDone)
            {
                QTime t1,t2;
                t1.start();
                somethingHasChanged=true;
                routeDone=true;
                if(useMultiThreading)
                {
                    QList<QList<vlmPoint> > listList;
                    int pp=0;
                    QList<vlmPoint> tempList;
                    int threadCount=QThread::idealThreadCount()+1;
                    for (int t=1;t<=threadCount;t++)
                    {
                        tempList.clear();
                        for (;pp<tempPoints.count()*t/threadCount;pp++)
                        {
                            tempList.append(tempPoints.at(pp));
                        }
                        listList.append(tempList);
                    }
                    tempList.clear();
                    listList = QtConcurrent::blockingMapped(listList, findRoute);
                    tempPoints.clear();
                    for(pp=0;pp<listList.count();pp++)
                        tempPoints.append(listList.at(pp));
                }
                for(int np=0;np<tempPoints.count();np++)
                {
                    vlmPoint newPoint=tempPoints.at(np);
                    if(!useMultiThreading)
                    {
                        QList<vlmPoint> tempPList;
                        tempPList.append(newPoint);
                        newPoint=findRoute(tempPList).first();
                    }
                    if(newPoint.isDead)
                    {
                        tempPoints.removeAt(np);
                        np--;
                        continue;
                    }
                    if(np!=0)
                    {
                        if(tempPoints.at(np-1)==newPoint)
                        {
                            tempPoints.removeAt(np);
                            np--;
                            continue;
                        }
                    }
                    double x,y;
                    proj->map2screenFloat(cLFA(newPoint.lon),newPoint.lat,&x,&y);
                    newPoint.x=x;
                    newPoint.y=y;
#if 1 /*check again if crossing with coast*/
                    if(checkCoast && !this->useMultiThreading)
                    {
                        t2.start();
                        double x1,y1,x2,y2;
                        x1=newPoint.origin->x;
                        y1=newPoint.origin->y;
                        x2=newPoint.x;
                        y2=newPoint.y;
                        if(map->crossing(QLineF(x1,y1,x2,y2),QLineF(newPoint.origin->lon,newPoint.origin->lat,newPoint.lon,newPoint.lat)))
                        {
                            msecs_14=msecs_14+t2.elapsed();
                            tempPoints.removeAt(np);
                            np--;
                            continue;
                        }
                        msecs_14=msecs_14+t2.elapsed();
                    }
#endif
                    if(newPoint.origin->isStart)
                        newPoint.distIso=newPoint.distStart;
                    else
                        newPoint.distIso=findDistancePreviousIso(newPoint,&previousIso);
                    tempPoints.replace(np,newPoint);
                }
                if(checkCoast && this->useMultiThreading)
                {
                    t2.start();
                    tempPoints = QtConcurrent::blockingMapped(tempPoints, checkCoastCollision);
                    for (int np=0;np<tempPoints.count();++np)
                    {
                        if(tempPoints.at(np).isDead)
                        {
                            tempPoints.removeAt(np);
                            --np;
                        }
                    }
                    msecs_14=msecs_14+t2.elapsed();
                }
                msecs_12=msecs_12+t1.elapsed();
                t1.start();
                if(tempPoints.count()>0 && !tempPoints.at(0).origin->isStart)
                    removeCrossedSegments();
                msecs_13=msecs_13+t1.elapsed();
            }
        }
        msecs_7=msecs_7+time.elapsed();
        /*epuration finale final part*/
        time.restart();
        c=tempPoints.count();
        toBeRemoved=c-limit;
        if(tempPoints.count()>limit)
        {
            epuration(toBeRemoved);
        }
        msecs_6=msecs_6+time.elapsed();





        previousIso.clear();
        QTime t2;
        double x1,y1,x2,y2;
        if(tempPoints.count()>0)
        {
            int mmm=0;
            for (int n=0;n<tempPoints.count();n++)
            {
                if(tempPoints.at(n).isDead) continue;
                if(n!=tempPoints.count()-1 && checkCoast)
                {
                    t2.start();
                    x1=tempPoints.at(n).x;
                    y1=tempPoints.at(n).y;
                    x2=tempPoints.at(n+1).x;
                    y2=tempPoints.at(n+1).y;
                    if(map->crossing(QLineF(x1,y1,x2,y2),QLineF(tempPoints.at(n).lon,tempPoints.at(n).lat,tempPoints.at(n+1).lon,tempPoints.at(n+1).lat)))
                    {
                        vlmPoint temp=tempPoints.at(n);
                        temp.isBroken=true;
                        tempPoints.replace(n,temp);
                    }
                    msecs_14=msecs_14+t2.elapsed();
                }
                tempPoints[n].eta=eta+(int)timeStep*60.00;
                currentIso->addVlmPoint(tempPoints[n]);
                previousIso.append(QPointF(tempPoints.at(n).x,tempPoints.at(n).y));
                vlmPointGraphic * vg=new vlmPointGraphic(this,nbIso+1,mmm,
                                                       tempPoints.at(n).lon,
                                                       tempPoints.at(n).lat,
                                                       this->proj,this->myscene,
                                                       Z_VALUE_ISOPOINT);
                vg->setParent(this);
                mmm++;
                QString ss;
                vg->setDebug(ss.sprintf("mdi=%.2f di=%.2f",tempPoints.at(n).maxDistIso,tempPoints.at(n).distIso));
                vg->setEta(eta+(int)timeStep*60.00);
                this->isoPointList.append(vg);
            }
        }
        else
            break;
        list = currentIso->getPoints();
        iso=currentIso;
        if(ncolor>=colorsList.count()) ncolor=0;
        pen.setColor(colorsList[ncolor]);
        pen.setBrush(colorsList[ncolor]);
        iso->setLinePen(pen);
        ncolor++;
        previousSegments.clear();
        time.restart();
        for (int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            segment=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
            segment->setParent(this);
            vlmPoint temp=* iso->getOrigin(n);
            temp.isBroken=false;
            segment->addVlmPoint(temp);
            temp=list->at(n);
            temp.isBroken=false;
            segment->addVlmPoint(temp);
#if 0
            if(temp.debugBool)
                penSegment.setColor(Qt::red);
            else
                penSegment.setColor(Qt::blue);
#endif
            segment->setLinePen(penSegment);
            segment->slot_showMe();
            segments.append(segment);
            previousSegments.append(QLineF(list->at(n).origin->x,list->at(n).origin->y,list->at(n).x,list->at(n).y));
        }
        msecs_15=msecs_15+time.elapsed();
        iso->slot_showMe();
        isochrones.append(iso);
        QCoreApplication::processEvents();
        //qWarning()<<"nb of points in iso"<<isochrones.count()<<":"<<iso->count();
        nbIso++;
        eta=eta+(int)timeStep*60.00;
        vlmPoint to(arrival.x(),arrival.y());
        time.restart();
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
        for (int n=0;n<list->count();n++)
        {
            vlmPoint from=list->at(n);
            orth.setPoints(from.lon,from.lat,to.lon,to.lat);
            if(orth.getDistance()<myBoat->getPolarData()->getMaxSpeed()*60/timeStep)
            {
                if(checkCoast && map->crossing(QLineF(list->at(n).x,list->at(n).y,xa,ya),QLineF(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y())))
                    continue;
                int thisTime=calculateTimeRoute(from,to, &dataThread, NULL, NULL, (timeStep+1)*60);
                if(thisTime<=timeStep*60)
                {
                    arrived=true;
                    break;
                }
            }
        }
        msecs_8=msecs_8+time.elapsed();
        if(nbIso>3000 || arrived || nbNotDead==0)
        {
            break;
        }
    }
    list=iso->getPoints();
    //orth.setEndPoint(arrival.x(),arrival.y());
    int nBest=0;
    if(arrived)
    {
        int minTime=timeStep*10000*60;
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
        for(int n=0;n<list->count();n++)
        {
            if(checkCoast && map->crossing(QLineF(list->at(n).x,list->at(n).y,xa,ya),QLineF(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y())))
                continue;
            vlmPoint from=list->at(n);
            int thisTime=calculateTimeRoute(from,to,&dataThread);
            if(thisTime<minTime)
            {
                nBest=n;
                minTime=thisTime;
            }
        }
        eta=eta+minTime;
        drawResult(list->at(nBest));
    }
    else
    {
        int minDist=10e5;
        Orthodromie oo(0,0,arrival.x(),arrival.y());
        for(int n=0;n<list->count();n++)
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
    if(routeN==0) routeN=1;
    QString temp;
    QTime tt(0,0,0,0);
    int msecs=timeTotal.elapsed();
    tt=tt.addMSecs(msecs);
    QString info;
    qWarning()<<"Total calculation time:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    info="Total calculation time: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
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
    tt=tt.addMSecs(msecs_5);
    qWarning()<<".........out of which detecting collision with previous iso:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    info=info+"\n.........out of which detecting collision with previous iso: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_11);
    qWarning()<<".........out of which calculating distance from previous iso:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    info=info+"\n.........out of which calculating distance from previous iso: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
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
    qWarning()<<"...removing segments crossing their own isochron:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    info=info+"\n...removing segments crossing their own isochron: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
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
    int nbCpu=1;
    if(this->useMultiThreading)
        nbCpu=QThread::idealThreadCount();
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
    qWarning()<<"...displaying segments"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    info=info+"\n...displaying segments: "+tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs-(msecs_1+msecs_2+msecs_21+msecs_4+msecs_6+msecs_7+msecs_8+msecs_9+msecs_15));
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
    QMessageBox msgBox;
    if(this->useRouteModule && !this->useMultiThreading)
    {
        info=info+"\n---Route module statistics---";
        info=info+"\nTotal number of route segments calculated: "+temp.sprintf("%d",routeN);
        info=info+"\npercentage of correct guesses using rough method: "+temp.sprintf("%.2f",100-((double)(NR_n)/(double)routeN)*100.00)+"%";
        info=info+"\nAverage number of iterations: "+temp.sprintf("%.2f",(double)routeTotN/(double)routeN);
        info=info+"\nMax number of iterations made to find a solution: "+temp.sprintf("%d",routeMaxN);
        info=info+"\nNumber of failures using Route between Iso: "+temp.sprintf("%d",routeFailedN);
        info=info+"\nNumber of successes using Route between Iso: "+temp.sprintf("%d",routeN-routeFailedN);
        info=info+"\nNumber of Newton-Raphson calculations: "+temp.sprintf("%d",NR_n);
        info=info+"\nNumber of Newton-Raphson successful calculations: "+temp.sprintf("%d",NR_success);
        info=info+"\n-------------------------------";
    }

    msgBox.setDetailedText(info);
    finalEta=QDateTime::fromTime_t(eta).toUTC();
    msgBox.setIcon(QMessageBox::Information);
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs);
    int elapsed=finalEta.toTime_t()-this->startTime.toTime_t();
    QTime eLapsed(0,0,0,0);
    double jours=elapsed/(24*60*60);
    if (qRound(jours)>jours)
        jours--;
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
    QSpacerItem * hs=new QSpacerItem(500,0,QSizePolicy::Minimum,QSizePolicy::Expanding);
    QGridLayout * layout =(QGridLayout*)msgBox.layout();
    layout->addItem(hs,layout->rowCount(),0,1,layout->columnCount());
    msgBox.exec();
    if(this->useRouteModule)
    {
        qWarning()<<"---Route module statistics---";
        qWarning()<<"Total number of route segments calculated"<<routeN;
        qWarning()<<"Average number of iterations"<<(double)routeTotN/(double)routeN;
        qWarning()<<"Max number of iterations made to find a solution"<<routeMaxN;
        qWarning()<<"Number of failures using Route between Iso"<<routeFailedN;
        qWarning()<<"Number of successes using Route between Iso"<<routeN-routeFailedN;
        qWarning()<<"Number of Newton-Raphson calculations"<<NR_n;
        qWarning()<<"Number of Newton-Raphson successful calculations"<<NR_success;
        qWarning()<<"-------------------------------";
    }
    if (!this->showIso)
        setShowIso(showIso);
    this->done=true;
    if(isConverted())
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
            convertToRoute();
    }
    this->slot_gribDateChanged();
    running=false;
    proj->setFrozen(false);
}
double ROUTAGE::findDistancePreviousIso(vlmPoint P, QPolygonF * isoShape)
{
    double cx=P.x;
    double cy=P.y;
    double minDistanceSegment=10e6;

    for(int i=0;i<isoShape->count()-1;i++)
    {
        double ax=isoShape->at(i).x();
        double ay=isoShape->at(i).y();
        double bx=isoShape->at(i+1).x();
        double by=isoShape->at(i+1).y();
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

        }
        if(distanceSegment<minDistanceSegment)
            minDistanceSegment=distanceSegment;
    }
    return minDistanceSegment;
}
void ROUTAGE::pruneWake(int wakeAngle)
{
    if (wakeAngle==0) return;
    double wakeDir=0;
    QList<vlmPoint> *pIso=isochrones.last()->getPoints();
    for(int n=tempPoints.count()-1;n>=0;n--)
    {
        double x1,y1,x2,y2;
        x1=tempPoints.at(n).x;
        y1=tempPoints.at(n).y;
        for(int m=0;m<pIso->count();m++)
        {
            if(pIso->at(m).distArrival>tempPoints.at(n).distArrival) continue;
            QLineF toArrival(pIso->at(m).x,pIso->at(m).y,xa,ya);
            QLineF toPoint(pIso->at(m).x,pIso->at(m).y,tempPoints.at(n).x,tempPoints.at(n).y);
            if(toArrival.length()/toPoint.length()>0.50) continue;
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
            temp1.setAngle(A360(wakeDir+wakeAngle));
            temp1.setLength(10e5);
            wake.append(temp1.p2());
            temp1.setAngle(A360(wakeDir-wakeAngle));
            wake.append(temp1.p2());
            wake.append(QPointF(x2,y2));
            if(wake.containsPoint(QPointF(x1,y1),Qt::OddEvenFill))
            {
                tempPoints.removeAt(n);
                break;
            }
        }
    }
}
#if 0
int ROUTAGE::routeFunction(double x,vlmPoint from)
{
    double res_lon,res_lat;
    Util::getCoordFromDistanceAngle(from.lat, from.lon, x, from.capOrigin, &res_lat, &res_lon);
    vlmPoint to(res_lon,res_lat);
    return calculateTimeRoute(from,to)-timeStep*60;
}
int ROUTAGE::routeFunctionDeriv(double x,vlmPoint from)
{
    double minGap=x/100;
    int   yr,yl;
    double xl,xr;
    for(int n=1;n<200;n++) /*bad trick to avoid flat spots*/
    {
        xr=x+minGap;
        xl=qMax(x/100,x-minGap);
        yr=routeFunction(xr,from);
        yl=routeFunction(xl,from);
        if(yr!=yl) break;
        minGap=minGap+x/100;
    }
#if 0
    if(yr==yl)
        qWarning()<<"couldn't avoid flat spot"<<"xr="<<xr<<"xl="<<xl<<"x="<<x;
#endif
    return((yr-yl)/(xr-xl));
}
int ROUTAGE::calculateTimeRoute(vlmPoint routeFrom,vlmPoint routeTo,int limit)
{
    double  lastTwa=routeFrom.wind_angle;
    bool    ignoreTackLoss=routeFrom.isStart;
    tooFar=false;
    time_t etaRoute=this->eta;
    time_t etaLimit=eta*2;
    time_t workEta;
    bool hasLimit=false;
    if(limit!=-1)
    {
        hasLimit=true;
        etaLimit=etaRoute+(limit*2);
    }
    bool has_eta=true;
    Orthodromie orth(0,0,0,0);
    Orthodromie orth2(0,0,0,0);
    double lon,lat;
    lon=routeFrom.lon;
    lat=routeFrom.lat;
    double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,previous_remaining_distance,cap1,cap2,diff1,diff2;
    double windAngle,windSpeed,cap,angle;
    time_t maxDate=grib->getMaxDate();
    newSpeed=0;
    distanceParcourue=0;
    res_lon=0;
    res_lat=0;
    previous_remaining_distance=0;
    windAngle=0;
    windSpeed=0;
    orth.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    orth2.setPoints(lon, lat, routeTo.lon,routeTo.lat);
    remaining_distance=orth.getDistance();
    double initialDistance=orth2.getDistance();
    do
        {
            workEta=etaRoute;
            if(whatIfUsed && whatIfJour<=eta)
                workEta=workEta+whatIfTime*3600;
            if(this->windIsForced || (grib->getInterpolatedValue_byDates(lon, lat, workEta,&windSpeed,&windAngle,INTERPOLATION_DEFAULT) && workEta<=maxDate && (!hasLimit || etaRoute<=etaLimit)))
            {
                if(this->windIsForced)
                {
                    windSpeed=this->wind_speed;
                    windAngle=this->wind_angle;
                }
                else
                {
                    windAngle=radToDeg(windAngle);
                }
                if(whatIfUsed && whatIfJour<=eta)
                    windSpeed=windSpeed*whatIfWind/100.00;
                previous_remaining_distance=remaining_distance;
                cap=orth.getAzimutDeg();
                angle=cap-windAngle;
                if(qAbs(angle)>180)
                {
                    if(angle<0)
                        angle=360+angle;
                    else
                        angle=angle-360;
                }
                if(qAbs(angle)<myBoat->getBvmgUp(windSpeed))
                {
                    angle=myBoat->getBvmgUp(windSpeed);
                    cap1=A360(windAngle+angle);
                    cap2=A360(windAngle-angle);
                    diff1=myDiffAngle(cap,cap1);
                    diff2=myDiffAngle(cap,cap2);
                    if(diff1<diff2)
                        cap=cap1;
                    else
                        cap=cap2;
                }
                else if(qAbs(angle)>myBoat->getBvmgDown(windSpeed))
                {
                    angle=myBoat->getBvmgDown(windSpeed);
                    cap1=A360(windAngle+angle);
                    cap2=A360(windAngle-angle);
                    diff1=myDiffAngle(cap,cap1);
                    diff2=myDiffAngle(cap,cap2);
                    if(diff1<diff2)
                        cap=cap1;
                    else
                        cap=cap2;
                }
                newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,angle);
                if(!ignoreTackLoss && this->speedLossOnTack!=1)
                {
                    if((angle>0 && lastTwa<0) || (angle<0 && lastTwa>0))
                        newSpeed=newSpeed*this->speedLossOnTack;
                }
                else
                    ignoreTackLoss=false;
                lastTwa=angle;
                distanceParcourue=newSpeed*myBoat->getVacLen()/3600.00;
                Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                orth.setStartPoint(res_lon, res_lat);
                remaining_distance=orth.getDistance();
            }
            else
            {
#if 0
                if(!grib->getInterpolatedValue_byDates(lon, lat, etaRoute,&windSpeed,&windAngle,INTERPOLATION_DEFAULT))
                    qWarning()<<"out of grib!!";
                if(etaRoute>maxDate)
                    qWarning()<<"out of gribDate!"<<QDateTime::fromTime_t(etaRoute).toString("dd/MM/yy hh:mm:ss")<<QDateTime::fromTime_t(maxDate).toString("dd/MM/yy hh:mm:ss")<<QDateTime::fromTime_t(this->eta).toString("dd/MM/yy hh:mm:ss")<<orth.getDistance();
#endif
                has_eta=false;
                break;
            }
            orth2.setEndPoint(res_lon,res_lat);
            if(orth2.getDistance()>=initialDistance)
            {
                lon=res_lon;
                lat=res_lat;
                etaRoute= etaRoute + myBoat->getVacLen();
                break;
            }
//            if(remaining_distance<distanceParcourue/2 || remaining_distance>previous_remaining_distance)
//            {
//                lon=res_lon;
//                lat=res_lat;
//                etaRoute= etaRoute + myBoat->getVacLen();
//                break;
//            }
            lon=res_lon;
            lat=res_lat;
            etaRoute= etaRoute + myBoat->getVacLen();
        } while (has_eta);
    if(!has_eta)
    {
        if(hasLimit)
            return 10e5;
    }
    lastLonFound=lon;
    lastLatFound=lat;
    return(etaRoute-eta);
}
#endif
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
    if(qAbs(angle)<myBoat->getBvmgUp(windSpeed) && angle!=90) //if too close to wind then use BVMG technique
    {
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,myBoat->getBvmgUp(windSpeed));
//        newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(myBoat->getBvmgUp(windSpeed),qAbs(angle)))));
        newSpeed=newSpeed*qAbs(cos(degToRad(myBoat->getBvmgUp(windSpeed)))/cos(degToRad(qAbs(angle))));
    }
    else if(qAbs(angle)>myBoat->getBvmgDown(windSpeed) && angle!=90)
    {
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,myBoat->getBvmgDown(windSpeed));
//        newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(qAbs(angle),myBoat->getBvmgDown(windSpeed)))));
        newSpeed=newSpeed*qAbs(cos(degToRad(myBoat->getBvmgDown(windSpeed)))/cos(degToRad(qAbs(angle))));
    }
    else
        newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,angle);
    return orth.getDistance()/newSpeed;
}
double ROUTAGE::A360(double hdg)
{
    while (hdg>=360.0) hdg=hdg-360.0;
    while (hdg<0.0) hdg=hdg+360.0;
    return hdg;
}
double ROUTAGE::myDiffAngle(double a1,double a2)
{
    return qAbs(A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
}
double ROUTAGE::mySignedDiffAngle(double a1,double a2)
{
    return (A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
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
    for(int n=0;n<isochrones.count();n++)
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

void ROUTAGE::setShowIso(bool b)
{
    this->showIso=b;
    for (int n=0;n<isochrones.count();n++)
        isochrones[n]->setHidden(!showIso);
    for (int n=0;n<segments.count();n++)
        segments[n]->setHidden(!showIso);
    for (int n=0;n<isoPointList.count();n++)
    {
        isoPointList[n]->shown(b);
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
    for (int n=0;n<result->getPoints()->count();n++)
    {
        vlmPoint temp=result->getPoints()->at(n);
        temp.isBroken=false;
        initialRoad.append(temp);
    }
    result->deleteAll();
    if(arrived)
        result->addPoint(arrival.y(),arrival.x());
    while (true)
    {
        P.isBroken=false;
        result->addVlmPoint(P);
        if (P.isStart) break;
        P= (*P.origin);
    }
    for(int n=1;n<initialRoad.count();n++)
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
void ROUTAGE::setPivotPoint(int isoNb,int pointNb)
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
    colorsList.append(Qt::white);
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
    way->hide();
}

bool ROUTAGE::findPoint(double lon, double lat, double windAngle, double windSpeed, double cap, vlmPoint *pt)
{
    cap=A360(cap);
    double angle,newSpeed;
    time_t workEta;
    double res_lon,res_lat;
    double distanceParcourue=0;
    for(int a=0;a<=1;a++)
    {
        angle=cap-(double)windAngle;
        if(qAbs(angle)>180)
        {
            if(angle<0)
                angle=360+angle;
            else
                angle=angle-360;
        }
        double limit=myBoat->getBvmgUp(windSpeed);
        if(qAbs(angle)<limit && angle!=90) //if too close to wind then use VB-VMG technique
        {
            newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,limit);
//            newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(limit,qAbs(angle)))));
            newSpeed=newSpeed*qAbs(cos(degToRad(limit))/cos(degToRad(qAbs(angle))));
        }
        else
        {
            limit=myBoat->getBvmgDown(windSpeed);
            if(qAbs(angle)>limit && angle!=90)
            {
                newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,limit);
//                newSpeed=newSpeed*qAbs(cos(degToRad(myDiffAngle(qAbs(angle),limit))));
                newSpeed=newSpeed*qAbs(cos(degToRad(limit))/cos(degToRad(qAbs(angle))));
            }
            else
                newSpeed=myBoat->getPolarData()->getSpeed(windSpeed,angle);
        }
        distanceParcourue=newSpeed*timeStep/60.0;
        Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap, &res_lat, &res_lon);
        pt->lon=res_lon;
        pt->lat=res_lat;
        pt->distOrigin=distanceParcourue;
        if(a==0)
        {
            double newWindAngle,newWindSpeed;
            if(!this->windIsForced)
            {
                workEta=eta;
                if(whatIfUsed && whatIfJour<=eta)
                    workEta=workEta+whatIfTime*3600;
                if(!grib->getInterpolatedValue_byDates(res_lon,res_lat,
                       workEta+timeStep*60,&newWindSpeed,&newWindAngle,INTERPOLATION_DEFAULT)||workEta+timeStep*60>grib->getMaxDate())
                {
                    return false;
                }
                newWindAngle=radToDeg(newWindAngle);
            }
            else
            {
                newWindAngle=this->wind_angle;
                newWindSpeed=this->wind_speed;
            }
            if(whatIfUsed && whatIfJour<=eta)
                windSpeed=windSpeed*whatIfWind/100.00;
            windAngle=A360((windAngle+newWindAngle)/2);
            windSpeed=(windSpeed+newWindSpeed)/2;
//            if(!this->useDistIso)
//            {
//                double vmg=0;
//                Orthodromie oo(res_lon,res_lat,arrival.x(),arrival.y());
//                double loxo=oo.getLoxoCap();
//                myBoat->getPolarData()->getBvmg(A360(loxo-newWindAngle),newWindSpeed,&vmg);
//                pt->vmgSpeed=qAbs(myBoat->getPolarData()  ->getSpeed(newWindSpeed,vmg)*cos(degToRad(A360(myDiffAngle(loxo,cap)))));
//            }
        }
    }
    return true;
}
void ROUTAGE::convertToRoute()
{
    bool routeStartBoat=QMessageBox::question(0,tr("Convertion d'un routage en route"),
                                              tr("Voulez-vous que la route parte du bateau a la prochaine vacation?"),
                                              QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes;
    this->converted=true;
    ROUTE * route=parent->addRoute();
    route->setName(tr("Routage: ")+name);
    route->setUseVbVmgVlm(false);
    parent->update_menuRoute();
    route->setBoat(this->myBoat);
    ROUTAGE * parentRoutage=this;
    while(parentRoutage->getIsPivot())
    {
        if(parentRoutage->getFromRoutage())
            parentRoutage=parentRoutage->getFromRoutage();
        else
            break;
    }
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
    for (int n=0;n<list->count();n++)
    {
       if(n!=0)
       {
           if(list->at(n-1)==list->at(n)) continue;
       }
       QString poiName;
       poiName.sprintf("%.5i",list->count()-n);
       poiName=poiPrefix+poiName;
       POI * poi = parent->slot_addPOI(poiName,0,list->at(n).lat,list->at(n).lon,-1,false,false,myBoat);
       poi->setRoute(route);
       poi->setNotSimplificable(list->at(n).notSimplificable);
    }
    delete result;
    result=NULL;
    route->setHidePois(true);
    route->setFrozen(false);
    if(routeStartBoat)
    {
        if (route->getPoiList().at(0)->getRouteTimeStamp()!=-1)
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
}
void ROUTAGE::checkSegmentCrossingOwnIso()
{
    int nn=0;
    int mm=0;
    QPointF crossPoint;
    int maxLook=30;
    for (nn=0;nn<tempPoints.count()-1;nn++)
    {
        QLineF S1(tempPoints.at(nn).lon,tempPoints.at(nn).lat,tempPoints.at(nn+1).lon,tempPoints.at(nn+1).lat);
        bool foundCross=false;
        for(mm=(qMax(0,nn-maxLook));mm<tempPoints.count();mm++)
        {
            if(mm>nn+maxLook) break;
            //if(tempPoints.at(nn).originNb==tempPoints.at(mm).originNb) continue;
            if(mm==nn || mm==nn+1 || mm==nn-1) continue;
            QLineF S2(tempPoints.at(mm).lon,tempPoints.at(mm).lat,tempPoints.at(mm).origin->lon,tempPoints.at(mm).origin->lat);
            if(S1.intersect(S2,&crossPoint)==QLineF::BoundedIntersection)
            {
                if(crossPoint!=S1.p1() && crossPoint!=S1.p2())
                {
                    foundCross=true;
                    break;
                }
            }
        }
        if(foundCross)
        {
            somethingHasChanged=true;
#if 0 /*debug*/
            qWarning()<<"nn="<<nn<<"mm="<<mm;
            QPen penDebug;
            penDebug.setColor(Qt::red);
            penDebug.setBrush(Qt::red);
            penDebug.setWidthF(1);
            vlmLine *L1=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
            L1->setLinePen(penDebug);
            L1->addVlmPoint(tempPoints.at(nn));
            L1->addVlmPoint(tempPoints.at(nn+1));
            vlmLine *L2=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
            penDebug.setColor(Qt::black);
            penDebug.setBrush(Qt::black);
            L2->setLinePen(penDebug);
            L2->addVlmPoint(tempPoints.at(mm));
            vlmPoint O(tempPoints.at(mm).origin->lon,tempPoints.at(mm).origin->lat);
            L2->addVlmPoint(O);
            L1->slot_showMe();
            L2->slot_showMe();
            QApplication::processEvents();
            QMessageBox::information(0,"cross found","cross found!");
            delete L1;
            delete L2;
#endif
            int badOne=0;
#if 1
            QLineF S1(tempPoints.at(nn).lon,tempPoints.at(nn).lat,crossPoint.x(),crossPoint.y());
            QLineF S2(tempPoints.at(mm).lon,tempPoints.at(mm).lat,crossPoint.x(),crossPoint.y());
            QLineF S3(tempPoints.at(nn+1).lon,tempPoints.at(nn+1).lat,crossPoint.x(),crossPoint.y());
            if(S1.length()<S2.length())
            {
                badOne=nn;
                if(S3.length()<S1.length())
                    badOne=nn+1;
            }
            else
            {
                badOne=mm;
                if(S3.length()<S2.length())
                    badOne=nn+1;
            }
#else
            if(tempPoints.at(nn).distIso<tempPoints.at(nn+1).distIso)
                badOne=nn;
            else
                badOne=nn+1;
            if(tempPoints.at(badOne).distIso<tempPoints.at(mm).distIso)
                badOne=mm;
#endif
            tempPoints.removeAt(badOne);
            nn=qMax(-1,nn-maxLook); //not so clever restart of the loop
            maxLook=30;
        }
    }
}
void ROUTAGE::checkIsoCrossingPreviousSegments()
{
    for(int nn=0;nn<tempPoints.count()-1;nn++)
    {
        QPointF dummy;
        QLineF S1(tempPoints.at(nn).x,tempPoints.at(nn).y,tempPoints.at(nn+1).x,tempPoints.at(nn+1).y);
        bool bad=false;
        for(int mm=0;mm<previousSegments.count();mm++)
        {
            if(S1.intersect(previousSegments.at(mm),&dummy)==QLineF::BoundedIntersection)
            {
                bad=true;
                somethingHasChanged=true;
                if(tempPoints.at(nn).distIso<tempPoints.at(nn+1).distIso)
                {
                    tempPoints.removeAt(nn);
                    nn--;
                }
                else
                    tempPoints.removeAt(nn+1);
                break;
            }
        }
        if(bad) continue;
        for(int mm=0;mm<previousIso.count()-1;mm++) /*also check that new Iso does not cross previous iso*/
        {
            QLineF S2(previousIso.at(mm),previousIso.at(mm));
            if(S1.intersect(S2,&dummy)==QLineF::BoundedIntersection)
//                    if(fastIntersects(S1,S2))
            {
                if(tempPoints.at(nn).distIso<tempPoints.at(nn+1).distIso)
                {
                    tempPoints.removeAt(nn);
                    nn--;
                }
                else
                    tempPoints.removeAt(nn+1);
                break;
            }
        }
    }
}
void ROUTAGE::epuration(int toBeRemoved)
{
    QList<vlmPoint> rightFromLoxo;
    QList<vlmPoint> leftFromLoxo;
    bool rightSide=true;
    for(int n=0;n<tempPoints.count();n++)
    {
        if(rightSide)
        {
            Triangle test(Point(xs,ys),Point(xa,ya),Point(tempPoints.at(n).x,tempPoints.at(n).y));
            if(test.orientation()==left_turn)
            {
                tempPoints[n].debugBool=true;
                rightFromLoxo.append(tempPoints.at(n));
            }
            else
            {
                rightSide=false;
                leftFromLoxo.append(tempPoints.at(n));
            }
        }
        else
            leftFromLoxo.append(tempPoints.at(n));
    }
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
    QHash<QString,double> byIndices;
    QList<bool> deadStatus;
    QString s;
    for(int n=0;n<tempPoints.size()-1;n++)
    {
        QLineF temp1(tempPoints.at(n).origin->x,tempPoints.at(n).origin->y,
                     tempPoints.at(n+1).origin->x,tempPoints.at(n+1).origin->y);
        QPointF middle=temp1.pointAt(0.5);
        QLineF temp2(middle.x(),middle.y(),tempPoints.at(n).x,tempPoints.at(n).y);
        QLineF temp3(middle.x(),middle.y(),tempPoints.at(n+1).x,tempPoints.at(n+1).y);
        debugCross0++;
        double critere=temp2.angleTo(temp3);
        byCriteres.insert(critere,QPoint(n,n+1));
        s=s.sprintf("%d;%d",n,n+1);
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
            QString s;
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
            byCriteres.remove(critereNext,QPoint(badOne,next));
//            double length=QLineF(QPointF(tempPoints.at(previous).lon,tempPoints.at(previous).lat),QPointF(tempPoints.at(next).lon,tempPoints.at(next).lat)).length();
#if 1
            QLineF temp1(tempPoints.at(previous).origin->x,tempPoints.at(previous).origin->y,
                         tempPoints.at(next).origin->x,tempPoints.at(next).origin->y);
            QPointF middle=temp1.pointAt(0.5);
            QLineF temp2(middle.x(),middle.y(),tempPoints.at(previous).x,tempPoints.at(previous).y);
            QLineF temp3(middle.x(),middle.y(),tempPoints.at(next).x,tempPoints.at(next).y);
            double critere=temp2.angleTo(temp3);
#else
            Orthodromie oo(tempPoints.at(previous).lon,tempPoints.at(previous).lat,tempPoints.at(next).lon,tempPoints.at(next).lat);
            double critere=oo.getDistance();
#endif
            byCriteres.insert(critere,QPoint(previous,next));
            s=s.sprintf("%d;%d",previous,next);
            byIndices.insert(s,critere);
        }
        else if(previous==-1)
        {
            QString s;
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(critereNext,QPoint(badOne,next));
        }
        else if(next==-1)
        {
            QString s;
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
        }
        currentCount--;
    }
    /*remove points too close from each other*/
    while(currentCount>0)
    {
        d.toFront();
        if(!d.hasNext()) break;
        d.next();
        if(d.key()>this->angleStep) break;
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
            QString s;
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
            byCriteres.remove(critereNext,QPoint(badOne,next));
//            double length=QLineF(QPointF(tempPoints.at(previous).lon,tempPoints.at(previous).lat),QPointF(tempPoints.at(next).lon,tempPoints.at(next).lat)).length();
#if 1
            QLineF temp1(tempPoints.at(previous).origin->x,tempPoints.at(previous).origin->y,
                         tempPoints.at(next).origin->x,tempPoints.at(next).origin->y);
            QPointF middle=temp1.pointAt(0.5);
            QLineF temp2(middle.x(),middle.y(),tempPoints.at(previous).x,tempPoints.at(previous).y);
            QLineF temp3(middle.x(),middle.y(),tempPoints.at(next).x,tempPoints.at(next).y);
            double critere=temp2.angleTo(temp3);
#else
            Orthodromie oo(tempPoints.at(previous).lon,tempPoints.at(previous).lat,tempPoints.at(next).lon,tempPoints.at(next).lat);
            double critere=oo.getDistance();
#endif
            byCriteres.insert(critere,QPoint(previous,next));
            s=s.sprintf("%d;%d",previous,next);
            byIndices.insert(s,critere);
        }
        else if(previous==-1)
        {
            QString s;
            s=s.sprintf("%d;%d",badOne,next);
            double critereNext=byIndices.value(s);
            byCriteres.remove(critereNext,QPoint(badOne,next));
        }
        else if(next==-1)
        {
            QString s;
            s=s.sprintf("%d;%d",previous,badOne);
            double criterePrevious=byIndices.value(s);
            byCriteres.remove(criterePrevious,QPoint(previous,badOne));
        }
        currentCount--;
    }
    for (int nn=deadStatus.count()-1;nn>=0;nn--)
    {
        if(deadStatus.at(nn))
            tempPoints.removeAt(nn);
    }
}
QList<double> ROUTAGE::calculateCaps(vlmPoint point, double workAngleStep, double workAngleRange)
{
    QList<double> caps;
    for(double cc=0;true;cc=cc+workAngleStep)
    {
        if(cc>workAngleRange/2.0)
            cc=workAngleRange/2;
        caps.append(A360(point.capArrival-cc));
        if(cc!=0)
            caps.prepend(A360(point.capArrival+cc));
        if(cc>=workAngleRange/2.0) break;
    }
#if 1
    Point vmg(cos(degToRad(point.capVmg)),sin(degToRad(point.capVmg)));
    Point O(0,0);
    for (int n=0;n<caps.count();n++)
    {
        Point C(cos(degToRad(caps.at(n))),sin(degToRad(caps.at(n))));
        Triangle T(O,C,vmg);
        if(T.orientation()==left_turn)
        {
            caps.insert(n,point.capVmg);
            break;
        }
    }
#endif
#if 0
/*sort headings*/
    double rightMostCap=A360(point.capArrival+89);
    QLineF repere(0,0,100,100);
    repere.setAngle(rightMostCap);
    QMultiMap<double,double> sortedCaps;
    for (int cc=0;cc<caps.count();cc++)
    {
        QLineF temp(0,0,100,100);
        temp.setAngle(caps.at(cc));
        sortedCaps.insert(temp.angleTo(repere),caps.at(cc));
    }
    caps.clear();
    QMapIterator<double,double> iterator(sortedCaps);
    while (iterator.hasNext())
        caps.append(iterator.next().value());
#endif
    return caps;
}
void ROUTAGE::showContextMenu(int isoNb, int pointNb)
{
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
    this->whatIfDate=fromRoutage->getWhatIfDate();
    this->whatIfUsed=fromRoutage->getWhatIfUsed();
    this->whatIfTime=fromRoutage->getWhatIfTime();
    this->whatIfWind=fromRoutage->getWhatIfWind();
    this->angleRange=fromRoutage->getAngleRange();
    this->speedLossOnTack=fromRoutage->getSpeedLossOnTack();
    this->angleStep=fromRoutage->getAngleStep();
    this->timeStep=fromRoutage->getTimeStep();
    this->explo=fromRoutage->getExplo();
    this->wind_angle=fromRoutage->getWindAngle();
    this->wind_speed=fromRoutage->getWindSpeed();
    this->windIsForced=fromRoutage->getWindIsForced();
    this->useRouteModule=fromRoutage->getUseRouteModule();
    this->checkCoast=fromRoutage->getCheckCoast();
    this->useConverge=fromRoutage->useConverge;
    this->pruneWakeAngle=fromRoutage->pruneWakeAngle;
    this->routeFromBoat=false;
    this->toPOI=fromRoutage->getToPOI();
    this->autoZoom=fromRoutage->getAutoZoom();
    isPivot=true;
    ROUTAGE *parentRoutage=this;
    result->deleteAll();
    QList<vlmPoint> initialRoad;
    while(true)
    {
        QList<vlmPoint> parentWay=*parentRoutage->getFromRoutage()->getWay()->getPoints();
        for(int n=1;n<parentWay.count();n++)
            initialRoad.append(parentWay.at(n));
        if(!parentRoutage->getFromRoutage()->getIsPivot() || !parentRoutage->getFromRoutage())
        {
//            initialRoad.prepend(parentWay.at(0));
            break;
        }
        parentRoutage=parentRoutage->getFromRoutage();
    }
    initialRoad.prepend(pivotPoint);
    for(int n=0;n<initialRoad.count();n++)
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
double ROUTAGE::cLFA(double lon)
//convertLonForAntiMeridian
{
    double xW=proj->getXmin();
    if(xW>=0 && lon>=0) return lon;
    if(xW<=0 && lon<=0) return lon;
    if(qAbs(qRound(qAbs(lon-xW))-qRound(myDiffAngle(A360(lon),A360(xW))))<=2) return lon;
    if(xW>=0)
    {
        return xW+myDiffAngle(xW,lon+360.0);
    }
    else
    {
        if(xW<-180)
            return lon-360;
        else
            return xW-myDiffAngle(A360(xW),lon);
    }
}
