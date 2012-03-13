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
#include "GshhsReader.h"
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
    double whatIfWind;
    double windSpeed;
    double windAngle;
    boat *Boat;
    int timeStep;
    double speedLossOnTack;
    bool i_iso;
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

        void setWidth(double width);
        double getWidth() {return this->width;}

        void setWindIsForced(bool b){this->windIsForced=b;}
        void setWind(double twd, double tws){this->wind_angle=twd;this->wind_speed=tws;}
        double getWindAngle(void){return this->wind_angle;}
        double getWindSpeed(void){return this->wind_speed;}
        void setAngleRange(double a) {this->angleRange=a;}
        double getAngleRange(void){return this->angleRange;}
        void setAngleStep(double a) {this->angleStep=a;}
        double getAngleStep(void){return this->angleStep;}
        double getTimeStep(void);
        void setTimeStepMore24(double t) {this->timeStepMore24=t;}
        double getTimeStepMore24(void){return this->timeStepMore24;}
        void setTimeStepLess24(double t) {this->timeStepLess24=t;}
        double getTimeStepLess24(void){return this->timeStepLess24;}
        bool getWindIsForced(void){return this->windIsForced;}
        bool getShowIso(void){return this->showIso;}
        void setShowIso(bool b);
        void setExplo(double e){this->explo=e;}
        double getExplo(void){return explo;}
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
        bool getCheckLine(){return this->checkLine;}
        void setCheckLine(bool b){this->checkLine=b;}
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
        int  pruneWakeAngle;
        bool useConverge;
        time_t getEta(){return eta;}
        Grib * getGrib(){return grib;}
        time_t getWhatIfJour(){return whatIfJour;}
        static int calculateTimeRoute(vlmPoint RouteFrom,vlmPoint routeTo, datathread *dataThread,double * lastLonFound=NULL, double * lastLatFound=NULL, int limit=-1);
        static int routeFunction(double x,vlmPoint from, double * lastLonFound, double * lastLatFound, datathread *dataThread);
        static int routeFunctionDeriv(double x,vlmPoint from, double * lastLonFound, double * lastLatFound, datathread *dataThread);
        bool getUseMultiThreading(){return this->useMultiThreading;}
        void setUseMultiThreading(bool b){this->useMultiThreading=b;}
        vlmLine * getResult(){return result;}
        ROUTAGE * getFromRoutage(){return fromRoutage;}
        bool getAutoZoom(){return autoZoom;}
        void setAutoZoom(bool b){this->autoZoom=b;}
        double getSpeedLossOnTack(){return speedLossOnTack;}
        void setSpeedLossOnTack(double d){this->speedLossOnTack=d;}
        GshhsReader * getMap(){return this->map;}
        double getMaxPres(){return maxPres;}
        double getMaxPortant(){return maxPortant;}
        double getMinPres(){return minPres;}
        double getMinPortant(){return minPortant;}
        void setMaxPres(double d){this->maxPres=d;}
        void setMaxPortant(double d){this->maxPortant=d;}
        void setMinPres(double d){this->minPres=d;}
        void setMinPortant(double d){this->minPortant=d;}
        Projection * getProj(){return proj;}
        static double findDistancePreviousIso(const vlmPoint P, const QPolygonF * poly);
        QPolygonF * getPreviousIso(){return &previousIso;}
        QList<QLineF> * getPreviousSegments(){return &previousSegments;}
        bool getVisibleOnly(){return visibleOnly;}
        void setVisibleOnly(bool b){this->visibleOnly=b;}
        int getZoomLevel(){return this->zoomLevel;}
        void setZoomLevel(int i){this->zoomLevel=i;}
        void calculateInverse();
        bool getI_iso(){return i_iso;}
        void setI_iso(bool b){this->i_iso=b;}
        bool getI_done(){return i_done;}
        time_t getI_eta(){return i_eta;}
        void showIsoRoute();
        int getIsoRouteValue(){return isoRouteValue;}
        void setIsoRouteValue(int i){this->isoRouteValue=i;}
        const bool crossBarriere(const QLineF line);
    public slots:
        void slot_edit();
        void slot_abort(){this->aborted=true;}
        void slot_createPivot();
        void slot_createPivotM();
        void slot_drawWay();
        void slot_calculate();
        void slot_calculate_with_tempo();
        void eraseWay();
        void slot_gribDateChanged();
    signals:
        void editMe(ROUTAGE *);
    private:
        /* parent, main */
        myCentralWidget *parent;
        QGraphicsScene *myscene;
        Projection *proj;

        /* widget component */
        QColor color;
        double width;
        vlmLine *iso;
        QPen pen;

        /* data */
        QString name;
        POI * fromPOI;
        POI * toPOI;
        boat *myBoat;
        Grib *grib;
        double angleRange;
        double angleStep;
        double timeStepMore24;
        double timeStepLess24;
        int explo;
        bool showIso;
        QDateTime startTime;
        bool useRouteModule;


        /*various*/
        QPointF start;
        QPointF arrival;
        double mySignedDiffAngle(double a1,double a2);
        bool findPoint(double lon, double lat, double wind_angle, double wind_speed, double cap, vlmPoint *pt);
        double findTime(const vlmPoint * pt, QPointF P, double * cap);
        double loxoCap;
        double initialDist;
        GshhsReader *map;




        bool windIsForced;
        double wind_speed;
        double wind_angle;

        vlmLine * result;
        vlmLine * way;
        void drawResult(vlmPoint P);
        bool intersects(QList<vlmPoint> *iso,int nn,int mm,int * toBeKilled);
        bool converted;
        void pruneWake(int wakeAngle);
        //int calculateTimeRoute(vlmPoint RouteFrom,vlmPoint routeTo,int limit=-1);
        //int routeFunction(double x,vlmPoint from);
        //int routeFunctionDeriv(double x,vlmPoint from);
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
        void checkIsoCrossingPreviousSegments();
        void epuration(int toBeRemoved);
        void removeCrossedSegments();
        double xa,ya,xs,ys;
        bool checkCoast,checkLine;
        bool arrivalIsClosest;
        bool routeFromBoat;
        QList<double> calculateCaps(vlmPoint point,double workAngleStep, double workAngleRange);
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
        double speedLossOnTack;
        int highlightedIso;
        double maxPres,maxPortant;
        double minPres,minPortant;
        bool visibleOnly;
        QTimer * timerTempo;
        bool approaching;
        int zoomLevel;
        QList<vlmPointGraphic *> isoPointList;
        bool arrived;
        time_t eta;
        QList<vlmLine *> isochrones;
        QList<vlmLine *> segments;
        bool done;
        /*iso inversed*/
        bool i_iso;
        bool i_arrived;
        time_t i_eta;
        QList<vlmLine *> i_isochrones;
        QList<vlmLine *> i_segments;
        bool i_done;
        static QPointF pointAt(const QPolygonF * poly, const double ratio);
        double findDistancePoly(const QPointF P, const QPolygonF * poly, QPointF * closest);
        double pointDistanceRatio(double x, double goal, QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly);
        double pointDistanceRatioDeriv(double x, double xStep, double goal, bool * status, QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly);
        bool newtownRaphson(double * root, double goal,double precision,QPolygonF *poly, QPolygonF *prev_poly, QPolygonF *i_poly);
        int isoRouteValue;
        QList<vlmLine*> isoRoutes;
        QList<QLineF> barrieres;
    };
#endif // ROUTAGE_H
