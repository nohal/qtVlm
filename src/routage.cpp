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
#include "Grib.h"
#include "mycentralwidget.h"
#include "vlmLine.h"
#include "POI.h"
#include "Routage_Editor.h"
#include "boat.h"
#include "Polar.h"
#include "Point.h"
#include "Segment.h"
#include "Triangle.h"
#include "Util.h"
#include "Polygon.h"
#include "route.h"
#include <QDebug>
bool leftToRight(const vlmPoint & P1,const vlmPoint & P2)
{
    Triangle triangle(Point(P1.eyeLon,P1.eyeLat),
                      Point(P1.lon,P1.lat),
                      Point(P2.lon,P2.lat));
    return triangle.orientation()==left_turn;
}
bool leftToRightFromOrigin(const vlmPoint & P1,const vlmPoint & P2)
{
    Triangle triangle(Point(P1.origin->lon,P1.origin->lat),
                      Point(P1.lon,P1.lat),
                      Point(P2.lon,P2.lat));
    return triangle.orientation()==left_turn;
}
bool byOrigin(const vlmPoint & P1,const vlmPoint & P2)
{
    return P1.originNb<P2.originNb;
}
bool byDistanceArrival(const vlmPoint & P1,const vlmPoint & P2)
{
    return P1.distArrival<P2.distArrival;
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
    this->angleStep=3;
    this->timeStep=60;
    this->explo=2;
    this->wind_angle=0;
    this->wind_speed=20;
    this->windIsForced=false;
    this->showIso=true;
    this->done=false;
    this->converted=false;
    this->useTrueVacation=true;
    result=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    debug1=0;
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
void ROUTAGE::setBoat(boat *myBoat)
{
    this->myBoat=myBoat;
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
    if (!(myBoat && myBoat->getPolarData() && myBoat!=NULL)) return;
    QTime timeTotal;
    timeTotal.start();
    int msecs_1=0;
    int msecs_2=0;
    int msecs_3=0;
    int msecs_4=0;
    int msecs_5=0;
    int msecs_6=0;
    int msecs_7=0;
    QTime time;
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
    eta=startTime.toUTC().toTime_t();
    Orthodromie orth(0,0,0,0);
    float   cap;
    start.setX(fromPOI->getLongitude());
    start.setY(fromPOI->getLatitude());
    arrival.setX(toPOI->getLongitude());
    arrival.setY(toPOI->getLatitude());
    iso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
    orth.setPoints(start.x(),start.y(),arrival.x(),arrival.y());
    loxoCap=orth.getLoxoCap();
    initialDist=orth.getLoxoDistance();
    vlmPoint point(start.x(),start.y());
    point.isStart=true;
    point.origin=NULL;
    point.eyeLon=start.x();
    point.eyeLat=start.y();
    iso->addVlmPoint(point);
    point.capOrigin=A360(loxoCap);
    double farAwayX=0;
    double farAwayY=0;
    Util::getCoordFromDistanceAngle(start.y(), start.x(), orth.getLoxoDistance()*2, A360(loxoCap), &farAwayY, &farAwayX);
    QPointF farAway(farAwayX,farAwayY);
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
//    QPolygonF * isoShape=new QPolygonF();
//    isoShape->push_back(start);
    routeN=0;
    routeMaxN=0;
    routeTotN=0;
    routeFailedN=0;
    time_t maxDate=grib->getMaxDate();

    QPolygonF previousIso;
    QList<QLineF> previousSegments;
    while(true)
    {
        currentIso=new vlmLine(proj,myscene,Z_VALUE_ROUTAGE);
        list = iso->getPoints();
        int nbNotDead=0;
        float minDist=initialDist*10;
        for(int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead)
            {
                continue;
            }
            if(list->at(n).distArrival<minDist) minDist=list->at(n).distArrival;
            if(!windIsForced)
            {
                if(!grib->getInterpolatedValue_byDates((double) list->at(n).lon,(double) list->at(n).lat,
                       eta,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA)||eta+timeStep*60>maxDate)
                {
                    iso->setPointDead(n);
                    continue;
                }
                wind_angle=radToDeg(wind_angle);
            }
            nbNotDead++;
            iso->setPointWind(n,wind_angle,wind_speed);
        }
        if(nbNotDead==0) break;
        QList<vlmPoint> tempPoints;
        time.start();
        QList<QList<vlmPoint> > polarList;
        for(int n=0;n<list->count();n++)
        {
            if(list->at(n).isDead) continue;
            wind_angle=list->at(n).wind_angle;
            wind_speed=list->at(n).wind_speed;
            orth.setPoints(list->at(n).lon,list->at(n).lat,arrival.x(),arrival.y());
            float loxo=orth.getLoxoCap();
            QList<float> caps;
            QList<float> cap_left;
            QList<float> cap_right;
            float workAngleStep=0;
            float workAngleRange=0;
            if(list->at(n).isStart)
            {
                workAngleStep=angleStep;
                workAngleRange=angleRange;
            }
            else
            {
                /*force une convergence logarithmique vers l'arrivee. On utilise la distance au meilleur point pour donner une chance aux autres de se refaire, je me comprends*/
                workAngleRange=qMax((float)60.0,angleRange*(1/(1+log(initialDist/minDist))));
                /*une fois le 1er iso passe, on explore juste 10 caps a partir de chaque point---> a voir si c'est bon et pkoi 10 et pas 5 ou 3?*/
#if 0
                workAngleStep=qMax(workAngleRange/10,angleStep);
#else
                workAngleStep=angleStep;
#endif
            }
            for(float cc=0;true;cc=cc+workAngleStep)
            {
                if(cc>angleRange/2.0) cc=workAngleRange/2;
                cap_left.append(loxo-cc);
                cap_right.append(loxo+cc);
                if(cc>=workAngleRange/2.0) break;
            }
            for(int ccc=cap_left.count()-1;ccc>=0;ccc--)
                caps.append(cap_left[ccc]);
            for (int ccc=1;ccc<cap_right.count();ccc++)
                caps.append(cap_right[ccc]);
#if 1 /*systematically add BVMG in the list of headings to explore, who knows...might get lucky*/
            if(!list->at(n).isStart)
            {
                double vmg;
                myBoat->getPolarData()->getBvmg(A360(loxo-wind_angle),wind_speed,&vmg);
                caps.append(A360(vmg+wind_angle));
            }
#endif
            QList<vlmPoint> polarPoints;
#warning multiprocessing is probably doable here
            for(int ccc=caps.count()-1;ccc>=0;ccc--)
            {
                cap=caps[ccc];
                vlmPoint newPoint(0,0);
                findPoint(list->at(n).lon, list->at(n).lat, wind_angle, wind_speed, cap, &newPoint);
                QPointF P(newPoint.lon,newPoint.lat);
#if 0 //check that point doesn't belong to previous iso global shape
                if(isoShape->containsPoint(P,Qt::OddEvenFill)) continue;
#endif
#if 0
                //check that the shape made by origin, point, farAway does not cross previous iso global shape
                QPolygonF temp;
                temp.push_back(QPointF(iso->getPoint(n)->lon,iso->getPoint(n)->lat));
                temp.push_back(QPointF(newPoint.lon,newPoint.lat));
                temp.push_back(farAway);
                temp.push_back(QPointF(iso->getPoint(n)->lon,iso->getPoint(n)->lat));
                if(isoShape->intersected(temp).size()!=0) continue;
#endif
#if 1
                //check that the line (newpoint,arrival) does not cross previous iso
                QLineF temp(P,arrival);
                QPointF dummy(0,0);
                bool bad=false;
                if(!list->at(n).isStart)
                {
                    for (int i=0;i<previousIso.count()-1;i++)
                    {
                        QLineF s(previousIso.at(i),previousIso.at(i+1));
                        if(temp.intersect(s,&dummy)==QLineF::BoundedIntersection)
                        {
                            bad=true;
                            break;
                        }
                    }
                    if (bad) continue;
                }
#endif
#if 1
                //check that the new segment wont' cross any segments from previous iso
                QLineF s(newPoint.lon,newPoint.lat,list->at(n).lon,list->at(n).lat);
                bad=false;
                for(int i=0;i<previousSegments.count();i++)
                {
                    if (i==n) continue;
                    if(s.intersect(previousSegments.at(i),&dummy)==QLineF::BoundedIntersection)
                    {
                        bad=true;
                        break;
                    }
                }
                if(bad) continue;
#endif
                newPoint.origin=iso->getPoint(n);
                newPoint.originNb=n;
                newPoint.capOrigin=A360(cap);
                newPoint.eyeLon=start.x();
                newPoint.eyeLat=start.y();
                orth.setPoints(start.x(),start.y(),newPoint.lon,newPoint.lat);
                newPoint.distStart=orth.getDistance();
                if(list->at(n).isStart)
                    newPoint.distIso=newPoint.distStart;
                newPoint.capStart=A360(orth.getLoxoCap()+180);
                orth.setStartPoint(arrival.x(),arrival.y());
                newPoint.distArrival=orth.getDistance();
                newPoint.capArrival=A360(orth.getLoxoCap()+180);
                newPoint.distIso=findDistancePreviousIso(newPoint,&previousIso);
                polarPoints.append(newPoint);
            }
            qSort(polarPoints.begin(),polarPoints.end(),leftToRightFromOrigin);
            if(!polarPoints.isEmpty())
                polarList.append(polarPoints);
        }
        msecs_1=msecs_1+time.elapsed();
#if 0
/* epuration par sillage: si un pt se trouve dans le sillage d'un autre, il est elimine*/
        //Warning: not compatible with current method to clean crossing segments.
        for (int nn=0;nn<polarList.count();nn++)
            tempPoints=tempPoints+polartList.at(nn);
        if(!list->at(0).isStart)
        {
            qSort(tempPoints.begin(),tempPoints.end(),byDistanceArrival);
            pruneWake(&tempPoints,30,2);
#if 0
            pruneWake(&tempPoints,10,1);
#endif
        }
#endif
/*1eme epuration: on supprime les segments qui se croisent*/
#if 1 /*compare last point of one polar with the first of the next polar. If they cross delete the closest point from previous iso*/
        if(!list->at(0).isStart)
        {
            time.start();
            for (int nn=0;nn<polarList.count()-1;nn++)
            {
                if(polarList.at(nn).isEmpty()) continue;
                bool exist=false;
                int mm=0;
                for(mm=nn+1;mm<polarList.count();mm++)
                {
                    if(polarList.at(mm).isEmpty()) continue;
                    exist=true;
                    break;
                }
                if(!exist) continue;
                QList<vlmPoint> polarNN=polarList.at(nn);
                QList<vlmPoint> polarMM=polarList.at(mm);
                while(true)
                {
                    if(polarNN.isEmpty() || polarMM.isEmpty()) break;
                    QLineF S1(polarNN.last().origin->lon,polarNN.last().origin->lat,polarNN.last().lon,polarNN.last().lat);
                    QLineF S2(polarMM.first().origin->lon,polarMM.first().origin->lat,polarMM.first().lon,polarMM.first().lat);
                    QPointF crossPoint;
                    QLineF::IntersectType intersect=S1.intersect(S2,&crossPoint);
                    if(intersect!=QLineF::BoundedIntersection) /*some rocket science here*/
                    {
                        QPointF dummy;
                        QLineF S1bis=S1;
                        S1bis.setP2(crossPoint);
                        S1bis.setLength(S1bis.length()*2);
                        intersect=S1bis.intersect(S2,&dummy);
                        if(intersect!=QLineF::BoundedIntersection)
                        {
                            QLineF S2bis=S2;
                            S2bis.setP2(crossPoint);
                            S2bis.setLength(S2bis.length()*2);
                            intersect=S2bis.intersect(S1,&dummy);
                        }
                    }
                    if(intersect==QLineF::BoundedIntersection)
                    {
                        if(polarNN.last().distIso<polarMM.first().distIso)
                            polarNN.removeLast();
                        else
                            polarMM.removeFirst();
                    }
                    else
                        break;
                }
                polarList.replace(nn,polarNN);
                polarList.replace(mm,polarMM);
#if 0
                if(polarMM.isEmpty() && !polarNN.isEmpty())
                    nn--;
                if(polarNN.isEmpty())
                {
                    for (int i=nn-1;i>=0;i--)
                    {
                        if(polarList.at(i).isEmpty()) continue;
                        nn=i-1;
                        break;
                    }
                }
#else
                if(polarNN.isEmpty()||polarMM.isEmpty())
                    nn=-1; /*restart the loop completely, can to better than that (maybe)*/
#endif
            }
        }
        for (int nn=0;nn<polarList.count();nn++)
            tempPoints=tempPoints+polarList.at(nn);
        polarList.clear();
#endif
#if 0 /*remove remaining crossing segments*/
/*counts how many times a segment crosses others and remove first those which crosses more. For example if a segment crosses 4 other segments
it is pruned first than one which crosses only three times. In case 2 segments have the same number of crosses, we pruned first the farest from arrival*/
        qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
        QMultiMap<int,int> nbCross_1;
        QMultiMap<int,int> nbCross_2;
        QList<QList<int> > crossDetails;
        for (int nn=0;nn<tempPoints.count()-1;nn++)
        {
            if(tempPoints.at(nn).isDead) continue;
            QList<int> myCrossDetails;
            int myNbCross=0;
            for (int mm=0;mm<tempPoints.count()-1;mm++)
            {
                if(tempPoints.at(mm).isDead) continue;
                if(tempPoints.at(mm).originNb==tempPoints.at(nn).originNb) continue;
                int toBeKilled=0;
                if(intersects(&tempPoints, nn, mm, & toBeKilled))
                {
                    myNbCross++;
                    myCrossDetails.append(mm);
                }
            }
            nbCross_1.insert(myNbCross*100000+tempPoints.at(nn).distArrival,nn);
            nbCross_2.insert(nn,myNbCross*100000+tempPoints.at(nn).distArrival);
            crossDetails.append(myCrossDetails);
        }
        while(true)
        {
            QMapIterator<int,int> index(nbCross_1);
            index.toBack();
            index.previous();
            if(index.key()<=99999) break;
            vlmPoint temp=tempPoints.at(index.value());
            temp.isDead=true;
            tempPoints.replace(index.value(),temp);;
            QList<int> myCrossDetails=crossDetails.at(index.value());
            for(int n=0;n<myCrossDetails.count();n++)
            {
                int segNbCross=nbCross_2.value(myCrossDetails.at(n));
                nbCross_2.replace(myCrossDetails[n],segNbCross-100000);
                nbCross_1.remove(segNbCross,myCrossDetails[n]);
                nbCross_1.insert(segNbCross-100000,myCrossDetails[n]);
            }
            nbCross_1.remove(index.key(),index.value());
            nbCross_2.remove(index.value(),index.key());
        }
        for (int nn=tempPoints.count()-1;nn>=0;nn--)
        {
            if(tempPoints.at(nn).isDead) tempPoints.removeAt(nn);
        }
        crossDetails.clear();
        nbCross_1.clear();
        nbCross_2.clear();
#endif
#if 0 //when 2 segments crosses, just discard the worst point
        qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
        for (int nn=0;nn<tempPoints.count()-1;nn++)
        {
            if(tempPoints.at(nn).isDead) continue;
            for (int mm=0;mm<tempPoints.count()-1;mm++)
            {
                if(tempPoints.at(mm).isDead) continue;
                if(tempPoints.at(mm).originNb==tempPoints.at(nn).originNb) continue;
                int toBeKilled=0;
                if(intersects(&tempPoints, nn, mm, & toBeKilled))
                {
                    if(toBeKilled==1)
                    {
                        vlmPoint temp=tempPoints.at(nn);
                        temp.isDead=true;
                        tempPoints.replace(nn,temp);;
                        break;
                    }
                    else if(toBeKilled==2)
                    {
                        vlmPoint temp=tempPoints.at(mm);
                        temp.isDead=true;
                        tempPoints.replace(mm,temp);;
                    }
                    else if(tempPoints.at(nn).distArrival>tempPoints.at(mm).distArrival)
                    {
                        vlmPoint temp=tempPoints.at(nn);
                        temp.isDead=true;
                        tempPoints.replace(nn,temp);;
                        break;
                    }
                    else
                    {
                        vlmPoint temp=tempPoints.at(mm);
                        temp.isDead=true;
                        tempPoints.replace(mm,temp);;
                    }
                }
            }
        }
        for (int nn=tempPoints.count()-1;nn>=0;nn--)
        {
            if(tempPoints.at(nn).isDead) tempPoints.removeAt(nn);
        }
#endif
        msecs_2=msecs_2+time.elapsed();
#if 1
        time.start();
        //Check that no segment is crossing it's own isochron. If this is the case remove worst point
        qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
        int nn=0;
        int mm=0;
        int kk=0;
#if 0
        /*optimisation: find 1st 2 points not dead before nn and test just that. For instance if nn==4 test only (2,3). Do the same for after nn*/
        /*Not covering every case... can do better here*/
        for(nn=2;nn<tempPoints.count();nn++)
        {
            bool foundCross=false;
            if(tempPoints.at(nn).isDead) continue;
            QLineF S1(tempPoints.at(nn).lon,tempPoints.at(nn).lat,tempPoints.at(nn).origin->lon,tempPoints.at(nn).origin->lat);
            QPointF crossPoint(0,0);
            for(mm=nn-1;mm>=0;mm--)
            {
                if(tempPoints.at(mm).isDead) continue;
                for(kk=mm-1;kk>=0;kk--)
                {
                    if(tempPoints.at(kk).isDead) continue;
                    QLineF S2(tempPoints.at(mm).lon,tempPoints.at(mm).lat,tempPoints.at(kk).lon,tempPoints.at(kk).lat);
                    if(S1.intersect(S2,&crossPoint)==QLineF::BoundedIntersection)
                    {
                        foundCross=true;
                    }
                    break;
                }
                break;
            }
            if(!foundCross)
            {
                for(mm=nn+1;mm<tempPoints.count()-1;mm++)
                {
                    if(tempPoints.at(mm).isDead) continue;
                    for(kk=mm+1;kk<tempPoints.count();kk++)
                    {
                        if(tempPoints.at(kk).isDead) continue;
                        QLineF S2(tempPoints.at(mm).lon,tempPoints.at(mm).lat,tempPoints.at(kk).lon,tempPoints.at(kk).lat);
                        if(S1.intersect(S2,&crossPoint)==QLineF::BoundedIntersection)
                        {
                            foundCross=true;
                        }
                        break;
                    }
                    break;
                }
            }
#else
#warning Optimisable here
        for(nn=0;nn<tempPoints.count();nn++)
        {
            if(tempPoints.at(nn).isDead) continue;
            QLineF S1(tempPoints.at(nn).lon,tempPoints.at(nn).lat,tempPoints.at(nn).origin->lon,tempPoints.at(nn).origin->lat);
            bool foundCross=false;
            for(mm=0;mm<tempPoints.count()-1;mm++)
            {
                if(mm==nn) continue;
                if(tempPoints.at(mm).isDead) continue;
                for (kk=mm+1;kk<tempPoints.count();kk++)
                {
                    if(tempPoints.at(kk).isDead) continue;
                    if (kk==nn) break;
                    QPointF crossPoint;
                    QLineF S2(tempPoints.at(mm).lon,tempPoints.at(mm).lat,tempPoints.at(kk).lon,tempPoints.at(kk).lat);
                    if(S1.intersect(S2,&crossPoint)==QLineF::BoundedIntersection)
                    {
                        foundCross=true;
                    }
                    break;
                }
                if(foundCross) break;
            }
#endif
            if(foundCross)
            {
                int badOne=0;
                if(tempPoints.at(nn).distIso<tempPoints.at(mm).distIso)
                    badOne=nn;
                else
                    badOne=mm;
                if(tempPoints.at(kk).distIso<tempPoints.at(badOne).distIso)
                    badOne=kk;
                vlmPoint temp=tempPoints.at(badOne);
                temp.isDead=true;
                tempPoints.replace(badOne,temp);
            }
        }
        for (int nn=tempPoints.count()-1;nn>=0;nn--)
        {
            if(tempPoints.at(nn).isDead) tempPoints.removeAt(nn);
        }
        msecs_4=msecs_4+time.elapsed();
#endif
#if 1 /*smoothing the isoline*/
        time.start();
        for(int smoothPass=1;smoothPass<=1;smoothPass++)
        {
            for(nn=1;nn<tempPoints.count()-1;nn++)
            {
                if(tempPoints.at(nn).isDead) continue;
                bool first=true;
                for(mm=nn-1;mm>=0;mm--)
                {
                    if(tempPoints.at(mm).isDead) continue;
                    first=false;
                    break;
                }
                if (first) continue;
                if(tempPoints.at(mm).distIso>tempPoints.at(nn).distIso && tempPoints.at(nn).distIso<tempPoints.at(nn+1).distIso)
                {
                    vlmPoint temp=tempPoints.at(nn);
                    temp.isDead=true;
                    tempPoints.replace(nn,temp);
                }
            }
            for (int nn=tempPoints.count()-1;nn>=0;nn--)
            {
                if(tempPoints.at(nn).isDead) tempPoints.removeAt(nn);
            }
        }
        msecs_5=msecs_5+time.elapsed();
#endif
/*epuration finale, on ne garde que le nombre initial de points + le coeff d'exploration (e.g. 120/10+2=14 pts)*/
        time.start();
        int limit=(this->angleRange/this->angleStep)+this->explo;
        int c=tempPoints.count();
        int toBeRemoved=c-limit;
//        qWarning()<<"start cleaning"<<tempPoints.count()<<"points. Limit is"<<limit;
        if(tempPoints.count()>limit)
        {
#if 0
/* Method 1: just take them out at regular interval */
            int n=0;
            float m=0;
            float inc=(float) c/(float) toBeRemoved;
            for (int i=0;i<toBeRemoved;i++)
            {
                m=m+inc;
                if(qRound(m)<c)
                {
                    tempPoints.at(qRound(m)).isDead=true;
                    n++;
                }
            }
#endif
#if 0
/* Method 2: we first identify the root with most children, then we kill the child which is the farest from arrival
   if there is the same number of children then the root farest from destination is pruned first*/
            qSort(tempPoints.begin(),tempPoints.end(),byOrigin);
            QMultiMap<int,int> rootNb;
            QMultiMap<int,int> rootMap;
            int indexRoot=0;
            int count=0;
            vlmPoint lastOrig(tempPoints.at(0).origin->lon,tempPoints.at(0).origin->lat);
            lastOrig.distArrival=tempPoints.at(0).distArrival;
            for(int n=0;n<tempPoints.count();n++)
            {
                if(tempPoints.at(n).origin->lon!=lastOrig.lon && tempPoints.at(n).origin->lat!=lastOrig.lat)
                {
                    rootNb.insert(count*100000+lastOrig.distArrival,indexRoot);
                    lastOrig.lon=tempPoints.at(n).origin->lon;
                    lastOrig.lat=tempPoints.at(n).origin->lat;
                    lastOrig.distArrival=tempPoints.at(n).distArrival;
                    indexRoot++;
                    count=0;
                }
                rootMap.insert(indexRoot,n);
                count++;
            }
            rootNb.insert(count*100000+lastOrig.distArrival,indexRoot);
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
                QMultiMap<float,int> dist;
                for(int n=0;n<children.size();n++)
                {
                    dist.insert(tempPoints.at(children.at(n)).distArrival,children.at(n));
                }
                QMapIterator<float,int> distIndex(dist);
                distIndex.toBack();
                while(distIndex.hasPrevious())
                {
                    int debug1=distIndex.previous().value();
                    float debug2=distIndex.key();
                    qWarning()<<debug2<<debug1;
                }
                distIndex.toBack();
                int indice=distIndex.previous().value();
                vlmPoint temp=tempPoints.at(indice);
                temp.isDead=true;
                tempPoints.replace(indice,temp);
                toBeRemoved--;
                rootMap.remove(indexRoot,indice);
                rootNb.remove(keyRoot,indexRoot);
                rootNb.insert(keyRoot-100000,indexRoot);
            }
            qSort(tempPoints.begin(),tempPoints.end(),leftToRight);
#endif
#if 1
/* Method 3: in this solution we sort each couple of adjacent iso points by the distance between them,
   then we kill one of the point belonging to the first couples, based on the distance from previous iso shape*/
            while(toBeRemoved>0)
            {
                QMultiMap<float,int> byDistances;
                QMultiMap<int,float> byIndex;
                for(int n=0;n<tempPoints.size()-1;n++)
                {
                    float length=QLineF(QPointF(tempPoints.at(n).lon,tempPoints.at(n).lat),QPointF(tempPoints.at(n+1).lon,tempPoints.at(n+1).lat)).length();
                    byDistances.insert(length,n);
                    byIndex.insert(n,length);
                }
                QMapIterator<float,int> d(byDistances);
                int coupleNb=d.next().value();
#if 1
                if(tempPoints.at(coupleNb).distIso<tempPoints.at(coupleNb+1).distIso)
                    tempPoints.removeAt(coupleNb);
                else
                    tempPoints.removeAt(coupleNb+1);
#endif
#if 0
// playing with distStart and distArrival
                if(tempPoints.at(coupleNb).distArrival<tempPoints.at(coupleNb).distStart)
                {
                    if(tempPoints.at(coupleNb).distArrival>tempPoints.at(coupleNb+1).distArrival)
                        tempPoints.removeAt(coupleNb);
                    else
                        tempPoints.removeAt(coupleNb+1);
                }
                else
                {
                    if(tempPoints.at(coupleNb).distStart>tempPoints.at(coupleNb+1).distStart)
                        tempPoints.removeAt(coupleNb);
                    else
                        tempPoints.removeAt(coupleNb+1);
                }
#endif
#if 0
//here we remove the one that is closest from next point. Exemple: if couple3 has the smallest distance, then we look at couple2 and couple4
//if couple2 has a smallest distance than couple 4, then we remove point 3 otherwise we remove point 4
                if(tempPoints.at(coupleNb).distArrival>tempPoints.at(coupleNb).distStart/2)
                {
                    if(coupleNb==0)
                        tempPoints.removeAt(1);
                    else if (coupleNb==tempPoints.count()-2)
                        tempPoints.removeAt(tempPoints.count()-3);
                    else
                    {
                        if(byIndex.value(coupleNb)>byIndex.value(coupleNb+1))
                            tempPoints.removeAt(coupleNb);
                        else
                            tempPoints.removeAt(coupleNb+1);
                    }
                }
                else
                {
                    if(tempPoints.at(coupleNb).distArrival>tempPoints.at(coupleNb+1).distArrival)
                        tempPoints.removeAt(coupleNb);
                    else
                        tempPoints.removeAt(coupleNb+1);
                }
#endif
                toBeRemoved--;
            }
#endif
        }
        msecs_6=msecs_6+time.elapsed();
        previousIso.clear();
        if(tempPoints.count()>0)
        {
            for (int n=0;n<tempPoints.count();n++)
            {
                if(tempPoints.at(n).isDead) continue;
                currentIso->addVlmPoint(tempPoints[n]);
                previousIso.append(QPointF(tempPoints.at(n).lon,tempPoints.at(n).lat));
            }
        }
        list = currentIso->getPoints();
        iso=currentIso;
        if(ncolor>=colorsList.count()) ncolor=0;
        pen.setColor(colorsList[ncolor]);
        pen.setBrush(colorsList[ncolor]);
        iso->setLinePen(pen);
        ncolor++;
        previousSegments.clear();
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
            previousSegments.append(QLineF(list->at(n).origin->lon,list->at(n).origin->lat,list->at(n).lon,list->at(n).lat));
        }
        iso->slot_showMe();
        QCoreApplication::processEvents();
        isochrones.append(iso);
        nbIso++;
//        qWarning()<<"Isochrone Nb"<<nbIso<<"has"<<list->count()<<"points";
#if 0
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
//        if(isoShape->containsPoint(arrival,Qt::OddEvenFill)||list->count()==0)
//        {
//            arrived=true;
//            iso=isochrones.at(isochrones.count()-2);
//        }
#endif
        eta=eta+(int)timeStep*60.00;
        vlmPoint to(arrival.x(),arrival.y());
        time.start();
        for (int n=0;n<list->count();n++)
        {
            vlmPoint from=list->at(n);
            int thisTime=calculateTimeRoute(from,to,(timeStep+1)*60);
            if(thisTime<=timeStep*60)
            {
                arrived=true;
                break;
            }
        }
        msecs_7=msecs_7+time.elapsed();
        if(nbIso>3000 || arrived || nbNotDead==0)
        {
            break;
        }
    }
    list=iso->getPoints();
    orth.setEndPoint(arrival.x(),arrival.y());
    int nBest=0;
    if(arrived)
    {
        int minTime=timeStep*10000*60;
        vlmPoint to(arrival.x(),arrival.y());
        for(int n=0;n<list->count();n++)
        {
            vlmPoint from=list->at(n);
            int thisTime=calculateTimeRoute(from,to);
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
        vlmPoint to(arrival.x(),arrival.y());
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
    QTime tt(0,0,0,0);
    int msecs=timeTotal.elapsed();
    tt=tt.addMSecs(msecs);
    qWarning()<<"Total calculation time:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_1);
    qWarning()<<"...Calculating iso points:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_2);
    qWarning()<<"...removing crossed segments:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_3);
    qWarning()<<"...calculating distance from previous iso:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_4);
    qWarning()<<"...removing segments crossing their own isochron:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_5);
    qWarning()<<"...smoothing iso:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_6);
    qWarning()<<"...final cleaning:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs_7);
    qWarning()<<"...checking if arrived:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    tt.setHMS(0,0,0,0);
    tt=tt.addMSecs(msecs-(msecs_1+msecs_2+msecs_3+msecs_4+msecs_5+msecs_6));
    qWarning()<<"...sum of other calculations:"<<tt.toString("hh'h'mm'min'ss.zzz'secs'");
    if(arrived)
    {
        QDateTime finalEta=QDateTime::fromTime_t(eta).toUTC();
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("Fin de routage : ")+finalEta.toString("dd MMM-hh:mm"));
        msgBox.exec();
    }
    if(this->useTrueVacation)
    {
        qWarning()<<"---Newton-Raphson Statistics---";
        qWarning()<<"Total number of route segments calculated"<<routeN;
        qWarning()<<"Average number of iterations"<<(float)routeTotN/(float)routeN;
        qWarning()<<"Max number of iterations made to find a solution"<<routeMaxN;
        qWarning()<<"Number of iterations failure"<<routeFailedN;
        qWarning()<<"-------------------------------";
    }
    if (!this->showIso)
        setShowIso(showIso);
    this->done=true;
    if(isConverted())
        convertToRoute();
}
float ROUTAGE::findDistancePreviousIso(vlmPoint P, QPolygonF * isoShape)
{
    //QLineF line(start.x(),start.y(),P.lon,P.lat);
    QLineF line(P.lon,P.lat,arrival.x(),arrival.y());
    line.setAngle(line.angle()+180);
    line.setLength(P.distStart*2);
    QPointF result(0,0);
    for(int i=0;i<isoShape->count()-1;i++)
    {
        QLineF s(isoShape->at(i),isoShape->at(i+1));
        if(line.intersect(s,&result)==QLineF::BoundedIntersection)
        {
            QLineF isoDist(result.x(),result.y(),P.lon,P.lat);
            return isoDist.length();
        }
    }
    return P.distStart;
}
void ROUTAGE::pruneWake(QList<vlmPoint> * tempPoints,int wakeAngle,int mode)
{
//    qWarning()<<"start pruning "<<tempPoints->count()<<" points";
    for(int n=0;n<tempPoints->count();n++)
    {
        if(tempPoints->at(n).isDead) continue;
        for(int m=0;m<tempPoints->count();m++)
        {
            if(m==n) continue;
            if(tempPoints->at(m).isDead) continue;
            if(tempPoints->at(m).distArrival>tempPoints->at(n).distArrival) continue;
            float wakeDir=0;
            float wakeDist=0;
            if(mode==1)
            {
                wakeDist=tempPoints->at(m).distStart;
                wakeDir=A360(this->loxoCap+180);
            }
            else
            {
                wakeDist=tempPoints->at(m).distOrigin;
                if(tempPoints->at(m).distStart>tempPoints->at(m).distArrival)
                    wakeDir=A360(tempPoints->at(m).capStart);
                else
                    wakeDir=A360(tempPoints->at(m).capArrival+180);
            }
            QPolygonF wake;
            double res_lat,res_lon;
            wake.push_back(QPointF(tempPoints->at(m).lon,tempPoints->at(m).lat));
            Util::getCoordFromDistanceAngle(tempPoints->at(m).lat, tempPoints->at(m).lon, wakeDist, A360(wakeDir+wakeAngle), &res_lat, &res_lon);
            wake.push_back(QPointF(res_lon,res_lat));
            Util::getCoordFromDistanceAngle(tempPoints->at(m).lat, tempPoints->at(m).lon, wakeDist, A360(wakeDir-wakeAngle), &res_lat, &res_lon);
            wake.push_back(QPointF(res_lon,res_lat));
            wake.push_back(QPointF(tempPoints->at(m).lon,tempPoints->at(m).lat));
            if(wake.containsPoint(QPointF(tempPoints->at(n).lon,tempPoints->at(n).lat),Qt::OddEvenFill))
            {
                vlmPoint temp=tempPoints->at(n);
                temp.isDead=true;
                tempPoints->replace(n,temp);
                break;
            }
        }
    }
    for(int n=tempPoints->count()-1;n>=0;n--)
    {
        if(tempPoints->at(n).isDead)
            tempPoints->removeAt(n);
    }
    //qWarning()<<"exit from pruning "<<tempPoints->count()<<" remaining points";
}
bool ROUTAGE::intersects(QList<vlmPoint> *list, int nn, int mm,int *toBeKilled)
{
    vlmPoint P1=list->at(nn);
    vlmPoint P2=list->at(mm);
    if(P1.originNb==P2.originNb) return false;
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
    cap=A360(cap);
    float angle,newSpeed;
    double res_lon,res_lat;
    angle=cap-(float)wind_angle;
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    float limit=myBoat->getBvmgUp(wind_speed);
    if(qAbs(angle)<limit) //if too close to wind then use VB-VMG technique
    {
        newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,limit);
        newSpeed=newSpeed*cos(degToRad(limit-qAbs(angle)));
    }
    else
    {
        limit=myBoat->getBvmgDown(wind_speed);
        if(qAbs(angle)>limit)
        {
            newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,limit);
            newSpeed=newSpeed*cos(degToRad(limit-qAbs(angle)));
        }
        else
            newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,angle);
    }
    float distanceParcourue=0;
    distanceParcourue=newSpeed*timeStep/60.0;
    Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap, &res_lat, &res_lon);
    pt->lon=res_lon;
    pt->lat=res_lat;
    pt->distOrigin=distanceParcourue;
    if(!this->useTrueVacation || windIsForced) return;
    vlmPoint from(lon,lat);
    vlmPoint to(res_lon,res_lat);
    int realTime=calculateTimeRoute(from,to);
    if(realTime==0)
    {
        qWarning()<<"out of grib or something while computing in route-mode";
        return;
    }
    int timeStepSec=timeStep*60;
    if(realTime==timeStepSec)
    {
        pt->lon=lastLonFound;
        pt->lat=lastLatFound;
        return;
    }
    int minDiff=qAbs(realTime-timeStepSec);
    float bestDist=distanceParcourue;
    bool found=false;
    int n=1;
    int oldTime=timeStepSec;
    /*first, trying to be clever*/
    float newDist=distanceParcourue*oldTime/realTime;
    Util::getCoordFromDistanceAngle(lat, lon, newDist, cap, &res_lat, &res_lon);
    to.lon=res_lon;
    to.lat=res_lat;
    oldTime=realTime;
    realTime=calculateTimeRoute(from,to);
    if(realTime==0)
    {
        qWarning()<<"out of grib or something while computing in route-mode";
    }
    if(realTime==timeStepSec)
    {
        pt->distOrigin=newDist;
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
        float x=distanceParcourue;
        float term=0;
        from.capOrigin=cap;
        for (n=2;n<=100;n++)
        {
            float y=routeFunction(x,from);
            if(qAbs(y)<=timeStepSec/2)
            {
                found=true;
                pt->distOrigin=x;
                break;
            }
            float deriv=routeFunctionDeriv(x,from);
            if (deriv==0)
            {
#if 0
                qWarning()<<"Flat spot in Newton-raphson! x="<<x<<"distanceParcourue="<<distanceParcourue<<"realTime="<<realTime<<"oldTime="<<oldTime;
#endif
                bestDist=distanceParcourue;
                break; /*flat spot, there is no solution*/
            }
            term=y/deriv;
            x=qAbs(x-term);
        }
#endif
    }
    routeN++;
    if(found)
    {
        if(n>routeMaxN)
            routeMaxN=n;
        routeTotN=routeTotN+n;
        pt->lon=lastLonFound;
        pt->lat=lastLatFound;
    }
    else
    {
        pt->distOrigin=bestDist;
        Util::getCoordFromDistanceAngle(lat, lon, bestDist, cap, &res_lat, &res_lon);
        routeFailedN++;
        pt->lon=res_lon;
        pt->lat=res_lat;
    }
}
int ROUTAGE::routeFunction(float x,vlmPoint from)
{
    double res_lon,res_lat;
    Util::getCoordFromDistanceAngle(from.lat, from.lon, x, from.capOrigin, &res_lat, &res_lon);
    vlmPoint to(res_lon,res_lat);
    return calculateTimeRoute(from,to)-timeStep*60;
}
int ROUTAGE::routeFunctionDeriv(float x,vlmPoint from)
{
    float minGap=x/10;
    int   yr,yl;
    float xl,xr;
    for(int n=1;n<20;n++) /*bad trick to avoid flat spots*/
    {
        xr=x+minGap;
        xl=qMax(x/10,x-minGap);
        yr=routeFunction(xr,from);
        yl=routeFunction(xl,from);
        if(yr!=yl) break;
        minGap=minGap+x/10;
    }
#if 0
    if(yr==yl)
        qWarning()<<"couldn't avoid flat spot"<<"xr="<<xr<<"xl="<<xl<<"x="<<x;
#endif
    return((yr-yl)/(xr-xl));
}
int ROUTAGE::calculateTimeRoute(vlmPoint routeFrom,vlmPoint routeTo,int limit)
{
        time_t etaRoute=this->eta;
        time_t etaLimit=eta*2;
        bool hasLimit=false;
        if(limit!=-1)
        {
            hasLimit=true;
            etaLimit=etaRoute+(limit*2);
        }
        bool has_eta=true;
        Orthodromie orth(0,0,0,0);
        double lon,lat;
        lon=routeFrom.lon;
        lat=routeFrom.lat;
        double newSpeed,distanceParcourue,remaining_distance,res_lon,res_lat,previous_remaining_distance,cap1,cap2,diff1,diff2;
        double wind_angle,wind_speed,cap,angle;
        time_t maxDate=grib->getMaxDate();
        newSpeed=0;
        distanceParcourue=0;
        res_lon=0;
        res_lat=0;
        previous_remaining_distance=0;
        wind_angle=0;
        wind_speed=0;
        orth.setPoints(lon, lat, routeTo.lon,routeTo.lat);
        remaining_distance=orth.getDistance();
        do
            {
                if(grib->getInterpolatedValue_byDates(lon, lat, etaRoute,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA) && etaRoute<=maxDate && (!hasLimit || etaRoute<=etaLimit))
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
                    newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,angle);
                    distanceParcourue=newSpeed*myBoat->getVacLen()/3600.00;
                    Util::getCoordFromDistanceAngle(lat, lon, distanceParcourue, cap,&res_lat,&res_lon);
                    orth.setStartPoint(res_lon, res_lat);
                    remaining_distance=orth.getDistance();
                    lon=res_lon;
                    lat=res_lat;
                    if(remaining_distance>previous_remaining_distance)
                    {
                        etaRoute= etaRoute + myBoat->getVacLen();
                        break;
                    }
                    etaRoute= etaRoute + myBoat->getVacLen();
                }
                else
                {
#if 0
                    if(!grib->getInterpolatedValue_byDates(lon, lat, etaRoute,&wind_speed,&wind_angle,INTERPOLATION_SELECTIVE_TWSA))
                        qWarning()<<"out of grib!!";
                    if(etaRoute>maxDate)
                        qWarning()<<"out of gribDate!"<<QDateTime::fromTime_t(etaRoute).toString("dd/MM/yy hh:mm:ss")<<QDateTime::fromTime_t(maxDate).toString("dd/MM/yy hh:mm:ss")<<QDateTime::fromTime_t(this->eta).toString("dd/MM/yy hh:mm:ss")<<orth.getDistance();
#endif
                    has_eta=false;
                }
            } while (remaining_distance>distanceParcourue/2.000 && has_eta);
        if(!has_eta)
        {
            if(hasLimit)
                return 10e5;
        }
        lastLonFound=lon;
        lastLatFound=lat;
        return(etaRoute-eta);
    }
float ROUTAGE::findTime(const vlmPoint * pt, QPointF P, float * cap)
{
    float angle,newSpeed,lon,lat,wind_speed,wind_angle;
    lon=P.x();
    lat=P.y();
    wind_speed=pt->wind_speed;
    wind_angle=pt->wind_angle;
    Orthodromie orth(pt->lon,pt->lat,lon,lat);
    *cap=orth.getLoxoCap();
    angle=*cap-(float)wind_angle;
    if(qAbs(angle)>180)
    {
        if(angle<0)
            angle=360+angle;
        else
            angle=angle-360;
    }
    if(qAbs(angle)<myBoat->getBvmgUp(wind_speed)) //if too close to wind then use BVMG technique
    {
        newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,myBoat->getBvmgUp(wind_speed));
        newSpeed=newSpeed*cos(degToRad(myBoat->getBvmgUp(wind_speed)-qAbs(angle)));
    }
    else if(qAbs(angle)>myBoat->getBvmgDown(wind_speed))
    {
        newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,myBoat->getBvmgDown(wind_speed));
        newSpeed=newSpeed*cos(degToRad(qAbs(angle)-myBoat->getBvmgDown(wind_speed)));
    }
    else
        newSpeed=myBoat->getPolarData()->getSpeed(wind_speed,angle);
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
    route->setBoat(this->myBoat);
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
       POI * poi = parent->slot_addPOI(poiName,0,list->at(n).lat,list->at(n).lon,-1,false,false,myBoat);
       poi->setRoute(route);
    }
    delete result;
    result=NULL;
    route->setHidePois(true);
    route->setFrozen(false);
}
bool ROUTAGE::tooFar(vlmPoint point)
{
    QLineF S1(arrival,start);
    QLineF S2(arrival.x(),arrival.y(),point.lon,point.lat);
    QLineF S3(start,arrival);
    QLineF S4(start.x(),start.y(),point.lon,point.lat);
    float a1=qAbs(S2.angleTo(S1));
    float a2=qAbs(S4.angleTo(S3));

    if(a1>180) a1=360-a1;
    if(a2>180) a2=360-a2;
    if(a1>a2)
        return true;
    else
        return false;
}
