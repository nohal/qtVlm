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

#include "vlmPoint.h"
#include "DataManager.h"
#include "vlmLine.h"

#define NO_CROSS 1
#define BOUNDED_CROSS 2
#define L1_CROSS 3
#define L2_CROSS 4

//#define OLD_BARRIER

struct datathread
{
    time_t Eta;
    DataManager *dataManager;
    bool whatIfUsed;
    time_t whatIfJour;
    int whatIfTime;
    double whatIfWind;
    boat *Boat;
    int timeStep;
    double speedLossOnTack;
    bool i_iso;
};
Q_DECLARE_TYPEINFO(datathread,Q_PRIMITIVE_TYPE);

//===================================================================
class ROUTAGE : public QObject
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        ROUTAGE(QString name, Projection *proj, DataManager * dataManager, QGraphicsScene * myScene, myCentralWidget *parentWindow);

        ~ROUTAGE();
        void setName(const QString &name){this->name=name;}
        const QString getName() const {return(this->name);}

        void setBoat(boat *myBoat);
        boat * getBoat(){return this->myBoat;}

        void setColor(const QColor &color);
        const QColor getColor() const {return this->color;}

        void setWidth(const double &width);
        double getWidth() const {return this->width;}

        void setAngleRange(const double &a) {this->angleRange=a;}
        double getAngleRange(void) const {return this->angleRange;}
        void setAngleStep(const double &a) {this->angleStep=a;}
        double getAngleStep(void) const {return this->angleStep;}
        double getTimeStep(void) const;
        void setTimeStepMore24(const double &t) {this->timeStepMore24=t;}
        double getTimeStepMore24(void) const {return this->timeStepMore24;}
        void setTimeStepLess24(const double &t) {this->timeStepLess24=t;}
        double getTimeStepLess24(void) const {return this->timeStepLess24;}
        bool getShowIso(void) const {return this->showIso;}
        void setShowIso(const bool &b);
        void setExplo(const double &e){this->explo=e;}
        double getExplo(void) const {return explo;}
        bool getUseRouteModule(void) const {return useRouteModule;}
        void setUseRouteModule(const bool &b){this->useRouteModule=b;}
        bool getRouteFromBoat() const {return this->routeFromBoat;}
        void setRouteFromBoat(const bool &b){this->routeFromBoat=b;}

        void setStartTime(const QDateTime &date){this->startTime=date;this->etaStart=date.toTime_t();}
        QDateTime getStartTime() const {return this->startTime.toUTC();}
        void setWhatIfDate(const QDateTime &date){this->whatIfDate=date;}
        const QDateTime getWhatIfDate() const {return this->whatIfDate.toUTC();}
        bool getWhatIfUsed(void) const {return this->whatIfUsed;}
        void setWhatIfUsed(const bool &b){this->whatIfUsed=b;}
        int getWhatIfTime(void) const {return this->whatIfTime;}
        void setWhatIfTime(const int &t){this->whatIfTime=t;}
        int getWhatIfWind(void) const {return this->whatIfWind;}
        void setWhatIfWind(const int &p){this->whatIfWind=p;}

        static bool myLessThan(const ROUTAGE * ROUTAGE_1,const ROUTAGE* ROUTAGE_2) {return ROUTAGE_1->name < ROUTAGE_2->name;}
        static bool myEqual(const ROUTAGE * ROUTAGE_1,const ROUTAGE* ROUTAGE_2) {return ROUTAGE_1->name == ROUTAGE_2->name;}

        void setFromPOI(POI *poi){this->fromPOI=poi;}
        POI * getFromPOI() const {return this->fromPOI;}
        void setToPOI(POI *poi){this->toPOI=poi;}
        POI * getToPOI() const {return this->toPOI;}
        bool isDone(void) const {return this->done;}
        bool isConverted(void) const {return this->converted;}
        void setConverted(void) {this->converted=true;}
        void convertToRoute(void);
        const QDateTime getFinalEta(void) const {return finalEta;}
        bool getCheckCoast() const {return this->checkCoast;}
        void setCheckCoast(const bool &b){this->checkCoast=b;}
        bool getCheckLine() const {return this->checkLine;}
        void setCheckLine(const bool &b){this->checkLine=b;}
        bool isRunning() const {return this->running;}
        void setFromRoutage(ROUTAGE * fromRoutage,bool editOptions);
        const vlmPoint getPivotPoint() const {return pivotPoint;}
        vlmLine * getWay(){return way;}
        void setPivotPoint(const int &isoNb,const int &pointNb);
        void showContextMenu(const int &isoNb,const int &pointNb);
        bool getIsNewPivot() const {return isNewPivot;}
        void setIsNewPivot(const bool &b){isNewPivot=b;}
        bool getIsPivot() const {return this->isPivot;}
        void setPoiPrefix(const QString &s){this->poiPrefix=s;}
        const QString getPoiPrefix() const {return this->poiPrefix;}
        int  pruneWakeAngle;
        bool useConverge;
        time_t getEta() const {return eta;}
        FCT_GET(DataManager*,dataManager)
        time_t getWhatIfJour() const {return whatIfJour;}
        static int calculateTimeRoute(const vlmPoint &RouteFrom,const vlmPoint &routeTo, const datathread *dataThread,double * lastLonFound=NULL, double * lastLatFound=NULL, const int &limit=-1);
        static int routeFunction(const double &x,const vlmPoint &from, double * lastLonFound, double * lastLatFound, const datathread *dataThread);
        static int routeFunctionDeriv(const double &x,const vlmPoint &from, double * lastLonFound, double * lastLatFound, const datathread *dataThread);
        bool getUseMultiThreading(){return this->useMultiThreading;}
        void setUseMultiThreading(bool b){this->useMultiThreading=b;}
        vlmLine * getResult() const {return result;}
        ROUTAGE * getFromRoutage() const {return fromRoutage;}
        bool getAutoZoom() const {return autoZoom;}
        void setAutoZoom(const bool &b){this->autoZoom=b;}
        double getSpeedLossOnTack() const {return speedLossOnTack;}
        void setSpeedLossOnTack(const double &d){this->speedLossOnTack=d;}
        GshhsReader * getMap() const {return this->map;}
        double getMaxPres() const {return maxPres;}
        double getMaxPortant() const {return maxPortant;}
        double getMinPres() const {return minPres;}
        double getMinPortant() const {return minPortant;}
        void setMaxPres(const double &d){this->maxPres=d;}
        void setMaxPortant(const double &d){this->maxPortant=d;}
        void setMinPres(const double &d){this->minPres=d;}
        void setMinPortant(const double &d){this->minPortant=d;}
        Projection * getProj() const {return proj;}
        static double findDistancePreviousIso(const vlmPoint &P, const QPolygonF * poly);
        QPolygonF * getPreviousIso() {return &previousIso;}
        QList<QLineF> * getPreviousSegments() {return &previousSegments;}
        bool getVisibleOnly() const {return visibleOnly;}
        void setVisibleOnly(const bool &b){this->visibleOnly=b;}
        int getZoomLevel() const {return this->zoomLevel;}
        void setZoomLevel(const int &i){this->zoomLevel=i;}
        void calculateInverse();
        bool getI_iso() const {return i_iso;}
        void setI_iso(const bool &b){this->i_iso=b;}
        bool getI_done() const {return i_done;}
        time_t getI_eta() const {return i_eta;}
        void showIsoRoute();
        int getIsoRouteValue() const {return isoRouteValue;}
        void setIsoRouteValue(const int &i){this->isoRouteValue=i;}
        bool crossBarriere(const QLineF &line) const;
        void setThresholdAlternative(const int &i){this->thresholdAlternative=i;}
        int getThresholdAlternative() const {return this->thresholdAlternative;}
        void setNbAlternative(const int &i){this->nbAlternative=i;}
        int getNbAlternative() const {return this->nbAlternative;}
        void calculateAlternative();
        void deleteAlternative(){while(!alternateRoutes.isEmpty())
                delete alternateRoutes.takeFirst();}
        QList<vlmLine*> getIsochrones(){return isochrones;}
        void setColorGrib(const bool &b){this->colorGrib=b;}
        bool getColorGrib(){return this->colorGrib;}
        bool getArrived() const {return this->arrived;}
        bool checkIceGate(const vlmPoint &p) const;
        QPointF getStart() const {return this->start;}
        QPointF getArrival() const {return this->arrival;}
        double getXa() const {return xa;}
        double getYa() const {return ya;}
        double getXs() const {return xs;}
        double getYs() const {return ys;}
        bool getRoutageOrtho() const {return routageOrtho;}
        void setRoutageOrtho(const bool &b){routageOrtho=b;}
        bool getShowBestLive() const {return showBestLive;}
        void setShowBestLive(const bool &b){showBestLive=b;}
        QList<bool> * getPreviousIsoLand(){return &previousIsoLand;}
        QList<QLineF> * getForbidZone(){return &forbidZone;}
        QPolygonF * getShapeIso(){return &shapeIso;}
        FCT_SETGET(bool,multiRoutage)
        FCT_SETGET(int,multiDays)
        FCT_SETGET(int,multiHours)
        FCT_SETGET(int,multiMin)
        FCT_SETGET(int,multiNb)
        FCT_GET_CST(double,maxDist)
        FCT_SETGET_CST(double,maxWaveHeight)
public slots:
        void calculate();
        void slot_edit();
        void slot_abort(){this->aborted=true;}
        void slot_createPivot();
        void slot_createPivotM();
        void slot_drawWay();
        void slot_calculate();
        void slot_calculate_with_tempo();
        void eraseWay();
        void slot_gribDateChanged();
        void slot_deleteRoutage(void);
    signals:
        void editMe(ROUTAGE *);
        void updateVgTip(int,int,QString);
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
        DataManager * dataManager;
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
        double mySignedDiffAngle(const double a1,const double a2);
        double findTime(const vlmPoint * pt, QPointF P, double * cap);
        double loxoCap;
        double initialDist;
        GshhsReader *map;





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
        QList<bool> previousIsoLand;
        QList<QLineF> previousSegments;
        QList<QLineF> forbidZone;
        bool somethingHasChanged;
        void checkIsoCrossingPreviousSegments();
        void epuration(int toBeRemoved);
        void removeCrossedSegments();
        double xa,ya,xs,ys;
        bool checkCoast,checkLine;
        int  nbAlternative;
        int  thresholdAlternative;
        bool arrivalIsClosest;
        bool routeFromBoat;
        void calculateCaps(QList<double> *caps, const vlmPoint &point, const double &workAngleStep, const double &workAngleRange);
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

        QAction * ac_edit;
        QAction * ac_remove;
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
        double maxWaveHeight;
        bool visibleOnly;
        QTimer * timerTempo;
        bool approaching;
        int zoomLevel;
        QList<vlmPointGraphic *> isoPointList;
        bool arrived;
        time_t eta, etaStart;
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
        QList<vlmLine*> alternateRoutes;
#ifdef OLD_BARRIER
        QList<QLineF> barrieres;
#endif
        QList<QLineF> iceGates;
        void countDebug(int nbIso, QString s);
        bool colorGrib;
        bool routageOrtho;
        bool showBestLive;
        QPolygonF shapeIso;
        void calculateShapeIso();
        bool multiRoutage;
        int multiNb;
        int multiDays;
        int multiHours;
        int multiMin;
        double maxDist;
        void calculateMaxDist();
};
Q_DECLARE_TYPEINFO(ROUTAGE,Q_MOVABLE_TYPE);
#endif // ROUTAGE_H
