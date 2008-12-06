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
#include "DialogProxy.h"
#include "DialogUnits.h"
#include "POI.h"
#include "paramVLM.h"
#include "POI_input.h"
#include "xmlPOIData.h"

//#include "VlmGetData.h"
#include "boatAccount_dialog.h"
#include "vlmDebug.h"

class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(int w, int h, QWidget *parent = 0);
        ~MainWindow();

        void openGribFile(QString fileName, bool zoom=true);

    public slots:
        void slotOpenMeteotable();
        void slotCreatePOI();
        void slotOpenMeteotablePOI(POI*);

        void slotFile_Open();
        void slotFile_Close();
        void slotFile_Load_GRIB();
        void slotFile_Info_GRIB();
        void slotFile_Quit();
        void slotMap_Quality();
        void slotMap_CitiesNames();
        void slotIsobarsStep();
        void slotGribFileReceived(QString fileName);

        void slotMouseClicked(QMouseEvent * e);
        void slotMouseMoved(QMouseEvent * e);

        void slotDateGribChanged(int id);
        void slotDateGribChanged_next();
        void slotDateGribChanged_prev();

        void slotWindColors(bool b);
        void slotRainColors(bool b);
        void slotCloudColors(bool b);
        void slotHumidColors(bool b);

        void slotWindArrows(bool b);

        void slotOptions_Language();
        void slotHelp_Help();
        void slotHelp_APropos();
        void slotHelp_AProposQT();

        void slotVLM_Sync(void);
        void slotVLM_ParamBoat(void);
        void slotVLM_Param(void);
        void slotVLM_Test(void);
        void slotShowMessage(QString msg);
        void slotSelectBoat(boatAccount* newSelect);
        void slotProxyUpdated(void);
        void slotChgBoat(QString);
        void slotAccountListUpdated(void);
        void slotBoatUpdated(boatAccount * boat);
        void slotpastePOI();
        void slotChgWP(float lat,float lon, float wph=-1);
        void slotAddPOI(float lat,float lon, float wph=-1);
        void slotPOIinput(void);
        void slotDelPOIs(void);

        void addPOI_list(POI * poi);
        void delPOI_list(POI * poi);

    signals:
        void signalProjectionUpdated(Projection *proj);
        void signalMapQuality(int quality);

    private:
        GshhsReader *gshhsReader;
        Projection  *proj;

        QString      gribFileName;
        QString      gribFilePath;

        DialogLoadGrib  dialogLoadGrib;
        DialogProxy     dialogProxy;
        DialogUnits     dialogUnits;
        DialogGraphicsParams dialogGraphicsParams;

        Terrain      *terre;
        MenuBar      *menuBar;
        QToolBar     *toolBar;
        QStatusBar   *statusBar;

        QMenu    *menuPopupBtRight;

        void        connectSignals();
        void        InitActionsStatus();
        void        statusBar_showSelectedZone();
        QString    dataPresentInGrib(GribReader* grib, int type);

        int mouseClicX, mouseClicY;

        //---------------------------------------------
        QLabel     lbLon;
        QLabel     lbLat;
        QLabel     lbPres;
        QLabel     lbTemp;
        QLabel     lbWindDir;
        QLabel     lbWindSpeed;
        QLabel     lbWindBf;
        QLabel     lbRain;
        QLabel     lbCloud;
        QLabel     lbHumid;

        void closeEvent(QCloseEvent *) {QApplication::quit();};

        void showMessage(QString msg);

        QList<boatAccount*> acc_list;
//        VlmGetData * vlmData;
        boatAccount_dialog * boatAcc;
        vlmDebug * dbg;
        boardVLM * VLMBoard;
        boatAccount* selectedBoat;
        paramVLM * param;
        POI_input * poi_input_dialog;

        xml_POIData * xmlPOI;

        QList<POI*> poi_list;

};

#endif
