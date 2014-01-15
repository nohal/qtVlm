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
#include "MainWindowInterface.h"
//#include <QPluginLoader>
class MainWindow: public MainWindowInterface
{
    Q_OBJECT

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

        void openGribFile(QString fileName, bool zoom=true, bool current=false);
        bool getBoatLockStatus(void);
        bool isBoat(QString idu);

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
        FCT_GET(ToolBar*,toolBar)

        FCT_GET(int,nxtVac_cnt)

        int get_boatType(void);

        /*** Barrier ***/

        void getXY(int *X,int *Y){*X=this->mouseClicX;*Y=this->mouseClicY;}
        bool isStartingUp;

        bool getFinishStart(void) { return finishStart; }


        bool getNoSave(){return noSave;}
        void setPilototoFromRoute(ROUTE * route);
        void setPilototoFromRoute(QList<POI*> poiList);

        myCentralWidget * getMy_centralWidget(){return this->my_centralWidget;}
        void continueSetup();

        QMenu *createPopupMenu(void);

        void loadBoard();
        void showDashBoard();
        BoatInterface * get_selectedBoatInterface(){return (BoatInterface*)this->getSelectedBoat();}
        QColor getWindColorStatic(const double &v, const bool &smooth=true);
        QVariant getSettingApp(const int &key) const;
        QString get_folder(QString str) const;
        void showContextualMenu(const int &xPos, const int &yPos);
        QPalette getOriginalPalette() const;
        void setting_saveGeometry(QWidget * obj);
        bool getWPClipboard(QString *,double * lat,double * lon, double * wph, int * tStamp);
        void setWPClipboard(double lat,double lon, double wph);
        QString pos2String(const int &type,const double &value);
        QString formatLongitude(const double &x);
        QString formatLatitude(const double &y);
        void setFontDialog(QWidget * o);

public slots:
        void slot_POI_input();
        void slot_showPOI_input(POI *poi=NULL, const bool &fromMenu=false);        
        void slot_disablePopupMenu();
        void slotFile_Open();
        void slotFile_Reopen();
        void slotFile_Close();
        void slotFile_Open_Current();
        void slotFile_Close_Current();
        void slotFile_Quit();
        void slotFile_Lock(bool readOnly=false);
        void slotFile_QuitNoSave();
        void slot_gribFileReceived(QString fileName);
        void slotCombineGrib();

        void slot_clearPilototo();

        void slotShowContextualMenu(QGraphicsSceneContextMenuEvent *);

        void slotDateStepChanged(int step);
        void slotDateGribChanged_next();
        void slotDateGribChanged_prev();
        void slotDateGribChanged_now(bool b=true);
        void slotDateGribChanged_sel();
        void slotSetGribDate(time_t);

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
        void slot_centerSelectedBoat();
        void slot_moveBoat(void);

        void slotChgWP(double lat,double lon, double wph);
        void slotBoatLockStatusChanged(boat*,bool);
        void slotPilototo(void);
        void slotShowPolar(void);
        void slot_removePOI(void);
        void slot_newPOI(void);
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
        void slot_poiRoute();
        void slot_pasteRoute();
        void slot_routeComparator();
        void slot_zoomRoute();
        void slot_optimizeRoute();
        void slot_simplifyRouteMax();
        void slot_simplifyRouteMin();

        /*** Barrier ***/
        void slot_newBarrierSet();
        void slot_barrierAddPopup(void);
        void slot_barrierAddMenu(void);


        void slot_removeRoute();
        void slot_execDialogProxy();
signals:
        void setChangeStatus(bool status,bool pilototo,bool syncBtn);
        void outDatedVlmData(void);
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
        void addPOI(QString name,int type,double lat,double lon, double wph,int timestamp,bool useTimeStamp);
        void updateRoute(boat * boat);
        void showCompassCenterBoat();
        void showCompassCenterWp();
        void selectedBoatChanged();
        void boatChanged(boat *);
        void moveBoat(double lat, double lon);
        void setInstructions(boat * boat,QList<POI *>);
        void wpChanged();
        void boatSelected(boat*);
        void accountListUpdated(Player*);
        void selectPOI(bool);
        void updateLockIcon(QString ic);


    protected:
        void closeEvent(QCloseEvent *event);
        void keyPressEvent ( QKeyEvent * event );

    private:
        Projection  *proj;

        bool finishStart;

        QString      gribFileName;
        QString      gribFileNameCurrent;
        QString      gribFilePath;


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

        //Board * board;
        boat* selectedBoat;
        DialogProxy   * dialogProxy;
        DialogParamVlm * param;
        //DialogPoiInput * poi_input_dialog;
        DialogPilototo * pilototo;
        DialogPilototoInstruction * selPOI_instruction;
        DialogVlmGrib * loadVLM_grib;
        DialogGribValidation * gribValidation_dialog;
        bool isSelectingWP;

        int boatType;

        polarList * polar_list;


        /* central widget */
        myCentralWidget * my_centralWidget;

        Progress *progress;
        void closeProgress(void);

        int nBoat;
        int toBeCentered;
        boatVLM *acc;
        void VLM_Sync_sync();

        void listAllChildren(QObject * ptr,int);
        bool noSave;
        void updateTitle();
        BoardInterface * boardPlugin;
        board * myBoard;
        bool use_old_board;

        void loadGrib2();

        QString get_OSVersion(void);
        QPalette originalPalette;
        //QPluginLoader * pluginLoader;
};

#endif
