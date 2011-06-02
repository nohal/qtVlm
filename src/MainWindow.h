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

#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>
#include <QProgressDialog>
#include <QGraphicsSceneContextMenuEvent>

#include <QLibrary>

#include "class_list.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(int w, int h, QWidget *parent = 0);
        ~MainWindow();

        void openGribFile(QString fileName, bool zoom=true);
        bool getBoatLockStatus(void);
        bool isBoat(QString idu);
        Grib * getGrib();

        void getBoatWP(double * lat,double * lon);
        bool get_selPOI_instruction();
        void get_selectedBoatPos(double * lat,double* lon);
        int get_selectedBoatVacLen();
        void getBoatBvmg(float * up, float * down, float ws);
        float getBoatPolarSpeed(float ws,float angle);
        float getBoatPolarMaxSpeed();
        boat * getSelectedBoat(void) {if(selectedBoat) return selectedBoat;else return NULL;}
        polarList * getPolarList(void) {return polar_list;}

        void statusBar_showWindData(double x,double y);
        void statusBar_showSelectedZone(float x0, float y0, float x1, float y1);
        void drawVacInfo(void);

        void setBoardToggleAction(QAction * action);
        void getXY(int *X,int *Y){*X=this->mouseClicX;*Y=this->mouseClicY;}
        bool isStartingUp;

        bool getFinishStart(void) { return finishStart; }

        board * getBoard(void) { return myBoard; }
        bool getNoSave(){return noSave;}

    public slots:
        void slotFile_Open();
        void slotFile_Close();
        void slotFile_Quit();
        void slotFile_QuitNoSave();
        void slotMap_Quality();
        void slot_gribFileReceived(QString fileName);

        void slotShowContextualMenu(QGraphicsSceneContextMenuEvent *);

        void slotDateStepChanged(int);
        void slotDateGribChanged_next();
        void slotDateGribChanged_prev();
        void slotDateGribChanged_now(bool b=true);
        void slotDateGribChanged_sel();
        void slotSetGribDate(time_t);

        void slotWindArrows(bool b);

        void slotOptions_Language();
        void slotHelp_Help();
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

        void slotChgWP(float lat,float lon, float wph);
        void slotBoatLockStatusChanged(boat*,bool);
        void slotPilototo(void);
        void slotShowPolar(void);

        void slot_newPOI(void);        
        void slotCreatePOI();
        void slotpastePOI();
        //void slotMovePOI(POI *);

        void slotParamChanged(void);
        void slotNewZoom(float zoom);
        void slotSelectPOI(DialogPilototoInstruction * instruction);
        void slotSelectWP_POI(void);
        void slot_POIselected(POI* poi);

        void updateNxtVac();

        void slotUpdateOpponent(void);

        void releasePolar(QString fname);
        void slotLoadVLMGrib(void);

        void slotCompassLine(void);
        void slotCompassLineForced(int a,int b);
        void slotCompassCenterBoat(void);
        void slotCompassCenterWp(void);
        void slotEstime(int);
        void slot_ParamVLMchanged(void);
        void slot_deleteProgress(void);
        void slot_centerMap();
        void slot_boatHasUpdated(void);
        void slot_updPlayerFinished(bool res_ok, Player * player);

    signals:
        void signalMapQuality(int quality);
        void setChangeStatus(bool);
        void editPOI(POI *);
        void newPOI(float,float,Projection *, boat *);
        void editInstructions(void);
        void editInstructionsPOI(DialogPilototoInstruction * ,POI*);
        void editWP_POI(POI*);
        void boatHasUpdated(boat*);
        void paramVLMChanged();
        void WPChanged(double,double);
        void updateInet(void);
        void showCompassLine(int,int);
        void addPOI_list(POI*);
        void addPOI(QString name,int type,float lat,float lon, float wph,int timestamp,bool useTimeStamp, boat *);
        void updateRoute(boat * boat);
        void showCompassCenterBoat();
        void showCompassCenterWp();
        void selectedBoatChanged();
        void boatChanged(boat *);
        void moveBoat(double lat, double lon);


    protected:
        void closeEvent(QCloseEvent *) {QApplication::quit();}
        void keyPressEvent ( QKeyEvent * event );
        //bool eventFilter(QObject * obj, QEvent * e);

    private:
        Projection  *proj;

        bool finishStart;

        QString      gribFileName;
        QString      gribFilePath;

        DialogProxy   * dialogProxy;

        void updatePrevNext(void);
        int getGribStep(void);

        MenuBar      *menuBar;
        QToolBar     *toolBar;
        QStatusBar   *statusBar;
        QLabel       *stBar_label_1;
        QLabel       *stBar_label_2;
        QLabel       *stBar_label_3;

        QLabel       * tool_ETA;
        QLabel       * tool_ESTIME;
        QLabel       * tool_ESTIMEUNIT;
        //QPushButton  * btn_Pilototo;

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
        QProgressDialog *progress;
        QTimer * timerprogress;

        DialogGribValidation * gribValidation_dialog;
        int nBoat;
        int toBeCentered;
        boatVLM *acc;
        void VLM_Sync_sync();

        void listAllChildren(QObject * ptr,int);
        bool noSave;
};

#endif
