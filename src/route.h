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
#ifndef ROUTE_H
#define ROUTE_H

#include <QPainter>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QMenu>

class ROUTE;

#include "Projection.h"
#include "boatAccount.h"
#include "POI.h"
#include "vlmLine.h"
#include "Grib.h"


//===================================================================
class ROUTE : public QGraphicsWidget
{ Q_OBJECT
    public:
        /* constructeurs, destructeurs */
        ROUTE(QString name, Projection *proj, Grib *grib, QGraphicsScene * myScene, myCentralWidget *parentWindow);

        ~ROUTE();

        void setName(QString name){this->name=name;}
        QString getName() {return(this->name);}

        void setBoat(boatAccount *boat);
        boatAccount * getBoat(){return this->boat;}

        void setColor(QColor color);
        QColor getColor(){return this->color;}

        void setWidth(float width);
        float getWidth() {return this->width;}

        void setStartTime(QDateTime date){this->startTime=date;}
        QDateTime getStartTime() {return this->startTime.toUTC();}

        void setFrozen(bool frozen) {this->frozen=frozen;slot_recalculate();}
        bool getFrozen(){return this->frozen;}
        void setLive(bool live) {this->live=live;}
        bool isLive(){return this->live;}
        bool isBusy(){return this->busy;}

        void setStartFromBoat(bool startFromBoat){this->startFromBoat=startFromBoat;}
        bool getStartFromBoat() {return this->startFromBoat;}

        void setStartTimeOption(int startTimeOption){this->startTimeOption=startTimeOption;}
        int getStartTimeOption() {return this->startTimeOption;}

        static bool myLessThan(ROUTE * ROUTE_1,ROUTE* ROUTE_2) {return ROUTE_1->name < ROUTE_2->name;}
        static bool myEqual(ROUTE * ROUTE_1,ROUTE* ROUTE_2) {return ROUTE_1->name == ROUTE_2->name;}

        void insertPoi(POI *poi);
        void removePoi(POI *poi);

//        void movePoi(POI *poi);
//        void updatePoi(POI *poi);
    public slots:
        void slot_recalculate();
        void slot_edit();
        void slot_delete();
        void slot_shShow(){show();if(line) line->show();}
        void slot_shHidden(){hide();if(line) line->hide();}
        void slot_shRou(){if(this->isVisible()) slot_shHidden();else slot_shShow();}
    signals:
        void editMe(ROUTE *);
        void deletePoi(POI *);
    private:
        /* parent, main */
        myCentralWidget *parent;
        QGraphicsScene *myscene;
        Projection *proj;

        /* widget component */
        QColor color;
        float width;
        vlmLine *line;
        QPen pen;

        /* data */
        QString name;
        QList<POI*> my_poiList;
        boatAccount *boat;
        Grib *grib;
        bool startFromBoat;
        QDateTime startTime;
        int startTimeOption;
        bool frozen;
        bool superFrozen;
        bool live;
        bool busy;

        /*popup menu*/
        QMenu *popup;
        QAction *r_edit;
        QAction *r_hide;
        QAction *r_delete;
        void createPopUpMenu(void);

        /*various*/
        float A360(float hdg);
        float myDiffAngle(float a1,float a2);
};
#endif // ROUTE_H
