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

#include "class_list.h"

#include "DialogUnits.h"
#include "DialogGraphicsParams.h"
#include "selectionWidget.h"
#include <qsound.h>
#include <qdatetime.h>
#include <MainWindow.h>

/* Z value according to type */
#define Z_VALUE_TERRE      0
#define Z_VALUE_FAXMETEO   0.1
#define Z_VALUE_ROUTAGE    1
#define Z_VALUE_ISOPOINT   2
#define Z_VALUE_OPP        3
#define Z_VALUE_ESTIME     4
#define Z_VALUE_ROUTE      5
#define Z_VALUE_POI        7
#define Z_VALUE_GATE       8
#define Z_VALUE_NEXT_GATE  9
#define Z_VALUE_BOAT       10
#define Z_VALUE_COMPASS    11

#define Z_VALUE_SELECTION  15

/* graphicsWidget type */
#define TERRE_WTYPE       1
#define POI_WTYPE         2
#define COMPASS_WTYPE     3
#define BOAT_WTYPE        4
#define SELECTION_WTYPE   5
#define OPP_WTYPE         6
#define ISOPOINT          7
#define BOATREAL_WTYPE    8
#define FAXMETEO_WTYPE    9

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
        void wheelEvent(QGraphicsSceneWheelEvent* e);
        bool event(QEvent * event);
    signals:
        void paramVLMChanged();
        void eraseWay();
    private slots:
        void wheelTimerElapsed();
    private:
        myCentralWidget * parent;
        bool hasWay;
        int  wheelStrokes;
        QTimer *wheelTimer;
        int wheelPosX;
        int wheelPosY;
        bool wheelCenter;
};

class myCentralWidget : public QWidget
{ Q_OBJECT
    public:
        myCentralWidget(Projection * proj,MainWindow * parent,MenuBar *menuBar);
        void loadBoat(void);
        void loadPOI(void);
        ~myCentralWidget();

        /* access to pointer & data */
        Grib * getGrib(void);
        QGraphicsScene * getScene(void) { return scene; }
        bool compassHasLine(void);
        int getCompassMode(int m_x,int m_y);
        bool isSelecting(void);
        QList<boatVLM*> * getBoats() { return this->boat_list; }
        QList<Player*> &  getPlayers() { return this->player_list; }
        QList<raceData*> & getRaces() { return this->race_list; }
        QList<POI*> & getPois() { return this->poi_list; }
        QList<POI*> * getPoisList() { return & this->poi_list; }
        GshhsReader * get_gshhsReader(void) { return gshhsReader; }
        opponentList * getOppList() { return opponents; }
        inetConnexion * getInet(void) { return inetManager; }
        boat * getSelectedBoat(void);
        bool hornIsActivated(void){return hornActivated;}
        void setHornIsActivated(bool b){this->hornActivated=b;}
        QDateTime getHornDate(void){return this->hornDate;}
        void setHornDate(QDateTime t){this->hornDate=t;}
        void setHorn();
        void twaDraw(double lon, double lat);
        Player * getPlayer(void) { return currentPlayer; }
        boatReal * getRealBoat(void) {return realBoat; }
        bool getIsStartingUp(void){return mainW->isStartingUp;}

        void manageAccount(bool * res=NULL);
        void updatePlayer(Player * player);
        bool getIsSelecting(){return this->selection->isSelecting();}

        /* route */
        QList<ROUTE*> & getRouteList(){ return this->route_list;}
        bool freeRouteName(QString name, ROUTE * route);
        void assignPois();
        void emitUpdateRoute(boat * boat){emit updateRoute(boat);}
        ROUTE * addRoute();
        void setCompassFollow(ROUTE * route);
        ROUTE * getCompassFollow(){return this->compassRoute;}
        void centerCompass(double lon,double lat);
        void update_menuRoute();
        void simpAllPOIs(bool b);
/* routage */
        QList<ROUTAGE*> & getRoutageList(){ return this->routage_list;}
        bool freeRoutageName(QString name, ROUTAGE * routage);
        ROUTAGE * addRoutage();
        int getNbRoutage(){return nbRoutage;}
        void addPivot(ROUTAGE * fromRoutage,bool editOptions=false);
        void deleteRoutage(ROUTAGE * routage);
/*Other*/
        Projection * getProj(void){return proj;}
        void update_menuRoutage();

        void send_redrawAll() { emit redrawAll(); }

        /* grib */
        void   setCurrentDate(time_t t, bool uRoute=true);
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

        /* item state */
        bool get_shLab_st(void) { return shLab_st; }
        bool get_shPoi_st(void) { return shPoi_st; }
        bool get_shRoute_st(void) { return shRoute_st; }
        bool get_shOpp_st(void) { return shOpp_st; }
        bool get_shPor_st(void) { return shPor_st; }
        void exportRouteFromMenu(ROUTE * route);
        void exportRouteFromMenuGPX(ROUTE * route,QString fileName,bool POIonly);

        /*races*/
        void drawNSZ(int i);
        void removeOpponent(QString oppId, QString raceId);
        Terrain * getTerre(){return terre;}
        time_t getNextVac();
        void setPilototo(QList<POI*> poiList);
        void treatRoute(ROUTE* route);

    public slots :
        /* Zoom & position */
        void slot_Zoom_All();
        void slot_Zoom_In(double quantity=1.3);
        void slot_Zoom_Out(double quantity=1.3);
        void slot_Zoom_Wheel(double quantity, int XX, int YY, bool centerOnWheel);
        void slot_Go_Left();
        void slot_Go_Right();
        void slot_Go_Up();
        void slot_Go_Down();
        void slot_Zoom_Sel();
        void slot_keepPos(bool b){this->keepPos=b;}
        void slot_abortRequest();

        /* POI */
        POI * slot_addPOI(QString name,int type,double lat,double lon, double wph,int timestamp,bool useTimeStamp, boat *boat);
        void slot_addPOI_list(POI * poi);
        void slot_delPOI_list(POI * poi);
        void slot_POISave(void);
        void slot_POIRestore(void);
        void slot_POIimport(void); // import data from zyGrib
        void slot_delAllPOIs(void);
        void slot_delSelPOIs(void);
        void slot_notSimpAllPOIs(void);
        void slot_simpAllPOIs(void);

        /* item state */
        void slot_showALL(bool);
        void slot_hideALL(bool);

        void slot_shLab(bool);
        void slot_shPoi(bool);
        void slot_shRoute(bool);
        void slot_shOpp(bool);
        void slot_shPor(bool);
        void slot_shFla(bool);


        /*Routes */
        void slot_addRouteFromMenu();
        void slot_importRouteFromMenu(bool ortho=false);
        void slot_importRouteFromMenu2();
        void slot_editRoute(ROUTE * route,bool createMode=false);
        void slot_twaLine();
        void slot_releaseCompassFollow(){this->compassRoute=NULL;}
        void slot_deleteRoute();
        void withdrawRouteFromBank(QString routeName,QList<QVariant> details);

        /*Routages */
        void slot_addRoutageFromMenu();
        void slot_editRoutage(ROUTAGE * routage,bool createMode=false);
        void slot_deleteRoutage();

        /* Players */
        void slot_addPlayer_list(Player* player);
        void slot_delPlayer_list(Player* player);
        void slot_playerSelected(Player * player);

        /* Boats */
        void slot_addBoat(boat* boat);
        void slot_delBoat(boat* boat);
        void slot_writeBoatData(void);
        void slot_readBoatData(void);
        void slot_moveBoat(double lat, double lon);

        /* Races */
        void slot_addRace_list(raceData* race);
        void slot_delRace_list(raceData* race);
        void slot_readRaceData(void);

        /* Grib */
        void slot_fileLoad_GRIB(void);
        void slot_fileInfo_GRIB(void);
        void slotLoadSailsDocGrib(void);
        void slotFax_open();
        void slotFax_close();

        /* Dialogs */
        void slot_boatDialog(void);
        void slot_manageAccount();
        void slot_raceDialog(void);

        /* Events */
        void slot_mousePress(QGraphicsSceneMouseEvent*);
        void slot_mouseRelease(QGraphicsSceneMouseEvent* e);

        /* Menu */
        void slot_map_CitiesNames();
        void slot_clearSelection(void);
        void slotIsobarsStep();
        void slotIsotherms0Step();
        void slot_setColorMapMode(QAction*);
        void slot_editHorn();
        void slot_playHorn();
        void slot_startReplay();
        void slot_replay();
        void slot_takeScreenshot();
        void slot_showVlmLog();
        void slot_fetchVLMTrack();

    signals:
        /* drawing */
        void redrawAll(void);
        void redrawGrib(void);
        void startReplay(bool);
        void replay(int);

        /* POI */
        void readPOIData(QString);
        void writePOIData(QList<ROUTE*> &,QList<POI*> &,QString);
        void importZyGrib(void);
        void POI_selectAborted(POI*);
        void updateRoute(boat *);
        void updateRoutage();
        void twaDelPoi(POI*);

        /* Boats */
        void writeBoatData(QList<Player*> & player_list,QList<raceData*> & race_list,QString fname);
        void readBoatData(QString fname, bool readAll);
        void boatPointerHasChanged(boat *);
        void accountListUpdated(void);

        /* compass */
        void stopCompassLine(void);

        /*show-hide*/
        void hideALL(bool);
        void showALL(bool);
        void shOpp(bool);
        void shPoi(bool);
        void shCom(bool);
        void shRou(bool);
        void shRouBis();
        void shPor(bool);
        void shPol(bool);
        void shLab(bool);
        void shFla();

    protected:
        void resizeEvent (QResizeEvent * e);

    private:        

        //QCursor cur_cursor;

        bool resizing;

        Projection * proj;
        MainWindow * mainW;
        MenuBar    *menuBar;

        /* item child */
        Terrain * terre;        
        mapCompass * compass;
        selectionWidget * selection;
        opponentList * opponents;
        vlmLine * NSZ;

        /* Grib */
        Grib *grib;
        void zoomOnGrib(void);
        QString  dataPresentInGrib(Grib* grib,
                                   int dataType,int levelType,int levelValue,
                                   bool *ok=NULL);
        /* other child */
        GshhsReader *gshhsReader;
        inetConnexion * inetManager;

        /* Scene & view */
        QGraphicsScene *scene;
        QGraphicsView * view;

        /* Dialogs */
        DialogGribDate * gribDateDialog;
        DialogPoi * poi_editor;
        DialogBoatAccount * boatAcc;
        DialogPlayerAccount * playerAcc;
        DialogRace * raceDialog;
        DialogLoadGrib  * dialogLoadGrib;
        DialogUnits     dialogUnits;
        DialogGraphicsParams  dialogGraphicsParams;
        DialogRealBoatConfig * realBoatConfig;
        DialogVlmLog * vlmLogViewer;
        DialogDownloadTracks * vlmTrackRetriever;

        /* Lists, POI*/
        QList<POI*> poi_list;
        QList<ROUTE*> route_list;
        QList<ROUTAGE*> routage_list;
        QList<boatVLM*> * boat_list;
        QList<Player*> player_list;
        QList<raceData*> race_list;
        Player * currentPlayer;
        boatReal * realBoat;

        /* Data file */
        xml_POIData * xmlPOI;
        xml_boatData * xmlData;
        bool aboutToQuit;

        /* items state */
        bool shLab_st;
        bool shPoi_st;
        bool shRoute_st;
        bool shOpp_st;
        bool shPor_st;

        QSound  *horn;
        bool    hornActivated;
        QDateTime  hornDate;
        QTimer *hornTimer;
        DialogTwaLine *twaTrace;
        ROUTE * compassRoute;
        int nbRoutage;
        bool keepPos;
        void deleteRoute(ROUTE * route);
        void myDeleteRoute(ROUTE * route);
        int replayStep;
        QTimer *replayTimer;
        void doSimplifyRoute(ROUTE * route, bool fast=false);
        bool abortRequest;
        faxMeteo * fax;
};

#endif // MYCENTRALWIDGET_H
