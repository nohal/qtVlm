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

#define NO_CROSS 1
#define BOUNDED_CROSS 2
#define L1_CROSS 3
#define L2_CROSS 4

//===================================================================
class ROUTAGE : public QGraphicsWidget
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

        void setStartTime(QDateTime date){this->startTime=date;}
        QDateTime getStartTime() {return this->startTime.toUTC();}

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
    public slots:
        void slot_edit();
        void slot_delete();
        void slot_shShow(){show();}
        void slot_shHidden(){hide();}
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

        /*popup menu*/
        QMenu *popup;
        QAction *r_edit;
        QAction *r_hide;
        QAction *r_delete;
        void createPopUpMenu(void);

        /*various*/
        QPointF start;
        QPointF arrival;
        float A360(float hdg);
        float myDiffAngle(float a1,float a2);
        void findPoint(float lon, float lat, double wind_angle, double wind_speed, float cap, vlmPoint *pt, bool estimateOnly);
        float findTime(const vlmPoint * pt, QPointF P, float * cap);
        float loxoCap;
        float initialDist;

        bool arrived;
        bool windIsForced;
        double wind_speed;
        double wind_angle;
        time_t eta;

        vlmLine * result;
        void drawResult(vlmPoint P);
        bool intersects(QList<vlmPoint> *iso,int nn,int mm,int * toBeKilled);
        bool done;
        bool converted;
        float findDistancePreviousIso(vlmPoint P,QPolygonF * isoShape);
        int superIntersects(QLineF L1,QLineF L2);
        bool tooFar(vlmPoint point);
        void pruneWake(QList<vlmPoint> * tempPoints,int wakeAngle,int mode);
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
    };
#endif // ROUTAGE_H
