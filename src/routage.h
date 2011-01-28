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
#ifndef ROUTAGE_H
#define ROUTAGE_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QMenu>
#include <QDateTime>
#include <cmath>

#include "class_list.h"

#include "Polygon.h"
#include "vlmpointgraphic.h"
#include "vlmPoint.h"
#define NO_CROSS 1
#define BOUNDED_CROSS 2
#define L1_CROSS 3
#define L2_CROSS 4
struct datathread
{
    time_t Eta;
    Grib *GriB;
    bool whatIfUsed;
    time_t whatIfJour;
    bool windIsForced;
    int whatIfTime;
    float whatIfWind;
    float windSpeed;
    float windAngle;
    boat *Boat;
    int timeStep;
};

//===================================================================
class ROUTAGE : public QObject
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        ROUTAGE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow);

        ~ROUTAGE();
        void setName(QString name){this->name=name;}
        QString getName() {return(this->name);}

        void setBoat(boat *myBoat);
        boat * getBoat(){return this->myBoat;}

        void setColor(QColor color);
        QColor getColor(){return this->color;}

        void setWidth(float width);
        float getWidth() {return this->width;}

        void setWindIsForced(bool b){this->windIsForced=b;}
        void setWind(float twd, float tws){this->wind_angle=twd;this->wind_speed=tws;}
        float getWindAngle(void){return this->wind_angle;}
        float getWindSpeed(void){return this->wind_speed;}
        void setAngleRange(float a) {this->angleRange=a;}
        float getAngleRange(void){return this->angleRange;}
        void setAngleStep(float a) {this->angleStep=a;}
        float getAngleStep(void){return this->angleStep;}
        void setTimeStep(float t) {this->timeStep=t;}
        float getTimeStep(void){return this->timeStep;}
        bool getWindIsForced(void){return this->windIsForced;}
        bool getShowIso(void){return this->showIso;}
        void setShowIso(bool b);
        void setExplo(float e){this->explo=e;}
        float getExplo(void){return explo;}
        bool getUseRouteModule(void){return useRouteModule;}
        void setUseRouteModule(bool b){this->useRouteModule=b;}
        bool getRouteFromBoat(){return this->routeFromBoat;}
        void setRouteFromBoat(bool b){this->routeFromBoat=b;}

        void setStartTime(QDateTime date){this->startTime=date;}
        QDateTime getStartTime() {return this->startTime.toUTC();}
        void setWhatIfDate(QDateTime date){this->whatIfDate=date;}
        QDateTime getWhatIfDate() {return this->whatIfDate.toUTC();}
        bool getWhatIfUsed(void){return this->whatIfUsed;}
        void setWhatIfUsed(bool b){this->whatIfUsed=b;}
        int getWhatIfTime(void){return this->whatIfTime;}
        void setWhatIfTime(int t){this->whatIfTime=t;}
        int getWhatIfWind(void){return this->whatIfWind;}
        void setWhatIfWind(int p){this->whatIfWind=p;}

        static bool myLessThan(ROUTAGE * ROUTAGE_1,ROUTAGE* ROUTAGE_2) {return ROUTAGE_1->name < ROUTAGE_2->name;}
        static bool myEqual(ROUTAGE * ROUTAGE_1,ROUTAGE* ROUTAGE_2) {return ROUTAGE_1->name == ROUTAGE_2->name;}
        double cLFA(double cx);

        void setFromPOI(POI *poi){this->fromPOI=poi;}
        POI * getFromPOI(){return this->fromPOI;}
        void setToPOI(POI *poi){this->toPOI=poi;}
        POI * getToPOI(){return this->toPOI;}
        void calculate();
        bool isDone(void){return this->done;}
        bool isConverted(void) {return this->converted;}
        void setConverted(void) {this->converted=true;}
        void convertToRoute(void);
        QDateTime getFinalEta(void){return finalEta;}
        bool getCheckCoast(){return this->checkCoast;}
        void setCheckCoast(bool b){this->checkCoast=b;}
        bool isRunning(){return this->running;}
        void setFromRoutage(ROUTAGE * fromRoutage,bool editOptions);
        vlmPoint getPivotPoint(){return pivotPoint;}
        vlmLine * getWay(){return way;}
        void setPivotPoint(int isoNb,int pointNb);
        void showContextMenu(int isoNb,int pointNb);
        bool getIsNewPivot(){return isNewPivot;}
        void setIsNewPivot(bool b){isNewPivot=b;}
        bool getIsPivot(){return this->isPivot;}
        void setPoiPrefix(QString s){this->poiPrefix=s;}
        QString getPoiPrefix(){return this->poiPrefix;}
/*beta testing advanced parameters*/
        int  pruneWakeAngle;
        bool useConverge;
        time_t getEta(){return eta;}
        Grib * getGrib(){return grib;}
        time_t getWhatIfJour(){return whatIfJour;}
        static int calculateTimeRouteThreaded(vlmPoint RouteFrom,vlmPoint routeTo,float * lastLonFound, float * lastLatFound, datathread *dataThread);
        static int routeFunctionThreaded(float x,vlmPoint from, float * lastLonFound, float * lastLatFound, datathread *dataThread);
        static int routeFunctionDerivThreaded(float x,vlmPoint from, float * lastLonFound, float * lastLatFound, datathread *dataThread);
        static float A360(float hdg);
        static float myDiffAngle(float a1,float a2);
        bool getUseMultiThreading(){return this->useMultiThreading;}
        void setUseMultiThreading(bool b){this->useMultiThreading=b;}
        vlmLine * getResult(){return result;}
        ROUTAGE * getFromRoutage(){return fromRoutage;}
        bool getAutoZoom(){return autoZoom;}
        void setAutoZoom(bool b){this->autoZoom=b;}
    public slots:
        void slot_edit();
        void slot_abort(){this->aborted=true;}
        void slot_createPivot();
        void slot_createPivotM();
        void slot_drawWay();
        void eraseWay();
    signals:
        void editMe(ROUTAGE *);
    private:
        /* parent, main */
        myCentralWidget *parent;
        QGraphicsScene *myscene;
        Projection *proj;

        /* widget component */
        QColor color;
        float width;
        vlmLine *iso;
        QPen pen;

        /* data */
        QString name;
        POI * fromPOI;
        POI * toPOI;
        QList<vlmLine *> isochrones;
        QList<vlmLine *> segments;
        boat *myBoat;
        Grib *grib;
        float angleRange;
        float angleStep;
        float timeStep;
        int explo;
        bool showIso;
        QDateTime startTime;
        bool useRouteModule;


        /*various*/
        QPointF start;
        QPointF arrival;
        float mySignedDiffAngle(float a1,float a2);
        bool findPoint(float lon, float lat, double wind_angle, double wind_speed, float cap, vlmPoint *pt);
        vlmPoint findRoute(const vlmPoint  & point);
        float findTime(const vlmPoint * pt, QPointF P, float * cap);
        float loxoCap;
        float initialDist;


//        vlmPoint findRouteThreaded(const vlmPoint  & point);




        bool arrived;
        bool windIsForced;
        double wind_speed;
        double wind_angle;
        time_t eta;

        vlmLine * result;
        vlmLine * way;
        void drawResult(vlmPoint P);
        bool intersects(QList<vlmPoint> *iso,int nn,int mm,int * toBeKilled);
        bool done;
        bool converted;
        float findDistancePreviousIso(vlmPoint P,QPolygonF * isoShape);
        void pruneWake(int wakeAngle);
        int calculateTimeRoute(vlmPoint RouteFrom,vlmPoint routeTo,int limit=-1);
        int routeFunction(float x,vlmPoint from);
        int routeFunctionDeriv(float x,vlmPoint from);
        int routeN;
        int routeMaxN;
        int routeTotN;
        int routeFailedN;
        int NR_n;
        int NR_success;
        double lastLonFound;
        double lastLatFound;
        QDateTime finalEta;
        int msecsD1;
        int msecsD2;
        QList<vlmPoint> tempPoints;
        QPolygonF previousIso;
        QList<QLineF> previousSegments;
        bool somethingHasChanged;
        void checkSegmentCrossingOwnIso();
        void checkIsoCrossingPreviousSegments();
        void epuration(int toBeRemoved);
        void finalEpuration(int toBeRemoved,QList<vlmPoint> *listPoints);
        void removeCrossedSegments();
        float xa,ya,xs,ys;
        bool checkCoast;
        bool arrivalIsClosest;
        bool routeFromBoat;
        QList<float> calculateCaps(vlmPoint point,float workAngleStep, float workAngleRange);
        bool aborted;
        bool running;
        int debugCross0;
        int debugCross1;
        QDateTime whatIfDate;
        int       whatIfTime;
        bool      whatIfUsed;
        int       whatIfWind;
        time_t whatIfJour;
        bool tooFar;
        QList<vlmPointGraphic *> isoPointList;
        vlmPoint pivotPoint;
        bool isPivot;
        QMenu * popup;
        QAction * ac_pivot;
        QAction * ac_pivotM;
        void createPopupMenu();
        bool useMultiThreading;
        bool isNewPivot;
        ROUTAGE * fromRoutage;
        QString poiPrefix;
        bool autoZoom;
    };
#endif // ROUTAGE_H
