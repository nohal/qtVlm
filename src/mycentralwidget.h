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

***********************************************************************/

#ifndef MYCENTRALWIDGET_H
#define MYCENTRALWIDGET_H

#include <QGraphicsScene>
#include <QGraphicsView>

class myCentralWidget;

#include "MainWindow.h"
#include "Projection.h"
#include "MenuBar.h"

/* item child */
#include "Terrain.h"
#include "mapcompass.h"
#include "selectionWidget.h"
#include "POI.h"
#include "vlmLine.h"
#include "opponentBoat.h"

/* other child */
#include "GshhsReader.h"
#include "Grib.h"
#include "inetConnexion.h"

/* dialog */
#include "dialog_gribDate.h"
#include "DialogLoadGrib.h"
#include "DialogUnits.h"
#include "POI_editor.h"
#include "Route_Editor.h"
#include "boatAccount_dialog.h"
#include "race_dialog.h"
#include "DialogGraphicsParams.h"
#include "route.h"

/* Data file */
#include "xmlPOIData.h"
#include "xmlBoatData.h"

/* Z value according to type */
#define Z_VALUE_TERRE      0
#define Z_VALUE_OPP        2
#define Z_VALUE_ESTIME     3
#define Z_VALUE_ROUTE      4
#define Z_VALUE_POI        5
#define Z_VALUE_GATE       6
#define Z_VALUE_BOAT       7
#define Z_VALUE_COMPASS    10

#define Z_VALUE_SELECTION  15

/* graphicsWidget type */
#define TERRE_WTYPE       1
#define POI_WTYPE         2
#define COMPASS_WTYPE     3
#define BOAT_WTYPE        4
#define SELECTION_WTYPE   5
#define OPP_WTYPE         6

/* compass mode */
#define COMPASS_NOTHING  0
#define COMPASS_LINEON 1
#define COMPASS_UNDER  2

class myScene : public QGraphicsScene
{ Q_OBJECT
    public:
        myScene(myCentralWidget * parent = 0);

    protected:
        void keyPressEvent (QKeyEvent *e);
        void keyReleaseEvent (QKeyEvent *e);
        void mouseMoveEvent (QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e);
    signals:
        void paramVLMChanged();
    private:
        myCentralWidget * parent;
};

class myCentralWidget : public QWidget
{ Q_OBJECT
    public:
        myCentralWidget(Projection * proj,MainWindow * parent,MenuBar *menuBar);
        void loadData(void);
        ~myCentralWidget();

        /* access to pointer & data */
        Grib * getGrib(void) { if(grib && grib->isOk()) return grib; else return NULL; }
        QGraphicsScene * getScene(void) { return scene; }
        bool compassHasLine(void);
        int getCompassMode(int m_x,int m_y);
        bool isSelecting(void);
        QList<boatAccount*> &  getBoats() { return this->acc_list; }
        QList<raceData*> & getRaces() { return this->race_list; }
        GshhsReader * get_gshhsReader(void) { return gshhsReader; }
        opponentList * getOppList() { return opponents; }
        inetConnexion * getInet(void) { return inetManager; }

        /* route */
        QList<ROUTE*> & getRouteList(){ return this->route_list;}
        bool freeRouteName(QString name, ROUTE * route);
        void update_menuRoute();
        void deleteRoute(ROUTE * route);
        void freezeRoutes(bool freeze);
        void assignPois();
        ROUTE * addRoute();
        Projection * getProj(void){return proj;}


        /* grib */
        void   setCurrentDate(time_t t);
        time_t getCurrentDate(void);
        void showGribDate_dialog(void);
        void loadGribFile(QString fileName, bool zoom);

        /* events */
        void mouseMove(int x, int y, QGraphicsItem * item);
        void mouseDoubleClick(int x, int y, QGraphicsItem * item);
        void keyModif(QKeyEvent *e);
        void escapeKeyPressed(void);
        bool getAboutToQuit(void){return aboutToQuit;}
        void setAboutToQuit(void){this->aboutToQuit=true;}

    public slots :
        /* Zoom & position */
        void slot_Zoom_All();
        void slot_Zoom_In();
        void slot_Zoom_Out();
        void slot_Go_Left();
        void slot_Go_Right();
        void slot_Go_Up();
        void slot_Go_Down();
        void slot_Zoom_Sel();

        /* POI */
        void slot_addPOI(QString name,int type,float lat,float lon, float wph,int timestamp,bool useTimeStamp, boatAccount *boat);
        void slot_addPOI_list(POI * poi);
        void slot_delPOI_list(POI * poi);
        void slot_POISave(void);
        void slot_POIimport(void); // import data from zyGrib
        void slot_delAllPOIs(void);
        void slot_delSelPOIs(void);

        /*Routes */
        void slot_addRouteFromMenu();
        void slot_editRoute(ROUTE * route,bool createMode=false);

        /* Boats */
        void slot_addBoat_list(boatAccount* boat);
        void slot_delBoat_list(boatAccount* boat);
        void slot_writeBoatData(void);
        void slot_readBoatData(void);

        /* Races */
        void slot_addRace_list(raceData* race);
        void slot_delRace_list(raceData* race);
        void slot_readRaceData(void);

        /* Grib */
        void slot_fileLoad_GRIB(void);
        void slot_fileInfo_GRIB(void);

        /* Dialogs */
        void slot_boatDialog(void);
        void slot_raceDialog(void);

        /* Events */
        void slot_mousePress(QGraphicsSceneMouseEvent*);
        void slot_mouseRelease(QGraphicsSceneMouseEvent* e);

        /* Menu */
        void slot_map_CitiesNames();
        void slot_clearSelection(void);

    signals:
        /* drawing */
        void redrawAll(void);
        void redrawGrib(void);

        /* POI */
        void readPOIData(QString);
        void writePOIData(QList<ROUTE*> &,QList<POI*> &,QString);
        void importZyGrib(void);
        void POI_selectAborted(POI*);
        void updateRoute();

        /* Boats */
        void writeBoatData(QList<boatAccount*> & boat_list,QList<raceData*> & race_list,QString fname);
        void readBoatData(QString fname, bool readAll);

        /* compass */
        void stopCompassLine(void);

        /*show-hide*/
        void hideALL(bool);
        void showALL(bool);
        void shOpp(bool);
        void shPoi(bool);
        void shCom(bool);
        void shRou(bool);
        void shPor(bool);
        void shPol(bool);
        void shLab(bool);

    protected:
        void resizeEvent (QResizeEvent * e);

    private:        

        //QCursor cur_cursor;

        Projection * proj;
        MainWindow * main;
        MenuBar    *menuBar;

        /* item child */
        Terrain * terre;        
        mapCompass * compass;
        selectionWidget * selection;
        opponentList * opponents;

        /* Grib */
        Grib *grib;
        void zoomOnGrib(void);

        /* other child */
        GshhsReader *gshhsReader;
        inetConnexion * inetManager;

        /* Scene & view */
        QGraphicsScene *scene;
        QGraphicsView * view;

        /* Dialogs */
        dialog_gribDate * gribDateDialog;
        POI_Editor * poi_editor;
        boatAccount_dialog * boatAcc;
        race_dialog * raceParam;
        DialogLoadGrib  * dialogLoadGrib;
        DialogUnits     dialogUnits;
        DialogGraphicsParams dialogGraphicsParams;

        /* Lists, POI*/
        QList<POI*> poi_list;
        QList<ROUTE*> route_list;
        QList<boatAccount*> acc_list;
        QList<raceData*> race_list;

        /* Data file */
        xml_POIData * xmlPOI;
        xml_boatData * xmlData;
        float A360(float hdg);
        bool aboutToQuit;

};

#endif // MYCENTRALWIDGET_H
