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

#include <QLibrary>

#include "GshhsReader.h"
#include "Terrain.h"
#include "MenuBar.h"
#include "BoardVLM.h"

#include "DialogGraphicsParams.h"
#include "DialogLoadGrib.h"
#include "DialogVLM_grib.h"
#include "DialogProxy.h"
#include "DialogUnits.h"
#include "POI.h"
#include "paramVLM.h"
#include "POI_input.h"
#include "mapcompass.h"
#include "xmlPOIData.h"
#include "xmlBoatData.h"

#include "boatAccount_dialog.h"
#include "POI_editor.h"
#include "Pilototo.h"
#include "race_dialog.h"
#include "Polar.h"
#include "gate.h"
#include "gate_editor.h"

class opponentList;
class raceData;

#include "opponentBoat.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(int w, int h, QWidget *parent = 0);
        ~MainWindow();

        void openGribFile(QString fileName, bool zoom=true);
        bool getBoatLockStatus(void);
        bool isBoat(QString idu);

        void getBoatWP(float * lat,float * lon);
        bool get_selPOI_instruction();


    public slots:
        void slotFile_Open();
        void slotFile_Close();
        void slotFile_Load_GRIB();
        void slotFile_Info_GRIB();
        void slotFile_Quit();
        void slotMap_Quality();
        void slotMap_CitiesNames();
        void slotGribFileReceived(QString fileName);

        void slotMouseClicked(QMouseEvent * e);
        void slotMouseDblClicked(QMouseEvent * e);
        void slotMouseMoved(QMouseEvent * e);
        void slotShowContextualMenu(QContextMenuEvent *);

        void  slotDateStepChanged(int);
        void slotDateGribChanged_next();
        void slotDateGribChanged_prev();
        void slotDateGribChanged_now();
        void slotDateGribChanged_sel();
        void slotSetGribDate(int);

        void slotWindColors(bool b);
        void slotWindArrows(bool b);

        void slotOptions_Language();
        void slotHelp_Help();
        void slotHelp_APropos();
        void slotHelp_AProposQT();

        void slotVLM_Sync(void);
        void slotVLM_ParamBoat(void);
        void slotVLM_ParamRace(void);
        void slotVLM_Param(void);
        void slotVLM_Test(void);
        void slotSelectBoat(boatAccount* newSelect);
        void slotInetUpdated(void);
        void slotChgBoat(int);
        void slotAccountListUpdated(void);
        void slotBoatUpdated(boatAccount * boat,bool newRace);

        void slotChgWP(float lat,float lon, float wph);
        void slotPOIinput(void);
        void slotPOISave(void);
        void slotPOIimport(void);
        void slotBoatLockStatusChanged(boatAccount*,bool);
        void slotPilototo(void);

        void slot_addGate(void);
        void addGate_list(gate*);
        void delGate_list(gate*);
        void slotEditGate(gate *);
        void slotCreateGate();

        void slot_newPOI(void);
        void addPOI_list(POI * poi);
        void delPOI_list(POI * poi);
        void slotEditPOI(POI *);
        void slotDelAllPOIs(void);
        void slotDelSelPOIs(void);
        void slotCreatePOI();
        void slotAddPOI(QString name,POI::POI_TYPE type,float lat,float lon, float wph,int timestamp,bool useTimeStamp);
        void slotpastePOI();

        void slotReadBoat(void);
        void slotWriteBoat(void);
        void slotParamChanged(void);
        void slotNewZoom(float zoom);
        void slotGetTrace(QString buff,QList<position*> * trace);
        void slotSelectPOI(Pilototo_instruction * instruction);
        void slotSelectWP_POI(void);
        void slotPOIselected(POI* poi);

        void updateNxtVac();

        void getPolar(QString fname,Polar ** ptr);
        void releasePolar(QString fname);
        void slotLoadVLMGrib(void);

        void slotValidationDone(bool);

        void slotCompassLine(void);

    signals:
        void signalMapQuality(int quality);
        void setChangeStatus(bool);
        void editPOI(POI *);
        void newPOI(float,float,Projection *);
        void editGate(gate*);
        void newGate(float,float,float,float,Projection*);
        void editInstructions(void);
        void editInstructionsPOI(Pilototo_instruction * ,POI*);
        void editWP_POI(POI*);
        void boatHasUpdated(boatAccount*);
        void paramVLMChanged();
        void WPChanged(float,float);
        void getTrace(QString buff,QList<position*> * trace);
        void updateInet(void);
        void showCompassLine(double,double,double);

    private:
        GshhsReader *gshhsReader;
        Projection  *proj;

        QString      gribFileName;
        QString      gribFilePath;

        DialogLoadGrib  dialogLoadGrib;
        DialogProxy     dialogProxy;
        DialogUnits     dialogUnits;
        DialogGraphicsParams dialogGraphicsParams;

        void updatePrevNext(void);
        int getGribStep(void);

        Terrain      *terre;
        MenuBar      *menuBar;
        QToolBar     *toolBar;
        QStatusBar   *statusBar;
        QLabel       *stBar_label_1;
        QLabel       *stBar_label_2;
        QLabel       *stBar_label_3;

        QLabel       * tool_ETA;
        //QPushButton  * btn_Pilototo;

        QMenu    *menuPopupBtRight;

        void     connectSignals();
        void     InitActionsStatus();
        void     statusBar_showSelectedZone();
        void     statusBar_showWindData(double x,double y);
        QString  dataPresentInGrib(Grib* grib, int type);
        void     updatePilototo_Btn(boatAccount * boat);

        int mouseClicX, mouseClicY;

        void  keyPressEvent (QKeyEvent *e);

        /* Vacation count*/
        QTimer * timer;
        int nxtVac_cnt;
        void drawVacInfo(void);

        void closeEvent(QCloseEvent *) {QApplication::quit();};

        QList<boatAccount*> acc_list;
        boatAccount_dialog * boatAcc;
        boardVLM * VLMBoard;
        boatAccount* selectedBoat;
        paramVLM * param;
        race_dialog * raceParam;
        POI_input * poi_input_dialog;

        xml_boatData * xmlData;
        xml_POIData * xmlPOI;

        QList<POI*> poi_list;
        POI_Editor * poi_editor;

        Pilototo * pilototo;

        opponentList * opponents;
        QList<raceData*> race_list;

        Pilototo_instruction * selPOI_instruction;
        bool isSelectingWP;

        polarList * polar_list;

        DialogVLM_grib * loadVLM_grib;

        mapCompass * compass;

        gate_editor * gate_edit;
        QList<gate*> gate_list;
};

#endif
