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


#include "Orthodromie.h"
#include "Projection.h"
#include "routage.h"
#include "route.h"
#include "Grib.h"
#include "mycentralwidget.h"
#include "vlmLine.h"
#include "POI.h"
#include "Routage_Editor.h"
#include "boatAccount.h"
#include "Polar.h"
#include "Point.h"
#include "Segment.h"
#include "Triangle.h"
#include "Util.h"
#include "Polygon.h"
#include <QDebug>
bool leftToRight(const vlmPoint & P1,const vlmPoint & P2)
{
    Triangle triangle(Point(P1.startLon,P1.startLat),
                      Point(P1.lon,P1.lat),
                      Point(P2.lon,P2.lat));
    if(triangle.orientation()==left_turn)
        return true;
    else
        return false;
}
ROUTAGE::ROUTAGE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow)
            : QGraphicsWidget()

{
    this->proj=proj;
    this->name=name;
    this->myscene=myScene;
    this->grib=grib;
    this->parent=parentWindow;
    this->color=Qt::red;
    this->width=3;
    this->startTime= QDateTime::currentDateTime().toUTC();
    pen.setColor(Qt::red);
    pen.setBrush(Qt::red);
    pen.setWidthF(2);
    this->angleRange=120;
    this->angleStep=1;
    this->timeStep=60;
    this->explo=2;
    this->wind_angle=0;
    this->wind_speed=20;
    this->windIsForced=false;
    this->showIso=false;
    this->done=false;
    this->converted=false;
    result=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
}
ROUTAGE::~ROUTAGE()
{
    for (int n=0;n<isochrones.count();n++)
        delete isochrones[n];
    for (int n=0;n<segments.count();n++)
        delete segments[n];
    if(result!=NULL)
        delete result;
}
void ROUTAGE::setBoat(boatAccount *boat)
{
    this->boat=boat;
}
void ROUTAGE::setWidth(float width)
{
    this->width=width;
//    pen.setWidthF(width);
}
void ROUTAGE::setColor(QColor color)
{
    this->color=color;
//    pen.setColor(color);
//    pen.setBrush(color);
}
void ROUTAGE::calculate()
{
//    if (!(boat && boat->getPolarData() && grib && grib->isOk() && boat!=NULL)) return;
    if (!(boat && boat->getPolarData() && boat!=NULL)) return;
    QColor colors[14]={Qt::white,Qt::black,Qt::red,Qt::darkRed,Qt::green,Qt::darkGreen,Qt::blue,Qt::darkBlue,Qt::cyan,Qt::darkCyan,Qt::magenta,Qt::darkMagenta,Qt::yellow,Qt::darkYellow};
    int ncolor=0;
    time_t eta;
    eta=grib->getCurrentDate();
    Orthodromie orth(0,0,0,0);
    float   cap;
    start.setX(fromPOI->getLongitude());
    start.setY(fromPOI->getLatitude());
    arrival.setX(toPOI->getLongitude());
    arrival.setY(toPOI->getLatitude());
    iso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    orth.setPoints(start.x(),start.y(),arrival.x(),arrival.y());
//    loxoCap=orth.getAzimutDeg();
// use LoxoCap instead
    loxoCap=orth.getLoxoCap();
    vlmPoint point(start.x(),start.y());
    point.isStart=true;
    point.origin=NULL;
    point.startLon=start.x();
    point.startLat=start.y();
    iso->addVlmPoint(point);
    isochrones.append(iso);
    int nbIso=0;
    arrived=false;
    QList<vlmPoint> * list;
    //QList<vlmPoint> * previousList;
    vlmLine * currentIso;
    vlmLine * segment;
    QPen penSegment;
    penSegment.setColor(Qt::gray);
    penSegment.setBrush(Qt::gray);
    penSegment.setWidthF(0.5);
    QPolygonF * isoShape=new QPolygonF();
    isoShape->push_back(start);
    while(true)
    {
        currentIso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
        list = iso->getPoints();
        int nbNotDead=0;
        for(int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            if(!windIsForced)
            {
                if(!grib->getInterpolatedValue_byDates((double) list->at(n).lon,(double) list->at(n).lat,
                       eta,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA))
                {
                    iso->setPointDead(n);
                    continue;
                }
                wind_angle=radToDeg(wind_angle);
            }
            nbNotDead++;
            iso->setPointWind(n,wind_angle,wind_speed);
        }
        QList<vlmPoint> tempPoints;
        for(int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead) continue;
            wind_angle=list->at(n).wind_angle;
            wind_speed=list->at(n).wind_speed;
            orth.setPoints(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y());
            double loxo=orth.getLoxoCap();
            QList<float> caps;
            if(list->at(n).isStart)
            {
                QList<float> cap_left;
                QList<float> cap_right;
                for(float cc=angleStep/2.0;true;cc=cc+angleStep)
                {
                    if(cc>angleRange/2.0)
                        cc=angleRange/2.0;
                    cap_left.append(loxoCap-cc);
                    cap_right.append(loxoCap+cc);
                    if(cc>=angleRange/2.0)
                        break;
                }
                for(int ccc=cap_left.count()-1;ccc>=0;ccc--)
                    caps.append(cap_left[ccc]);
                for (int ccc=0;ccc<cap_right.count();ccc++)
                    caps.append(cap_right[ccc]);
            }
            else
            {
                float twa=loxo-wind_angle;
                double angle=0;
                boat->getPolarData()->getBvmg(twa,wind_speed,&angle);
                float capBcmg=A360(angle+wind_angle);
                if(tooFar(list->at(n)))
                    caps.append(A360(loxoCap));
                else
                    caps.append(A360(list->at(n).initialCap));
//                caps.append(capBcmg);
//                caps.append(loxo);
            }
            for(int ccc=0;ccc<caps.count();ccc++)
            {
                cap=caps[ccc];
                vlmPoint newPoint(0,0);
                findPoint(list->at(n).lon, list->at(n).lat, wind_angle, wind_speed, cap, &newPoint);
                QPointF P(newPoint.lon,newPoint.lat);
                if(isoShape->containsPoint(P,Qt::OddEvenFill)) continue;
                QPolygonF temp;
                temp.push_back(QPointF(iso->getPoint(n)->lon,iso->getPoint(n)->lat));
                temp.push_back(QPointF(newPoint.lon,newPoint.lat));
                temp.push_back(arrival);
                temp.push_back(QPointF(iso->getPoint(n)->lon,iso->getPoint(n)->lat));
                if(isoShape->intersected(temp).size()!=0) continue;
//                if(list->at(n).isStart || true)
//                {
                    newPoint.origin=iso->getPoint(n);
                    newPoint.initialCap=A360(cap);
                    newPoint.startLon=start.x();
                    newPoint.startLat=start.y();
                    tempPoints.append(newPoint);
//                }
//                else
//                {
//                    if(n<list->count()-1)
//                    {
//                        newPoint.initialCap=A360(cap);
//                        double timeFromNext=findTime(iso->getPoint(n+1), & newPoint, & cap);
//                        if(timeFromNext>this->timeStep/60.0)
//                        {
//                            newPoint.origin=iso->getPoint(n);
//                            tempPoints.append(newPoint);
//                        }
//                    }
//                    if(n>0)
//                    {
//                        newPoint.initialCap=A360(cap);
//                        double timeFromPrev=findTime(iso->getPoint(n-1), & newPoint, & cap);
//                        if(timeFromPrev>this->timeStep/60.0)
//                        {
//                            newPoint.origin=iso->getPoint(n);
//                            tempPoints.append(newPoint);
//                        }
//                    }
//                }
            }
        }
        if(!list->at(0).isStart)
        {
            bool somethingHasBeenImproved=true;
            int passes;
            while(somethingHasBeenImproved)
            {
                qWarning()<<"passes="<<passes++;
                somethingHasBeenImproved=false;
                QList<vlmPoint> newPoints;
                if(tempPoints.count()>0)
                {
                    qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
                }
                for(int n=1;n<tempPoints.count()-1;n++)
                {
                    float cap1=0;
                    float cap2=0;
                    double timeFromNext=findTime(tempPoints.at(n+1).origin, tempPoints.at(n), & cap1);
                    double timeFromPrev=findTime(tempPoints.at(n-1).origin, tempPoints.at(n), & cap2);
                    double time=this->timeStep/60.0;
                    if(timeFromNext<timeFromPrev)
                    {
                        if(timeFromNext<time)
                        {
                            tempPoints[n].isDead=true;
                            vlmPoint newPoint(0,0);
                            findPoint(tempPoints.at(n+1).origin->lon, tempPoints.at(n+1).origin->lat,
                                      tempPoints.at(n+1).origin->wind_angle, tempPoints.at(n+1).origin->wind_speed, cap1, &newPoint);
                            newPoint.origin=tempPoints.at(n+1).origin;
                            newPoint.initialCap=A360(tempPoints.at(n).initialCap);
                            newPoint.startLon=start.x();
                            newPoint.startLat=start.y();
                            newPoints.append(newPoint);
                            somethingHasBeenImproved=true;
                            qWarning()<<"next is better";
                        }
                    }
                    else
                    {
                        if(timeFromPrev<time)
                        {
                            tempPoints[n].isDead=true;
                            vlmPoint newPoint(0,0);
                            findPoint(tempPoints.at(n-1).origin->lon, tempPoints.at(n-1).origin->lat,
                                      tempPoints.at(n-1).origin->wind_angle, tempPoints.at(n-1).origin->wind_speed, cap2, &newPoint);
                            newPoint.origin=tempPoints.at(n-1).origin;
                            newPoint.initialCap=A360(tempPoints.at(n).initialCap);
                            newPoint.startLon=start.x();
                            newPoint.startLat=start.y();
                            newPoints.append(newPoint);
                            somethingHasBeenImproved=true;
                            qWarning()<<"prev is better";
                        }
                    }
                }
                for(int n=tempPoints.count()-1;n>0;n--)
                {
                    if(tempPoints.at(n).isDead)
                        tempPoints.removeAt(n);
                }
                tempPoints=tempPoints+newPoints;
            }
        }
        if(tempPoints.count()>0)
        {
            qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
            for (int n=0;n<tempPoints.count();n++)
                currentIso->addVlmPoint(tempPoints[n]);
        }
        list = currentIso->getPoints();
//make sure the segment wont cross with any other (TO BE OPTIMIZED A LOT)
        for (int nn=10e10;nn<list->count()-1;nn++) // skipped for the time being
        {
            //qWarning()<<"treating point "<<nn<<" out of "<<list->size();
            int mm=nn+1;
            if(list->at(mm).origin==list->at(nn).origin) continue;
            int toBeKilled=0;
            if(intersects(currentIso, nn, mm, & toBeKilled))
            {
//                if(toBeKilled==1)
//                {
//                    currentIso->removeVlmPoint(nn);
//                }
//                else if (toBeKilled==2)
//                {
//                    currentIso->removeVlmPoint(mm);
//                }
//                else
                {
                    if(list->at(nn).distIso==-1)
                    {
                        QPointF P1=closestPointPreviousIso(list->at(nn),isoShape);
                        QLineF isoDist1(P1.x(),P1.y(),list->at(nn).lon,list->at(nn).lat);
                        currentIso->setPointDistIso(nn,isoDist1.length());
                    }
                    if(list->at(mm).distIso==-1)
                    {
                        QPointF P2=closestPointPreviousIso(list->at(mm),isoShape);
                        QLineF isoDist2(P2.x(),P2.y(),list->at(mm).lon,list->at(mm).lat);
                        currentIso->setPointDistIso(mm,isoDist2.length());
                    }
                    if(list->at(nn).distIso<list->at(mm).distIso)
                    {
                        currentIso->removeVlmPoint(nn);
                    }
                    else
                    {
                        currentIso->removeVlmPoint(mm);
                    }
                }
                nn=qMax(-1,nn-2); /*habile restart de la loop, a revoir p-tet*/
            }
        }
        //epuration finale, on ne garde que le nombre initial de points + le coeff d'exploration (e.g. 120/10+2=14 pts)
        int limit=(this->angleRange/this->angleStep)+this->explo;
        int c=list->count();
        int toBeRemoved=c-limit;
        if(list->count()>limit && false)
        {
#if 0 /* just take them out at regular interval */
            int n=0;
            float m=0;
            float inc=(float) c/(float) toBeRemoved;
            for (int i=0;i<toBeRemoved;i++)
            {
                m=m+inc;
                if(qRound(m)<c)
                {
                    currentIso->setPointDead(qRound(m));
                    n++;
                }
            }
#endif
#if 0 /* we first identify the root with most children, then we kill the child which is the closest from previous iso */
            QMultiMap<int,int> rootNb;
            QMultiMap<int,int> rootMap;
            int indexRoot=0;
            int count=0;
            vlmPoint lastOrig(list->at(0).origin->lon,list->at(0).origin->lat);
            for(int n=0;n<list->count();n++)
            {
                if(list->at(n).distIso==-1)
                {
                    QPointF P=closestPointPreviousIso(list->at(n),isoShape);
                    QLineF isoDist(P.x(),P.y(),list->at(n).lon,list->at(n).lat);
                    currentIso->setPointDistIso(n,isoDist.length());
                }
                if(list->at(n).origin->lon!=lastOrig.lon && list->at(n).origin->lat!=lastOrig.lat)
                {
                    rootNb.insert(count,indexRoot);
                    lastOrig.lon=list->at(n).origin->lon;
                    lastOrig.lat=list->at(n).origin->lat;
                    indexRoot++;
                    count=0;
                }
                rootMap.insert(indexRoot,n);
                count++;
            }
            rootNb.insert(count,indexRoot);
/* at that point we have two MultiMaps, one with (count_of_children,origin_nb), one with (origin_nb,index_in_list) */
            while(toBeRemoved>0)
            {
                QMapIterator<int,int> root(rootNb);
                while(root.hasNext())
                {
                    indexRoot=root.next().value();
                    int keyRoot=root.key();
                    qWarning()<<"iso nb "<<indexRoot<<" has "<<keyRoot<<" children";
                }
                root.toBack();
                indexRoot=root.previous().value();
                int keyRoot=root.key();
                QList<int> children=rootMap.values(indexRoot);
                QMultiMap<double,int> distIso;
                for(int n=0;n<children.size();n++)
                {
                    distIso.insert(list->at(children.at(n)).distIso,children.at(n));
                }
                QMapIterator<double,int> dist(distIso);
                int indice=dist.next().value();
                currentIso->setPointDead(indice);
                toBeRemoved--;
                rootMap.remove(indexRoot,indice);
                rootNb.remove(keyRoot,indexRoot);
                rootNb.insert(keyRoot-1,indexRoot);
            }
#endif
#if 1 /* in this solution we sort each couple of adjacent iso points by the distance between them, then we kill one of the point belonging to the first couples*/
            while(toBeRemoved>0)
            {
                QMultiMap<double,int> distances;
                for(int n=0;n<list->size()-1;n++)
                {
                    double length=QLineF(QPointF(list->at(n).lon,list->at(n).lat),QPointF(list->at(n+1).lon,list->at(n+1).lat)).length();
                    distances.insert(length,n);
                }
                QMapIterator<double,int> d(distances);
                int coupleNb=d.next().value();
                if(coupleNb==0)
                    currentIso->removeVlmPoint(coupleNb+1);
                else if(coupleNb==list->size()-2)
                    currentIso->removeVlmPoint(coupleNb);
                else
                {
//                    if(distances.key(coupleNb-1)<distances.key(coupleNb+1))
//                        currentIso->removeVlmPoint(coupleNb);
//                    else
//                        currentIso->removeVlmPoint(coupleNb+1);
                    if(list->at(coupleNb).distIso==-1)
                    {
                        QPointF P=closestPointPreviousIso(list->at(coupleNb),isoShape);
                        QLineF isoDist(P.x(),P.y(),list->at(coupleNb).lon,list->at(coupleNb).lat);
                        currentIso->setPointDistIso(coupleNb,isoDist.length());
                    }
                    if(list->at(coupleNb+1).distIso==-1)
                    {
                        QPointF P=closestPointPreviousIso(list->at(coupleNb+1),isoShape);
                        QLineF isoDist(P.x(),P.y(),list->at(coupleNb+1).lon,list->at(coupleNb+1).lat);
                        currentIso->setPointDistIso(coupleNb+1,isoDist.length());
                    }
                    if(list->at(coupleNb).distIso<list->at(coupleNb+1).distIso)
                        currentIso->removeVlmPoint(coupleNb);
                    else
                        currentIso->removeVlmPoint(coupleNb+1);
                }
                toBeRemoved--;
            }
#endif
        }
        for(int n=list->count()-1;n>0;n--)
        {
            if(list->at(n).isDead)
                currentIso->removeVlmPoint(n);
        }
        iso=currentIso;
        if(ncolor>14) ncolor=0;
        pen.setColor(colors[ncolor]);
        pen.setBrush(colors[ncolor]);
        iso->setLinePen(pen);
        ncolor++;
        for (int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            segment=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
            segment->addVlmPoint(* iso->getOrigin(n));
            segment->addVlmPoint(list->at(n));
            segment->setLinePen(penSegment);
            segment->slot_showMe();
            segments.append(segment);
        }
        iso->slot_showMe();
        QCoreApplication::processEvents();
        isochrones.append(iso);
        nbIso++;
        isoShape->clear();;
        isoShape->append(start);
        for (int n=1;n<isochrones.count()-1;n++)
        {
            QPointF P(isochrones[n]->getPoint(0)->lon,isochrones[n]->getPoint(0)->lat);
            isoShape->append(P);
        }
        for(int n=0;n<list->count();n++)
        {
            QPointF P(list->at(n).lon,list->at(n).lat);
            isoShape->append(P);
        }
        for (int n=isochrones.count()-2;n>1;n--)
        {
            QPointF P(isochrones[n]->getLastPoint()->lon,isochrones[n]->getLastPoint()->lat);
            isoShape->append(P);
        }
        isoShape->append(start);
        if(isoShape->containsPoint(arrival,Qt::OddEvenFill))
        {
            arrived=true;
            isochrones.removeLast();
            delete iso;
            iso=isochrones.last();
        }
        if(nbIso>100 || arrived || nbNotDead==0)
        {
            delete isoShape;
            break;
        }
        eta=eta+(int)timeStep*60.00;
    }
    list=iso->getPoints();
    orth.setEndPoint(arrival.x(),arrival.y());
    double minDist=10e30;
    int nBest=0;
    for(int n=0;n<list->count();n++)
    {
        orth.setStartPoint(list->at(n).lon,list->at(n).lat);
        if(orth.getDistance()<minDist)
        {
            nBest=n;
            minDist=orth.getDistance();
        }
    }
    drawResult(list->at(nBest));
    if (!this->showIso)
        setShowIso(showIso);
    this->done=true;
    if(isConverted())
        convertToRoute();
}
QPointF ROUTAGE::closestPointPreviousIso(vlmPoint P, QPolygonF * isoShape)
{
    QPolygonF toStart;
    toStart.append(start);
    toStart.append(QPointF(P.lon,P.lat));
    toStart.append(QPointF(start.x()+0.01,start.y()+0.01));
    QPolygonF intersection=isoShape->intersected(toStart);
    if(intersection.count()==0) return start;
    double minDist=10e1000;
    int N=0;
    QLineF length(P.lon,P.lat,start.x(),start.y());
    for(int n=0;n<intersection.count();n++)
    {
        length.setP2(intersection.at(n));
        if(length.length()<minDist)
        {
            minDist=length.length();
            N=n;
        }
    }
    return intersection.at(N);
}
bool ROUTAGE::intersects(vlmLine *iso, int nn, int mm,int *toBeKilled)
{
    QList<vlmPoint> * list=iso->getPoints();
    vlmPoint P1=list->at(nn);
    vlmPoint P2=list->at(mm);
    if(P1.origin==P2.origin) return false;
    QLineF S1(P1.origin->lon,P1.origin->lat,P1.lon,P1.lat);
    QLineF S2(P2.origin->lon,P2.origin->lat,P2.lon,P2.lat);
    int doesItCross=superIntersects(S1,S2);
    if(doesItCross==BOUNDED_CROSS)
        return true;
    else if(doesItCross==L1_CROSS)
    {
        *toBeKilled=1;
        return true;
    }
    else if(doesItCross==L2_CROSS)
    {
        *toBeKilled=2;
        return true;
    }
    return false;
}
int ROUTAGE::superIntersects(QLineF L1,QLineF L2)
{
    QPointF crossPoint(0,0);
    QPointF dummy(0,0);
    if(L1.intersect(L2,&crossPoint)==QLineF::BoundedIntersection)
        return BOUNDED_CROSS;
    QLineF S1=L1;
    S1.setP2(crossPoint);
    S1.setLength(S1.length()*2.0);
    if(S1.intersect(L2,&dummy)==QLineF::BoundedIntersection)
        return L1_CROSS;
    QLineF S2=L2;
    S2.setP2(crossPoint);
    S2.setLength(S2.length()*2.0);
    if(S2.intersect(L1,&dummy)==QLineF::BoundedIntersection)
        return L2_CROSS;
    return NO_CROSS;
}
void ROUTAGE::findPoint(float lon, float lat, double wind_angle, double wind_speed, float cap, vlmPoint *pt)
{
    double angle,newSpeed,res_lon,res_lat;
    angle=cap-(float)wind_angle;
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    if(qAbs(angle)<boat->getBvmgUp(wind_speed)) //if too close to wind then use BVMG technique
    {
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,boat->getBvmgUp(wind_speed));
        newSpeed=newSpeed*cos(degToRad(boat->getBvmgUp(wind_speed)-qAbs(angle)));
    }
    else if(qAbs(angle)>boat->getBvmgDown(wind_speed))
    {
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,boat->getBvmgDown(wind_speed));
        newSpeed=newSpeed*cos(degToRad(qAbs(angle)-boat->getBvmgDown(wind_speed)));
    }
    else
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,angle);
    Util::getCoordFromDistanceAngle(lat, lon, newSpeed*timeStep/60.00, cap, &res_lat, &res_lon);
    pt->lon=res_lon;
    pt->lat=res_lat;
}
double ROUTAGE::findTime(const vlmPoint * pt, vlmPoint newPoint, float * cap)
{
    double angle,newSpeed,lon,lat,wind_speed,wind_angle;
    lon=newPoint.lon;
    lat=newPoint.lat;
    wind_speed=pt->wind_speed;
    wind_angle=pt->wind_angle;
    angle=*cap-(float)wind_angle;
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    if(qAbs(angle)<boat->getBvmgUp(wind_speed)) //if too close to wind then use BVMG technique
    {
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,boat->getBvmgUp(wind_speed));
        newSpeed=newSpeed*cos(degToRad(boat->getBvmgUp(wind_speed)-qAbs(angle)));
    }
    else if(qAbs(angle)>boat->getBvmgDown(wind_speed))
    {
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,boat->getBvmgDown(wind_speed));
        newSpeed=newSpeed*cos(degToRad(qAbs(angle)-boat->getBvmgDown(wind_speed)));
    }
    else
        newSpeed=boat->getPolarData()->getSpeed(wind_speed,angle);
    Orthodromie orth(pt->lon,pt->lat,lon,lat);
    *cap=orth.getLoxoCap();
    return orth.getLoxoDistance()/newSpeed;
}
float ROUTAGE::A360(float hdg)
{
    while (hdg>=360.0) hdg=hdg-360.0;
    while (hdg<0.0) hdg=hdg+360.0;
    return hdg;
}
float ROUTAGE::myDiffAngle(float a1,float a2)
{
    return qAbs(A360(qAbs(a1)+ 180 -qAbs(a2)) -180);
}
void ROUTAGE::slot_delete()
{
    int rep = QMessageBox::question (parent,
            tr("Détruire le routage : %1?").arg(name),
            tr("La destruction d'un routage est definitive."),
            QMessageBox::Yes | QMessageBox::Cancel);
    if (rep == QMessageBox::Cancel) return;
    parent->deleteRoutage(this);
}
void ROUTAGE::slot_edit()
{
    emit editMe(this);
}
void ROUTAGE::setShowIso(bool b)
{
    this->showIso=b;
    for (int n=0;n<isochrones.count();n++)
        isochrones[n]->setHidden(!showIso);
    for (int n=0;n<segments.count();n++)
        segments[n]->setHidden(!showIso);
}
void ROUTAGE::drawResult(vlmPoint P)
{
    result->addPoint(arrival.y(),arrival.x());
    while (true)
    {
        result->addVlmPoint(P);
        if (P.isStart) break;
        P= (*P.origin);
    }
    pen.setWidthF(width);
    pen.setColor(color);
    pen.setBrush(color);
    result->setLinePen(pen);
    result->slot_showMe();
    pen.setColor(Qt::red);
    pen.setBrush(Qt::red);
    pen.setWidthF(2);
}
void ROUTAGE::convertToRoute()
{
    this->converted=true;
    ROUTE * route=parent->addRoute();
    route->setName("Routage: "+name);
    parent->update_menuRoute();
    route->setBoat(this->boat);
    route->setStartTime(this->startTime);
    route->setStartTimeOption(3);
    route->setStartFromBoat(false);
    route->setColor(this->color);
    route->setWidth(this->width);
    route->setFrozen(true);
    QList<vlmPoint> * list=result->getPoints();
    for (int n=0;n<list->count();n++)
    {
       QString poiName;
       poiName.sprintf("R%.5i",list->count()-n);
       if(n==list->count()-1)
       {
           fromPOI->setName(poiName);
           fromPOI->setRoute(route);
           break;
       }
       if(n==0)
       {
           toPOI->setName(poiName);
           toPOI->setRoute(route);
           continue;
       }
       POI * poi = parent->slot_addPOI(poiName,0,list->at(n).lat,list->at(n).lon,0,false,false,boat);
       poi->setRoute(route);
       poi->setMyLabelHidden(true);
    }
    delete result;
    result=NULL;
    route->setFrozen(false);
}
bool ROUTAGE::tooFar(vlmPoint point)
{
    QLineF S1(arrival,start);
    QLineF S2(arrival.x(),arrival.y(),point.lon,point.lat);
    QLineF S3(start,arrival);
    QLineF S4(start.x(),start.y(),point.lon,point.lat);
    double a1=qAbs(S2.angleTo(S1));
    double a2=qAbs(S4.angleTo(S3));

    if(a1>180) a1=360-a1;
    if(a2>180) a2=360-a2;
    if(a1>a2)
        return true;
    else
        return false;
}
