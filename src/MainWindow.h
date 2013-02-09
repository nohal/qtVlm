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

Original code zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr
***********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifdef QT_V5
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QGraphicsSceneContextMenuEvent>
#include <QtWidgets/QCheckBox>
#else
#include <QApplication>
#include <QMainWindow>
#include <QProgressDialog>
#include <QGraphicsSceneContextMenuEvent>
#include <QCheckBox>
#endif
#include <QMouseEvent>
#include <QTimer>
#include <QLibrary>

#include "class_list.h"
#include "dataDef.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(int w, int h, QWidget *parent = 0);
        ~MainWindow();

        void openGribFile(QString fileName, bool zoom=true, bool current=false);
        bool getBoatLockStatus(void);
        bool isBoat(QString idu);
        Grib * getGrib();

        void getBoatWP(double * lat,double * lon);
        bool get_selPOI_instruction();
        void get_selectedBoatPos(double * lat,double* lon);
        int get_selectedBoatVacLen();
        void getBoatBvmg(double * up, double * down, double ws);
        double getBoatPolarSpeed(double ws,double angle);
        double getBoatPolarMaxSpeed();
        boat * getSelectedBoat(void) {if(selectedBoat) return selectedBoat;else return NULL;}
        polarList * getPolarList(void) {return polar_list;}
        QList<POI *> * getPois();       

        FCT_GET(Progress*,progress)
        FCT_GET(StatusBar*,statusBar)
        FCT_GET(int,nxtVac_cnt)

        void getXY(int *X,int *Y){*X=this->mouseClicX;*Y=this->mouseClicY;}
        bool isStartingUp;

        bool getFinishStart(void) { return finishStart; }

        board * getBoard(void) { return myBoard; }
        bool getNoSave(){return noSave;}
        void setPilototoFromRoute(ROUTE * route);
        void setPilototoFromRoute(QList<POI*> poiList);

        void clearPilototo();
        myCentralWidget * getMy_centralWidget(){return this->my_centralWidget;}
        void setRestartNeeded(){this->restartNeeded=true;}
        bool getRestartNeeded(){return this->restartNeeded;}
        void continueSetup();

        QMenu *createPopupMenu(void);

public slots:
        void slotFile_Open();
        void slotFile_Reopen();
        void slotFile_Close();
        void slotFile_Open_Current();
        void slotFile_Close_Current();
        void slotFile_Quit();
        void slotFile_Lock(bool readOnly=false);
        void slotFile_QuitNoSave();
        void slot_gribFileReceived(QString fileName);

        void slotShowContextualMenu(QGraphicsSceneContextMenuEvent *);

        void slotDateStepChanged(int step);
        void slotDateGribChanged_next();
        void slotDateGribChanged_prev();
        void slotDateGribChanged_now(bool b=true);
        void slotDateGribChanged_sel();
        void slotSetGribDate(time_t);

        void slotWindArrows(bool b);

        void slotOptions_Language();
        void slotHelp_Help();
        void slotHelp_Forum();
        void slotHelp_APropos();
        void slotHelp_AProposQT();

        void slotVLM_Sync(void);
        void slotVLM_Param(void);
        void slotVLM_Test(void);
        void slotGribInterpolation(void);
        void slotSelectBoat(boat* newSelect);
        void slotInetUpdated(void);
        void slotChgBoat(int);
        void slotAccountListUpdated(void);
        void slotBoatUpdated(boat * boat,bool newRace,bool doingSync);
        void slot_centerBoat();
        void slot_moveBoat(void);

        void slotChgWP(double lat,double lon, double wph);
        void slotBoatLockStatusChanged(boat*,bool);
        void slotPilototo(void);
        void slotShowPolar(void);

        void slot_newPOI(void);
        void slot_removePOI(void);
        void slotCreatePOI();
        void slotpastePOI();

        void slotParamChanged(void);
        void slotNewZoom(double zoom);
        void slotSelectPOI(DialogPilototoInstruction * instruction);
        void slotSelectWP_POI(void);
        void slot_POIselected(POI* poi);

        void updateNxtVac();

        void slotUpdateOpponent(void);

        void releasePolar(QString fname);
        void slotLoadVLMGrib(void);

        void slotCompassLine(void);
        void slotCompassLineForced(double a,double b);
        void slotCompassCenterBoat(void);
        void slotCompassCenterWp(void);
        void slot_updateGribMono(void);
        void slot_centerMap();
        void slot_positScale();
        void slot_boatHasUpdated(void);
        void slot_updPlayerFinished(bool res_ok, Player * player);
        void slot_copyRoute();
        void slot_deleteRoute();
        void slot_editRoute();
        void slot_pasteRoute();
        void slot_zoomRoute();
        void slot_optimizeRoute();
        void slot_simplifyRoute();

    signals:
        void setChangeStatus(bool);
        void editPOI(POI *);
        void newPOI(double,double,Projection *, boat *);
        void editInstructions(void);
        void editInstructionsPOI(DialogPilototoInstruction * ,POI*);
        void editWP_POI(POI*);
        void boatHasUpdated(boat*);
        void paramVLMChanged();
        void WPChanged(double,double);
        void updateInet(void);
        void showCompassLine(double,double);
        void addPOI_list(POI*);
        void addPOI(QString name,int type,double lat,double lon, double wph,int timestamp,bool useTimeStamp, boat *);
        void updateRoute(boat * boat);
        void showCompassCenterBoat();
        void showCompassCenterWp();
        void selectedBoatChanged();
        void boatChanged(boat *);
        void moveBoat(double lat, double lon);
        void setInstructions(boat * boat,QList<POI *>);
        void wpChanged();


    protected:
        void closeEvent(QCloseEvent *event);
        void keyPressEvent ( QKeyEvent * event );

    private:
        Projection  *proj;

        bool finishStart;

        QString      gribFileName;
        QString      gribFileNameCurrent;
        QString      gribFilePath;

        DialogProxy   * dialogProxy;

        void updatePrevNext(void);

        MenuBar      *menuBar;

        /*
        QToolBar     *toolBar;
        QLabel       * tool_ETA;
        QLabel       * tool_ESTIME;
        QLabel       * tool_ESTIMEUNIT;
        QCheckBox    * startEstime;
        */
        ToolBar * toolBar;

        StatusBar   *statusBar;

        Settings * settings;

        QMenu    *menuPopupBtRight;

        void     connectSignals();

        void    updatePilototo_Btn(boatVLM * boat);
        int     mouseClicX, mouseClicY;

        /* Vacation count*/
        QTimer * timer;
        int nxtVac_cnt;
        bool showingSelectionMessage;

        board * myBoard;
        boat* selectedBoat;
        DialogParamVlm * param;
        DialogPoiInput * poi_input_dialog;

        DialogPilototo * pilototo;

        DialogPilototoInstruction * selPOI_instruction;
        bool isSelectingWP;

        polarList * polar_list;

        DialogVlmGrib * loadVLM_grib;

        /* central widget */
        myCentralWidget * my_centralWidget;

        Progress *progress;
        void closeProgress(void);

        DialogGribValidation * gribValidation_dialog;
        int nBoat;
        int toBeCentered;
        boatVLM *acc;
        void VLM_Sync_sync();

        void listAllChildren(QObject * ptr,int);
        bool noSave;
        bool restartNeeded;
        void updateTitle();       
};

#endif
