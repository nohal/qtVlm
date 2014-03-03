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

#include <cmath>

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QToolBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QDockWidget>
#include <QRegExp>
#include <QDebug>
#include <QTimer>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QClipboard>
#include <QResource>

#ifdef QT_V5
#include <QUiLoader>
#else
#include <QtUiTools/QUiLoader>
#endif

#include <iostream>
#include <fstream>

#include "MainWindow.h"
#include "Util.h"
#include "Orthodromie.h"
#include "Version.h"
#include "settings.h"
#include "opponentBoat.h"
#include "mycentralwidget.h"
#include "MenuBar.h"
//#include "GshhsReader.h"
#include "Polar.h"
#include "boatVLM.h"
#include "GribRecord.h"
#include "POI.h"
#include "Projection.h"
#include "Terrain.h"
#include "boatReal.h"
#include "route.h"
#include "ToolBar.h"
#include "Progress.h"
#include "StatusBar.h"
#include "BarrierSet.h"
#include "BoardInterface.h"
#include "Board.h"
#include "BoardVLM.h"
#include "BoardReal.h"
#include "MapDataDrawer.h"
#include "DataColors.h"
#include "DataManager.h"

#include "DialogPoiDelete.h"
#include "DialogPoi.h"
#include "DialogGribValidation.h"
#include "DialogPoiInput.h"
#include "DialogProxy.h"
#include "DialogVlmGrib_ctrl.h"
#include "DialogParamVlm.h"
#include "DialogPilototo.h"
#include "dialogviewpolar.h"
#include "DialogEditBarrier.h"
#include "DialogRouteComparator.h"
#include "DialogWp.h"
#include <QStyleFactory>
//#include <QtQuick/QQuickView>
//#include <QPluginLoader>
int INTERPOLATION_DEFAULT=INTERPOLATION_HYBRID;


//-----------------------------------------------------------
// Connexions des signaux
//-----------------------------------------------------------
void MainWindow::connectSignals()
{
    MenuBar  *mb = menuBar;

    //-------------------------------------
    // Actions
    //-------------------------------------
    connect(mb->acHorn, SIGNAL(triggered()), my_centralWidget, SLOT(slot_editHorn()));
    connect(mb->acKeep, SIGNAL(toggled(bool)), my_centralWidget, SLOT(slot_keepPos(bool)));
    connect(mb->acReplay, SIGNAL(triggered()), my_centralWidget, SLOT(slot_startReplay()));
    connect(mb->acScreenshot, SIGNAL(triggered()), my_centralWidget, SLOT(slot_takeScreenshot()));
    connect(mb->acShowLog, SIGNAL(triggered()), my_centralWidget, SLOT(slot_showVlmLog()));
    connect(mb->acGetTrack, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fetchVLMTrack()));
    connect(mb->ac_CreatePOI, SIGNAL(triggered()), this, SLOT(slotCreatePOI()));
    connect(mb->ac_pastePOI, SIGNAL(triggered()), this, SLOT(slotpastePOI()));


    connect(mb->ac_moveBoat, SIGNAL(triggered()), this,SLOT(slot_moveBoat()));

    connect(mb->acFile_Open, SIGNAL(triggered()), this, SLOT(slotFile_Open()));
    connect(mb->acFile_Reopen, SIGNAL(triggered()), this, SLOT(slotFile_Reopen()));
    connect(mb->acFile_Close, SIGNAL(triggered()), this, SLOT(slotFile_Close()));
    connect(mb->acFile_Open_Current, SIGNAL(triggered()), this, SLOT(slotFile_Open_Current()));
    connect(mb->acFile_Close_Current, SIGNAL(triggered()), this, SLOT(slotFile_Close_Current()));
    connect(mb->acFile_Load_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileLoad_GRIB()));
    connect(mb->acFile_Load_VLM_GRIB, SIGNAL(triggered()), this, SLOT(slotLoadVLMGrib()));
    connect(mb->acFile_Load_SAILSDOC_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slotLoadSailsDocGrib()));
    connect(mb->acFile_Info_GRIB_main, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileInfo_GRIB_main()));
    connect(mb->acFile_Info_GRIB_current, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileInfo_GRIB_current()));
    connect(mb->acGrib_dialog, SIGNAL(triggered()), my_centralWidget, SLOT(slot_gribDialog()));
    connect(mb->acFile_Quit, SIGNAL(triggered()), this, SLOT(slotFile_Quit()));
    connect(mb->acFile_Lock, SIGNAL(triggered()), this, SLOT(slotFile_Lock()));    
    connect(this,SIGNAL(updateLockIcon(QString)),mb,SLOT(slot_updateLockIcon(QString)));
    connect(mb->acFile_QuitNoSave, SIGNAL(triggered()), this, SLOT(slotFile_QuitNoSave()));
    connect(mb->acCombineGrib, SIGNAL(triggered()), this, SLOT(slotCombineGrib()));


    connect(mb->acFax_Open, SIGNAL(triggered()), my_centralWidget, SLOT(slotFax_open()));
    connect(mb->acFax_Close, SIGNAL(triggered()), my_centralWidget, SLOT(slotFax_close()));
    connect(mb->acImg_Open, SIGNAL(triggered()), my_centralWidget, SLOT(slotImg_open()));
    connect(mb->acImg_Close, SIGNAL(triggered()), my_centralWidget, SLOT(slotImg_close()));

    //-------------------------------------------------------

    connect(mb->acOptions_Proxy, SIGNAL(triggered()), this, SLOT(slot_execDialogProxy()));

    connect(mb->acOptions_GroupLanguage, SIGNAL(triggered(QAction *)),
            this, SLOT(slotOptions_Language()));

    //-------------------------------------------------------
    connect(mb->acHelp_Help, SIGNAL(triggered()), this, SLOT(slotHelp_Help()));
    connect(mb->acHelp_APropos, SIGNAL(triggered()), this, SLOT(slotHelp_APropos()));
    connect(mb->acHelp_AProposQT, SIGNAL(triggered()), this, SLOT(slotHelp_AProposQT()));
    connect(mb->acHelp_Forum,SIGNAL(triggered()),this,SLOT(slotHelp_Forum()));

    //-------------------------------------------------------
    connect(mb->acVLMParamBoat, SIGNAL(triggered()), my_centralWidget, SLOT(slot_boatDialog()));
    connect(mb->acVLMParamPlayer, SIGNAL(triggered()), my_centralWidget, SLOT(slot_manageAccount()));
    connect(mb->acRace, SIGNAL(triggered()), my_centralWidget, SLOT(slot_raceDialog()));
    connect(mb->acVLMParam, SIGNAL(triggered()), this, SLOT(slotVLM_Param()));
    connect(mb->acVLMSync, SIGNAL(triggered()), this, SLOT(slotVLM_Sync()));

    connect(mb->acPOIAdd, SIGNAL(triggered()), this, SLOT(slot_newPOI()));
    connect(mb->acPOIRemove, SIGNAL(triggered()), this, SLOT(slot_removePOI()));
    connect(mb->acPOIRemoveByType, SIGNAL(triggered()), my_centralWidget, SLOT(slot_removePOIType()));
    connect(mb->ac_twaLine,SIGNAL(triggered()), my_centralWidget, SLOT(slot_twaLine()));
    connect(mb->ac_compassLine,SIGNAL(triggered()), this, SLOT(slotCompassLine()));
    connect(mb->ac_compassCenterBoat,SIGNAL(triggered()), this, SLOT(slotCompassCenterBoat()));
    connect(mb->ac_compassCenterWp,SIGNAL(triggered()), this, SLOT(slotCompassCenterWp()));
    connect(mb->ac_centerMap,SIGNAL(triggered()), this, SLOT(slot_centerMap()));
    connect(mb->ac_positScale,SIGNAL(triggered()), this, SLOT(slot_positScale()));

    connect(mb->ac_copyRoute,SIGNAL(triggered()), this, SLOT(slot_copyRoute()));
    connect(mb->ac_deleteRoute,SIGNAL(triggered()), this, SLOT(slot_deleteRoute()));
    connect(mb->ac_editRoute,SIGNAL(triggered()), this, SLOT(slot_editRoute()));
    connect(mb->ac_poiRoute,SIGNAL(triggered()), this, SLOT(slot_poiRoute()));
    connect(mb->ac_simplifyRouteMax,SIGNAL(triggered()), this, SLOT(slot_simplifyRouteMax()));
    connect(mb->ac_simplifyRouteMin,SIGNAL(triggered()), this, SLOT(slot_simplifyRouteMin()));
    connect(mb->ac_optimizeRoute,SIGNAL(triggered()), this, SLOT(slot_optimizeRoute()));
    connect(mb->ac_pasteRoute,SIGNAL(triggered()), this, SLOT(slot_pasteRoute()));
    connect(mb->acRoute_paste,SIGNAL(triggered()), this, SLOT(slot_pasteRoute()));
    connect(mb->acRoute_comparator,SIGNAL(triggered()), this, SLOT(slot_routeComparator()));
    connect(mb->acRouteRemove, SIGNAL(triggered()), this, SLOT(slot_removeRoute()));
    connect(mb->ac_zoomRoute,SIGNAL(triggered()), this, SLOT(slot_zoomRoute()));
#ifdef __QTVLM_WITH_TEST
    if(mb->acVLMTest)
        connect(mb->acVLMTest, SIGNAL(triggered()), this, SLOT(slotVLM_Test()));
    if(mb->acGribInterpolation)
        connect(mb->acGribInterpolation, SIGNAL(triggered()), this, SLOT(slotGribInterpolation()));
#endif
    connect(mb->acPOIinput, SIGNAL(triggered()), this, SLOT(slot_POI_input()));
    connect(mb->acPilototo, SIGNAL(triggered()), this, SLOT(slotPilototo()));
    connect(mb->acShowPolar, SIGNAL(triggered()),this,SLOT(slotShowPolar()));

    connect(mb->acPOIimport, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIimport()));
    connect(mb->acPOIgeoData, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIimportGeoData()));


    connect(mb->acPOISave, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POISave()));
    connect(mb->acPOIRestore, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIRestore()));

    /*** Barrier ***/
    connect(mb->ac_addBarrierSet,SIGNAL(triggered()),this,SLOT(slot_newBarrierSet()));
    connect(mb->ac_addBarrier,SIGNAL(triggered()),my_centralWidget,SLOT(slot_addBarrier()));
    connect(mb->ac_popupBarrier,SIGNAL(triggered()),this,SLOT(slot_barrierAddPopup()));

    //-------------------------------------
    // Autres signaux
    //-------------------------------------

    connect(this,SIGNAL(setChangeStatus(bool,bool,bool)),mb,SLOT(slot_setChangeStatus(bool,bool,bool)));
}
void MainWindow::slot_execDialogProxy()
{
    dialogProxy = new DialogProxy(my_centralWidget);
    connect(dialogProxy, SIGNAL(proxyUpdated()), this, SLOT(slotInetUpdated()));
    dialogProxy->exec();
    delete dialogProxy;
}



//----------------------------------------------------
void MainWindow::slot_gribFileReceived(QString fileName)
{
    bool zoom =  (Settings::getSetting(gribZoomOnLoad).toInt()==1);
    openGribFile(fileName, zoom);
    if(my_centralWidget) my_centralWidget->fileInfo_GRIB(DataManager::GRIB_GRIB);
    updateTitle();
}

//=============================================================
MainWindow::MainWindow(QWidget *parent)
{
    this->setParent(parent);
    setWindowIcon (QIcon (appFolder.value("icon")+"qtVlm_48x48.png"));
    noSave=false;
    originalPalette=QApplication::palette();
    loadVLM_grib=NULL;
    qWarning()<<QStyleFactory::keys();
#ifdef QT_V5
#ifdef __ANDROID__
    QStyle * android=QStyleFactory::create("Android");
    qApp->setStyle(android);
#if 0
    QPalette p=qApp->palette();
    p.setColor(QPalette::Window, QColor(53,53,53));
    p.setColor(QPalette::Button, QColor(53,53,53));
    p.setColor(QPalette::Base, QColor(53,53,53));
    p.setColor(QPalette::Highlight, QColor(142,45,197));
    p.setColor(QPalette::ButtonText, QColor(234,221,21));
    p.setColor(QPalette::WindowText, QColor(255,255,255));
    p.setColor(QPalette::Text, QColor(255,255,255));
    p.setColor(QPalette::AlternateBase,QColor(100,100,100));
    QBrush brush = p.window();
    brush.setColor(brush.color().light(300));
    QColor dis=brush.color();
    p.setColor(QPalette::Disabled, QPalette::Text, dis); // color menu item base
    p.setColor(QPalette::Disabled, QPalette::Light, QColor(53,53,53));
    p.setColor(QPalette::Disabled, QPalette::WindowText, dis);
    qApp->setPalette(p);
    QString myColor1="#323232"; // dark grey
    QString myColor2="#5a5712"; // yellow dark
    QString myColor3="#6b0b86"; // violet legerement fonce
    QString myColor4="#b1b1b1"; // grey
    QString myColor5="#ebe300"; // yellow light
    QString myColor6="#d7d017"; // yellow
    QString myColor7="#ffffff"; // white
    QString customStyle=""
            "QProgressBar{"
                "text-align: center;}"

            "QProgressBar::chunk{"
                "background-color: myColor6;}"

            "QWidget:item:hover{"
                "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor6, stop: 0.7 myColor2, stop: 1 myColor6);"
                "color: myColor7;}"

            "QRadioButton::indicator:checked, QRadioButton::indicator:unchecked{"
                "color: myColor4;"
                "background-color: myColor1;"
                "border: solid myColor4;}"

            "QRadioButton::indicator:checked{"
                "background-color: qradialgradient("
                    "cx: 0.5, cy: 0.5,"
                    "fx: 0.5, fy: 0.5,"
                    "radius: 1.0,"
                    "stop: 0.25 myColor6,"
                    "stop: 0.3 myColor1);}"

            "QCheckBox::indicator, QGroupBox::indicator, QTreeView::indicator{"
                "color: myColor4;"
                "background-color: myColor1;"
                "border: solid myColor4;}"

            "QCheckBox::indicator:indeterminate, QGroupBox::indicator:indeterminate, QTreeView::indicator:indeterminate{"
                "color: myColor4;"
                "background-color:  myColor6;"
                "border: solid myColor4;}"


            "QRadioButton::indicator:hover, QCheckBox::indicator:hover, QGroupBox::indicator:hover, QTreeView::indicator:hover{"
                "border: solid myColor6;}"


            "QCheckBox::indicator:disabled, QGroupBox::indicator:disabled, QRadioButton::indicator:disabled,  QTreeView::indicator:disabled{"
                "border: solid #444;}"

    "QTabBar::tab {"
        "color: myColor4;"
        "border: solid #444;"
        "border-bottom-style: none;"
        "background-color: myColor1;}"

    "QTabWidget::pane {"
        "border: solid #444;}"

    "QTabBar::tab:!selected{"
        "color: myColor4;"
        "border-bottom-style: solid;"
        "background-color: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:1 #212121, stop:.4 #343434);}"

    "QTabBar::tab:selected{"
        "color: myColor6;}"

    "QTabBar::tab:!selected:hover{"
        "background-color: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:1 #212121, stop:0.4 #343434, stop:0.2 #343434, stop:0.1 myColor6);}"


            "QScrollBar:horizontal {"
                 "border: solid #222222;"
                 "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0.0 #121212, stop: 0.2 #282828, stop: 1 #484848);}"

            "QScrollBar::handle:horizontal{"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 0.5 myColor2, stop: 1 myColor5);}"

            "QScrollBar::add-line:horizontal {"
                  "border: solid #1b1b19;"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 1 myColor2);"
                  "subcontrol-position: right;"
                  "subcontrol-origin: margin;}"

            "QScrollBar::sub-line:horizontal {"
                  "border: solid #1b1b19;"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 1 myColor2);"
                  "subcontrol-position: left;"
                  "subcontrol-origin: margin;}"

            "QScrollBar::right-arrow:horizontal, QScrollBar::left-arrow:horizontal{"
                  "border: solid black;"
                  "background: white;}"

            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{"
                  "background: none;}"

            "QScrollBar:vertical{"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0.0 #121212, stop: 0.2 #282828, stop: 1 #484848);"
                  "border: solid #222222;}"

            "QScrollBar::handle:vertical{"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor5, stop: 0.5 myColor2, stop: 1 myColor5);}"

            "QScrollBar::add-line:vertical{"
                  "border: solid #1b1b19;"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor5, stop: 1 myColor2);"
                  "subcontrol-position: bottom;"
                  "subcontrol-origin: margin;}"

            "QScrollBar::sub-line:vertical{"
                  "border: solid #1b1b19;"
                  "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor2, stop: 1 myColor5);"
                  "subcontrol-position: top;"
                  "subcontrol-origin: margin;}"

            "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical{"
                  "border: solid black;"
                  "background: white;}"


            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{"
                  "background: none;}";

    customStyle.replace("myColor1",myColor1);
    customStyle.replace("myColor2",myColor2);
    customStyle.replace("myColor3",myColor3);
    customStyle.replace("myColor4",myColor4);
    customStyle.replace("myColor5",myColor5);
    customStyle.replace("myColor6",myColor6);
    customStyle.replace("myColor7",myColor7);
    qApp->setStyleSheet(customStyle);
#endif
#else
    if(Settings::getSetting(fusionStyle).toInt()==1)
    {
        qWarning()<<"setting up Black fusion style";
        QStyle * fusion=QStyleFactory::create("Fusion");
        qApp->setStyle(fusion);
        QPalette p=qApp->palette();
        p.setColor(QPalette::Window, QColor(53,53,53));
        p.setColor(QPalette::Button, QColor(53,53,53));
        p.setColor(QPalette::Disabled, QPalette::Button, QColor(83,83,83));
        p.setColor(QPalette::Base, QColor(53,53,53));
        p.setColor(QPalette::Highlight, QColor(142,45,197));
        p.setColor(QPalette::ButtonText, QColor(234,221,21));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(247,251,141));
        p.setColor(QPalette::WindowText, QColor(255,255,255));
        p.setColor(QPalette::Text, QColor(255,255,255));
        p.setColor(QPalette::AlternateBase,QColor(100,100,100));
        QBrush brush = p.window();
        brush.setColor(brush.color().light(300));
        QColor dis=brush.color();
        p.setColor(QPalette::Disabled, QPalette::Text, dis); // color menu item base
        p.setColor(QPalette::Disabled, QPalette::Light, QColor(53,53,53));
        p.setColor(QPalette::Disabled, QPalette::WindowText, dis);
        QString myColor1="#323232"; // dark grey
        //QString myColor2="#d7801a"; // orange dark
        QString myColor2="#5a5712"; // yellow dark
        //QString myColor3="#ca0619"; // red with a touch of orange
        QString myColor3="#6b0b86"; // violet legerement fonce
        QString myColor4="#b1b1b1"; // grey
        //QString myColor5="#ffa02f"; // orange light
        QString myColor5="#ebe300"; // yellow light
        //QString myColor6="#ffaa00"; // orange
        QString myColor6="#d7d017"; // yellow
        QString myColor7="#ffffff"; // white
        QString customStyle=""
                "QProgressBar{"
                    "border: 2px solid grey;"
                    "border-radius: 5px;"
                    "text-align: center;}"

                "QProgressBar::chunk{"
                    "background-color: myColor6;"
                    "width: 2.15px;"
                    "margin: 0.5px;}"

                "QWidget:item:hover{"
                    "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor6, stop: 0.7 myColor2, stop: 1 myColor6);"
                    "color: myColor7;}"

                "QRadioButton::indicator:checked, QRadioButton::indicator:unchecked{"
                    "color: myColor4;"
                    "background-color: myColor1;"
                    "border: 2px solid myColor4;"
                    "border-radius: 6px;}"

                "QRadioButton::indicator:checked{"
                    "background-color: qradialgradient("
                        "cx: 0.5, cy: 0.5,"
                        "fx: 0.5, fy: 0.5,"
                        "radius: 1.0,"
                        "stop: 0.25 myColor6,"
                        "stop: 0.3 myColor1);}"

                "QCheckBox::indicator, QGroupBox::indicator, QTreeView::indicator{"
                    "color: myColor4;"
                    "background-color: myColor1;"
                    "border: 2px solid myColor4;"
                    "width: 9px;"
                    "height: 9px;}"

                "QCheckBox::indicator:indeterminate, QGroupBox::indicator:indeterminate, QTreeView::indicator:indeterminate{"
                    "color: myColor4;"
                    "background-color:  myColor6;"
                    "border: 2px solid myColor4;"
                    "width: 9px;"
                    "height: 9px;}"

                "QRadioButton::indicator{"
                    "border-radius: 6px;}"

                "QRadioButton::indicator:hover, QCheckBox::indicator:hover, QGroupBox::indicator:hover, QTreeView::indicator:hover{"
                    "border: 2px solid myColor6;}"

                "QCheckBox::indicator:checked, QGroupBox::indicator:checked, QTreeView::indicator:checked{"
                    "image:url(img/o_checkbox.png);}"

                "QCheckBox::indicator:disabled, QGroupBox::indicator:disabled, QRadioButton::indicator:disabled,  QTreeView::indicator:disabled{"
                    "border: 1px solid #444;}"

        "QTabBar::tab {"
            "color: myColor4;"
            "border: 1px solid #444;"
            "border-bottom-style: none;"
            "background-color: myColor1;"
            "padding-left: 10px;"
            "padding-right: 10px;"
            "padding-top: 3px;"
            "padding-bottom: 2px;"
            "margin-right: -1px;}"

        "QTabWidget::pane {"
            "border: 1px solid #444;"
            "top: 1px;}"

        "QTabBar::tab:last{"
            "margin-right: 0; /* the last selected tab has nothing to overlap with on the right */"
            "border-top-right-radius: 3px;}"

        "QTabBar::tab:first:!selected{"
            "margin-left: 0px; /* the last selected tab has nothing to overlap with on the right */"
            "border-top-left-radius: 3px;}"

        "QTabBar::tab:!selected{"
            "color: myColor4;"
            "border-bottom-style: solid;"
            "margin-top: 3px;"
            "background-color: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:1 #212121, stop:.4 #343434);}"

        "QTabBar::tab:selected{"
            "color: myColor6;"
            "border-top-left-radius: 3px;"
            "border-top-right-radius: 3px;"
            "margin-bottom: 0px;}"

        "QTabBar::tab:!selected:hover{"
            "/*border-top: 2px solid myColor6;"
            "padding-bottom: 3px;*/"
            "border-top-left-radius: 3px;"
            "border-top-right-radius: 3px;"
            "background-color: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:1 #212121, stop:0.4 #343434, stop:0.2 #343434, stop:0.1 myColor6);}"


                "QScrollBar:horizontal {"
                     "border: 1px solid #222222;"
                     "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0.0 #121212, stop: 0.2 #282828, stop: 1 #484848);"
                     "height: 14px;"
                     "margin: 0px 16px 0 16px;}"

                "QScrollBar::handle:horizontal{"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 0.5 myColor2, stop: 1 myColor5);"
                      "min-height: 20px;"
                      "border-radius: 2px;}"

                "QScrollBar::add-line:horizontal {"
                      "border: 1px solid #1b1b19;"
                      "border-radius: 2px;"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 1 myColor2);"
                      "width: 14px;"
                      "subcontrol-position: right;"
                      "subcontrol-origin: margin;}"

                "QScrollBar::sub-line:horizontal {"
                      "border: 1px solid #1b1b19;"
                      "border-radius: 2px;"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 myColor5, stop: 1 myColor2);"
                      "width: 14px;"
                      "subcontrol-position: left;"
                      "subcontrol-origin: margin;}"

                "QScrollBar::right-arrow:horizontal, QScrollBar::left-arrow:horizontal{"
                      "border: 1px solid black;"
                      "width: 1px;"
                      "height: 1px;"
                      "background: white;}"

                "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{"
                      "background: none;}"

                "QScrollBar:vertical{"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0, stop: 0.0 #121212, stop: 0.2 #282828, stop: 1 #484848);"
                      "width: 14px;"
                      "margin: 16px 0 16px 0;"
                      "border: 1px solid #222222;}"

                "QScrollBar::handle:vertical{"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor5, stop: 0.5 myColor2, stop: 1 myColor5);"
                      "min-height: 20px;"
                      "border-radius: 2px;}"

                "QScrollBar::add-line:vertical{"
                      "border: 1px solid #1b1b19;"
                      "border-radius: 2px;"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor5, stop: 1 myColor2);"
                      "height: 14px;"
                      "subcontrol-position: bottom;"
                      "subcontrol-origin: margin;}"

                "QScrollBar::sub-line:vertical{"
                      "border: 1px solid #1b1b19;"
                      "border-radius: 2px;"
                      "background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 myColor2, stop: 1 myColor5);"
                      "height: 14px;"
                      "subcontrol-position: top;"
                      "subcontrol-origin: margin;}"

                "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical{"
                      "border: 1px solid black;"
                      "width: 1px;"
                      "height: 1px;"
                      "background: white;}"


                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{"
                      "background: none;}"
                "QMenu::separator{"
                    "height: 2px;"
                    "background: myColor6;"
                    "margin-left: 10px;"
                    "margin-right: 5px;}";
        customStyle.replace("myColor1",myColor1);
        customStyle.replace("myColor2",myColor2);
        customStyle.replace("myColor3",myColor3);
        customStyle.replace("myColor4",myColor4);
        customStyle.replace("myColor5",myColor5);
        customStyle.replace("myColor6",myColor6);
        customStyle.replace("myColor7",myColor7);
        qApp->setPalette(p);
        qApp->setStyleSheet(customStyle);
    }
    double fontInc=Settings::getSetting(defaultFontSizeInc).toDouble();
    if(Settings::getSetting(defaultFontName).toString()=="-1")
        Settings::setSetting(defaultFontName,Settings::getSettingDefault(defaultFontName));
    QFont def(Settings::getSetting(defaultFontName).toString());
    double fontSize=8.0+fontInc;
    def.setPointSizeF(fontSize);
    QApplication::setFont(def);
#endif
#endif
    isStartingUp=true;
    finishStart=true;
    nBoat=0;
    double prcx,prcy,scale;

    updateTitle();
    selectedBoat = NULL;
    lastUpdateTime=QDateTime::currentDateTimeUtc().toTime_t();

    INTERPOLATION_DEFAULT=Settings::getSetting(defaultInterpolation).toInt();

    qWarning() <<  "Starting qtVlm - " << Version::getCompleteName()+" "+QString().setNum(sizeof(int*)*8)+" bits ";

    /* timer de gestion des VAC */
    timer = new QTimer(this);
    timer->setSingleShot(false);
    nxtVac_cnt=0;
    connect(timer,SIGNAL(timeout()),this, SLOT(updateNxtVac()));

    prcx = Settings::getSetting(projectionCX).toDouble();
    prcy = Settings::getSetting(projectionCY).toDouble();
    proj = new Projection (width(), height(),prcx,prcy);
    connect(proj,SIGNAL(newZoom(double)),this,SLOT(slotNewZoom(double)));

    scale = Settings::getSetting(projectionScale).toDouble();
    proj->setScale(scale);
    QDesktopWidget * desktopWidget = QApplication::desktop ();
    QRect screenRect = desktopWidget->screenGeometry(desktopWidget->primaryScreen());
    if(Settings::getSetting(saveMainWindowGeometry).toInt())
    {
        QSize savedSize = Settings::getSetting(mainWindowSize).toSize();

        if(savedSize.height()>screenRect.height() || savedSize.width() > screenRect.width())
        {
            move(QPoint(0,0));
            showMaximized();
        }
        else
        {
            resize( Settings::getSetting(mainWindowSize).toSize() );
            move  ( Settings::getSetting(mainWindowPos).toPoint() );
            if(Settings::getSetting(mainWindowMaximized).toInt()==1)
                showMaximized();
        }
    }
    else
        showMaximized ();
    QString ver="qtVlm "+QString().setNum(sizeof(int*)*8)+" bits "+Version::getVersion();

    ver+=get_OSVersion();

    Settings::setSetting(qtVlm_version,ver);
}
void MainWindow::continueSetup()
{
    this->show();
    this->activateWindow();

    progress=new Progress(this);
    progress->move(this->window()->frameGeometry().topLeft() +
        this->window()->rect().center() -
        progress->rect().center());


    QFile testWrite;
    testWrite.setFileName("testWrite.txt");
    if(testWrite.open(QIODevice::WriteOnly | QIODevice::Text ))
        testWrite.remove();
    else
    {
        QMessageBox::critical (this,
           "Error",
           "Unable to write in qtVlm folder<br>"+QFileInfo(testWrite).absoluteFilePath()+"<br>Please change the folder permissions or<br>try to reinstall qtVlm elsewhere");  /*pas traduit expres*/
        my_centralWidget->setAboutToQuit();
        QApplication::quit();
    }
    if(!QFile(appFolder.value("img")+"skin_compas.png").exists())
        QMessageBox::critical (this,
           "Error","File 'skin_compas.png' cannot be find in img directory<br>Please check your installation"); /*untranslated on purpose*/

//--------------------------------------------------
    progress->newStep(5,tr("Initializing menus"));
    menuBar = new MenuBar(this);
    setMenuBar(menuBar);

//--------------------------------------------------
    progress->newStep(10,tr("Initializing maps drawing"));
    my_centralWidget = new myCentralWidget(proj,this,menuBar);
    this->setCentralWidget(my_centralWidget);

//--------------------------------------------------
    progress->newStep(15,tr("Initializing status bar"));
    statusBar =new StatusBar(this);

//--------------------------------------------------
    progress->newStep(20,tr("Initializing tool bar"));

    toolBar = new ToolBar(this);
    my_centralWidget->set_toolBar(toolBar);

    Util::setFontObject(menuBar);

    //--------------------------------------------------
    progress->newStep(25,tr("Creating context menus"));
    menuPopupBtRight = menuBar->createPopupBtRight(this);
    connect(this->menuPopupBtRight,SIGNAL(triggered(QAction*)),this,SLOT(slot_disablePopupMenu()));
    connect(this->menuPopupBtRight,SIGNAL(aboutToShow()),my_centralWidget,SLOT(slot_resetGestures()));
    connect(this->menuPopupBtRight,SIGNAL(aboutToHide()),my_centralWidget,SLOT(slot_resetGestures()));

//--------------------------------------------------
    progress->newStep(26,tr("Loading polars list"));
    polar_list = new polarList(my_centralWidget->getInet(),this);

//--------------------------------------------------
    progress->newStep(28,tr("Reading boats data"));
    my_centralWidget->loadBoat();

//--------------------------------------------------
    progress->newStep(30,tr("Creating board & dialogs"));
    selPOI_instruction=NULL;
    isSelectingWP=false;
    myBoard=NULL;
    boardPlugin=NULL;
    loadBoard();

    /* restore state */
    restoreState(Settings::getSetting(savedState).toByteArray());
    toolBar->load_settings();



    connect(toolBar,SIGNAL(estimeParamChanged()),this,SIGNAL(paramVLMChanged()));

#ifdef __MAC_QTVLM
    progress->raise();
#endif
    progress->activateWindow();

    pilototo = new DialogPilototo(this,my_centralWidget,my_centralWidget->getInet());


    connectSignals();

//---------------------------------------------------------
    progress->newStep(35,tr("Preparing coffee"));

     //--------------------------------------------------
    // get screen geometry

    /* init du dialog de validation de grib (present uniquement en mode debug)*/
#ifdef __QTVLM_WITH_TEST
    gribValidation_dialog = new DialogGribValidation(my_centralWidget,this);
#endif


#ifdef __REAL_BOAT_ONLY
    if(players.count()==0) {
        Player * newPlayer = new Player("MyBoat","",BOAT_REAL,0,"MyBoat",proj,this,my_centralWidget,my_centralWidget->getInet());
        my_centralWidget->slot_addPlayer_list(newPlayer);
        players=my_centralWidget->getPlayers();
    }

#endif

    QList<Player*> players=my_centralWidget->getPlayers();
    if(players.count()==1)
    {
        if(use_old_board)
            myBoard->playerChanged(players.at(0));
        if(players.at(0)->getType()==BOAT_VLM)
        {
            progress->newStep(50,tr("Updating player"));
            connect(players.at(0),SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updPlayerFinished(bool,Player*)));
            players.at(0)->updateData();
        }
        else
        {
            my_centralWidget->slot_playerSelected(players.at(0));
            my_centralWidget->loadPOI();
            isStartingUp=false;
            updateTitle();
            closeProgress();
            my_centralWidget->emitUpdateRoute(NULL);
        }
        Util::setFontObject(menuBar);
        return;
    }
//********************************************
    progress->newStep(55,tr("Calling player dialog"));
    bool res;
    my_centralWidget->manageAccount(&res);
    if(!res)
    {
        /* too bad we are exiting already */
        qWarning() << "let's quit - bye";
        finishStart=false;
    }
    else
    {
        if(my_centralWidget->getPlayer() && my_centralWidget->getPlayer()->getType()==BOAT_VLM)
        {
            if(!my_centralWidget->getBoats())
            {
                qWarning() << "CRITICAL: mainWin init - empty boatList";
                finishStart=false;
            }
            else
            {
                my_centralWidget->loadPOI();
                nBoat=my_centralWidget->getBoats()->size();
                toBeCentered=-1;
                if(nBoat>0)
                {
 //************************************************************
                    progress->newStep(60,tr("Updating boats"));
                    VLM_Sync_sync();
                    return;
                }
                else
                {
                    isStartingUp=false;
                    updateTitle();
                    my_centralWidget->emitUpdateRoute(NULL);
                }

            }
        }
        else
        {
            isStartingUp=false;
            my_centralWidget->loadPOI();
            updateTitle();
            my_centralWidget->emitUpdateRoute(NULL);
        }
    }

    closeProgress();
    Util::setFontObject(menuBar);
}

QString MainWindow::get_OSVersion(void) {
    QString ver="";
#ifdef __UNIX_QTVLM
    ver=" (UNIX)";
#ifdef __ANDROID__
    ver=" (ANDROID)";
#endif
#endif
#ifdef __WIN_QTVLM
    QString OS_versionName;
    int windowsVersion=QSysInfo::windowsVersion();
    switch(windowsVersion)
    {
        case QSysInfo::WV_32s:
            OS_versionName="Windows 3.1 with Win 32s";
            break;
        case QSysInfo::WV_95:
            OS_versionName="Windows 95";
            break;
        case QSysInfo::WV_98:
            OS_versionName="Windows 98";
            break;
        case QSysInfo::WV_Me:
            OS_versionName="Windows Me";
            break;
        case QSysInfo::WV_NT:
            OS_versionName="Windows NT";
            break;
        case QSysInfo::WV_2000:
            OS_versionName="Windows 2000";
            break;
        case QSysInfo::WV_XP:
            OS_versionName="Windows XP";
            break;
        case QSysInfo::WV_2003:
            OS_versionName="Windows Server 2003";
            break;
        case QSysInfo::WV_VISTA:
            OS_versionName="Windows VISTA";
            break;
        case QSysInfo::WV_WINDOWS7:
            OS_versionName="Windows 7";
            break;
        case QSysInfo::WV_WINDOWS8:
            OS_versionName="Windows 8";
            break;
        case QSysInfo::WV_CE:
            OS_versionName="Windows CE";
            break;
        case QSysInfo::WV_CENET:
            OS_versionName="Windows CE .NET";
            break;
        case QSysInfo::WV_CE_5:
            OS_versionName="Windows CE 5.x";
            break;
        case QSysInfo::WV_CE_6:
            OS_versionName="Windows CE 6.x";
            break;
        default:
            OS_versionName="Unknown Windows version";
            break;
    }
    qWarning()<<"windows version:"<<OS_versionName;
    ver=" ("+OS_versionName+")";
#endif
#ifdef __MAC_QTVLM
    QString OS_versionName;
    int windowsVersion=QSysInfo::macVersion();
    switch(windowsVersion)
    {
        case QSysInfo::MV_9:
            OS_versionName="Mac OS 9 (unsupported)";
            break;
        case QSysInfo::MV_10_0:
            OS_versionName="Mac OS X 10.0-CHEETAH (unsupported)";
            break;
        case QSysInfo::MV_10_1:
            OS_versionName="Mac OS X 10.1-PUMA (unsupported)";
            break;
        case QSysInfo::MV_10_2:
            OS_versionName="Mac OS X 10.2-JAGUAR (unsupported)";
            break;
        case QSysInfo::MV_10_3:
            OS_versionName="Mac OS X 10.3-PANTHER (unsupported)";
            break;
        case QSysInfo::MV_10_4:
            OS_versionName="Mac OS X 10.4-TIGER (unsupported)";
            break;
        case QSysInfo::MV_10_5:
            OS_versionName="Mac OS X 10.5-LEOPARD (unsupported)";
            break;
        case QSysInfo::MV_10_6:
            OS_versionName="Mac OS X 10.6-SNOWLEOPARD";
            break;
        case QSysInfo::MV_10_7:
            OS_versionName="Mac OS X 10.7-LION";
            break;
        case QSysInfo::MV_10_8:
            OS_versionName="Mac OS X 10.8-MONTAINLION";
            break;
        case QSysInfo::MV_10_9:
            OS_versionName="Mac OS X 10.9-MAVERICKS";
            break;
        default:
            OS_versionName="Unknown Macintosh version";
            break;
    }
    qWarning()<<"mac version:"<<OS_versionName;
    ver=" ("+OS_versionName+")";
#endif
    return ver;
}

void MainWindow::loadBoard()
{
    qWarning()<<"loading board";
    use_old_board=true;
    if(my_centralWidget->getPlayer() && my_centralWidget->getPlayer()->getType()==BOAT_VLM && Settings::getSetting(vlmBoardType).toString()!="0")
        use_old_board=false;
    if(use_old_board && myBoard)
    {
        myBoard->playerChanged(my_centralWidget->getPlayer());
        return;
    }
// TODO: do not reload same plugin
#if 0
    if(!use_old_board && boardPlugin)
    {
        if(boardPlugin->getUiFileName()==Settings::getSetting(vlmBoardType).toString()) return;
    }
#endif
    if(boardPlugin)
    {
        delete boardPlugin;
    }
    if(myBoard){
        delete myBoard;
    }
    boardPlugin=NULL;
    myBoard=NULL;
    if(use_old_board)
    {
        myBoard = new board(this,my_centralWidget->getInet());
        //connect(menuBar->acOptions_SH_ComBandeau,SIGNAL(triggered()),myBoard,SLOT(slot_hideShowCompass()));
        connect(this,SIGNAL(wpChanged()),myBoard->VLMBoard(),SLOT(update_btnWP()));
        // get screen geometry
        QDesktopWidget * desktopWidget = QApplication::desktop ();

        QRect screenRect = desktopWidget->screenGeometry(desktopWidget->primaryScreen());

        if(screenRect.height()<=600)
        {
            /* small screen height */
            qWarning() << "Small screen => no compas and floating panel";
            myBoard->VLMBoard()->setCompassVisible(false);
            myBoard->realBoard()->setCompassVisible(false);
            myBoard->floatingBoard(true);
        }
        else
        {
            myBoard->floatingBoard(false);
        }
        myBoard->playerChanged(my_centralWidget->getPlayer());
    }
    else
    {
        QFile       uiFile (Settings::getSetting(vlmBoardType).toString());
        // Load the associated resources if they exist
        QFileInfo   uiFileInfo (uiFile);
        QString     rccFileName = uiFileInfo.dir().filePath (uiFileInfo.completeBaseName() + ".rcc");
        if (!QResource::registerResource (rccFileName))
           qWarning() << "Could not load resources associated with the board" << rccFileName;
        // Load the .ui file
        uiFile.open (QFile::ReadOnly);
        QUiLoader   loader;
        QWidget* w = loader.load (&uiFile, NULL);
        boardPlugin = qobject_cast<BoardInterface*> (w);
        uiFile.close();
        if (boardPlugin != NULL) {
           boardPlugin->initBoard(this);
        } else {
           if (w != NULL) delete w;
           qWarning() << "Error loading board definition"
                      << Settings::getSetting(vlmBoardType).toString();
#ifdef QT_V5
           qWarning() << loader.errorString();
#endif
           Settings::setSetting(vlmBoardType,"0");
           loadBoard();
        }
    }
    this->showDashBoard();
}
QColor MainWindow::getWindColorStatic(const double &v, const bool &smooth)
{
    return MapDataDrawer::getWindColorStatic(v,smooth);
}
//-----------------------------------------------
void MainWindow::listAllChildren(QObject * ptr,int depth=0)
{
    if(!ptr) ptr=this;
    QObjectList childList=ptr->children();
    if(childList.count()!=0)
    {
        for(int i=0;i<childList.count();++i)
            listAllChildren(childList[i],depth+1);
    }
    //qWarning() << ptr << " (" << ptr->x() << "," << ptr->y() << ") " << ptr->objectName();
    if(ptr->isWidgetType())
    {
        QWidget * ptrWidget = (QWidget*) ptr;
        qWarning() << QString().fill(QChar(' '),depth*2) << ptr << " (" << ptrWidget->x() << "," << ptrWidget->y() << ") visible: " << ptrWidget->isVisible();
    }
    else
        qWarning() << QString().fill(QChar(' '),depth*2) << ptr ;
}

//-----------------------------------------------
MainWindow::~MainWindow()
{
    //--------------------------------------------------
    // Save global settings
    //--------------------------------------------------
    //qWarning()<<"mainwindow destructor";
    my_centralWidget->setAboutToQuit();
    if(noSave) return;
    if(Settings::getSetting(saveMainWindowGeometry).toInt())
    {
        Settings::setSetting(mainWindowSize, size());
        Settings::setSetting(mainWindowPos, pos());
        Settings::setSetting(mainWindowMaximized,this->isMaximized()?"1":"0");
    }

    toolBar->save_settings();
    Settings::setSetting(savedState,saveState());
    //qWarning() << "State saved: " <<  Settings::getSetting(savedState).toByteArray();

    Settings::setSetting(projectionCX, proj->getCX());
    Settings::setSetting(projectionCY, proj->getCY());
    Settings::setSetting(projectionScale, proj->getScale());

    /*freeze all routes*/
    if(selectedBoat) /* save the zoom factor */
        selectedBoat->setZoom(proj->getScale());
    delete toolBar;
}

QMenu * MainWindow::createPopupMenu(void) {

    QMenu * menu = new QMenu;
    Util::setFontObject(menu);
    int entry=0;

    if(use_old_board)
    {
        if(myBoard)
            entry=myBoard->build_showHideMenu(menu);
    }
    if(entry)
        menu->addSeparator();

    if(toolBar)
        entry+=toolBar->build_showHideMenu(menu);


    if(entry)
        return menu;
    else {
        delete menu;
        return NULL;
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/) {

    QApplication::quit();
}

void MainWindow::keyPressEvent ( QKeyEvent  * /* event */ )
{
    //qWarning() << "Key pressed in main: " << event->key();
}

void MainWindow::closeProgress(void)
{
    bool XP=false;
#ifdef __WIN_QTVLM
    if (QSysInfo::windowsVersion()==QSysInfo::WV_XP)
        XP=true;
#endif
    if(!XP && QThread::idealThreadCount()>1 && QFile(appFolder.value("img")+"benchmark.grb").exists())
    {
        progress->newStep(80,tr("Calibrating grib display"));
        QApplication::processEvents();
        DataManager * dataManager=my_centralWidget->get_dataManager();
        MapDataDrawer * mapDataDrawer=my_centralWidget->get_mapDataDrawer();
        if(dataManager && dataManager->load_data(appFolder.value("img")+"benchmark.grb",DataManager::GRIB_GRIB)
                && dataManager->isOk(DataManager::GRIB_GRIB) && mapDataDrawer)
        {
            dataManager->set_currentDate(dataManager->get_minDate()+3650); //not to be on a gribrecord;
            proj->blockSignals(true);
            double xW=proj->getXmin();
            double xE=proj->getXmax();
            double yS=proj->getYmin();
            double yN=proj->getYmax();
            my_centralWidget->zoomOnGrib(DataManager::GRIB_GRIB);
            QPixmap * imgAll = new QPixmap(my_centralWidget->get_terrain()->getSize());
            imgAll->fill(Qt::transparent);
            QPainter pnt(imgAll);
            pnt.setRenderHint(QPainter::Antialiasing, true);
            pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);
            QTime calibration;
            calibration.start();
            mapDataDrawer->drawTest_mono(pnt,proj);
            int cal2=calibration.elapsed();
            pnt.end();
            //imgAll->save("calib1.jpg");
            imgAll->fill(Qt::transparent);
            pnt.begin(imgAll);
            pnt.setRenderHint(QPainter::Antialiasing, true);
            pnt.setRenderHint(QPainter::SmoothPixmapTransform, true);
            calibration.start();
            mapDataDrawer->drawTest_multi(pnt,proj);
            int cal1=calibration.elapsed();
            pnt.end();
            //imgAll->save("calib2.jpg");
            delete imgAll;
            proj->zoomOnZone(xW,yN, xE,yS);
            proj->blockSignals(false);
            qWarning()<<"result of benchmark: multiThread="<<cal1<<"monoThread="<<cal2;
            Settings::setSetting(gribBench1,cal1);
            Settings::setSetting(gribBench2,cal2);

            /** **/
            dataManager->close_data(DataManager::GRIB_GRIB);
        }

    }
    else
    {
        Settings::setSetting(gribBench1,10);
        Settings::setSetting(gribBench2,0);
    }
    progress->newStep(90,tr("Opening grib"));
    gribFilePath = Settings::getSetting(gribFile_Path).toString();
    if(gribFilePath.isEmpty())
        gribFilePath = appFolder.value("grib");
    QString fname = Settings::getSetting(grib_FileName).toString();
    if (fname != "" && QFile::exists(fname))
    {
        openGribFile(fname, false);
        gribFileName=fname;
    }
    fname = Settings::getSetting(gribCurrent_FileName).toString();
    if (fname != "" && QFile::exists(fname))
    {
        openGribFile(fname, false,true);
        gribFileNameCurrent=fname;
    }
    slot_updateGribMono();
    progress->close();\
    if(!Settings::getSetting(LastKap).toString().isEmpty())
    {
        progress->newStep(95,tr("Opening kap"));
        my_centralWidget->imgKap_open(Settings::getSetting(LastKap).toString());
    }
//    QPluginLoader plugin("pluginExamplePlugin");
//    plugin.load();
//    if(!plugin.isLoaded())
//        qWarning()<<"error loading plugin"<<plugin.errorString();
//    else
//    {
//        PluginExampleInterface * myPlugin=qobject_cast<PluginExampleInterface *>(plugin.instance());
//        myPlugin->initPluginExample(this);
//    }
    delete progress;
    progress=NULL;

    if(selectedBoat)
    {
        selectedBoat->cleanBarrierList();
        if(selectedBoat->get_boatType()==BOAT_REAL)
        {                        
            proj->setScaleAndCenterInMap(selectedBoat->getZoom(),selectedBoat->getLon(),selectedBoat->getLat());
            if(Settings::getSetting(polar_efficiency).toInt()!=100)
            {
                selectedBoat->reloadPolar(true);
                emit boatHasUpdated(selectedBoat);
            }
            QDateTime dtm =QDateTime::fromTime_t(((boatReal*)selectedBoat)->getEta()).toUTC();
            if(!dtm.isValid() || ((boatReal*)selectedBoat)->getEta()<=0)
                statusBar->clear_eta();
            else
                statusBar->update_eta(dtm);
        }
        else
        {
            updatePilototo_Btn((boatVLM*)selectedBoat);
            this->slotUpdateOpponent();
            nxtVac_cnt=((boatVLM*)selectedBoat)->getNextVac();
            emit drawVacInfo();
            QDateTime dtm =QDateTime::fromString(((boatVLM*)selectedBoat)->getETA(),"yyyy-MM-ddTHH:mm:ssZ");
            if(!dtm.isValid())
                statusBar->clear_eta();
            else
                statusBar->update_eta(dtm);
            timer->start(1000);
        }
        if(use_old_board)
            myBoard->boatUpdated(selectedBoat);
        emit WPChanged(selectedBoat->getWPLat(),selectedBoat->getWPLon());
        emit boatChanged(selectedBoat);
        emit paramVLMChanged();
    }
    statusBar->show();
    menuBar->show();
    this->showDashBoard();
//    QQuickView *qml = new QQuickView;
//    qml->setSource(QUrl::fromLocalFile("src/qml_ui.qml"));
//    qml->show();

#ifdef __ANDROID__
    //menuBar->setNativeMenuBar(true);
    menuBar->hide();
#endif
}

//-------------------------------------------------
void MainWindow::openGribFile(QString fileName, bool zoom, bool current)
{
    bool badFile=false;
    bool badCurrent=false;

    DataManager * dataManager = my_centralWidget->get_dataManager();
    if(!dataManager) {
        qWarning() << "No data manager present !!";
        return;
    }

    if(current) /*if current tries to open it as a current file */
    {
        my_centralWidget->loadGribFileCurrent(fileName, zoom);

        if(!dataManager->isOk(DataManager::GRIB_CURRENT))
            badFile=true;
        else if(!dataManager->hasData(DATA_CURRENT_VX,DATA_LV_MSL,0,DataManager::GRIB_CURRENT))
        {
            slotFile_Close_Current();            
            badCurrent=true;
        }
    }
    else
    {
        my_centralWidget->loadGribFile(fileName, zoom);
        if(!dataManager->isOk(DataManager::GRIB_GRIB))
            badFile=false;
    }

    if (!badFile && !badCurrent)
    {
        slot_updateGribMono();        
        slotDateGribChanged_now();

        if(!current)
        {
            Settings::setSetting(grib_FileName,  fileName);
            Settings::setSetting(gribFile_Path,  gribFilePath);
        }
        else
        {
            Settings::setSetting(gribCurrent_FileName,  fileName);
            Settings::setSetting(gribFile_Path,  gribFilePath);
        }
    }
    else if (!badCurrent)
    {
        QMessageBox::critical (this,
            tr("Erreur"),
            tr("Fichier : ") + fileName + "\n\n"
                + tr("Echec lors de l'ouverture.") + "\n\n"
                + tr("Le fichier ne peut pas etre ouvert,") + "\n"
                + tr("ou ce n'est pas un fichier GRIB,") + "\n"
                + tr("ou le fichier est corrompu,") + "\n"
                + tr("ou il contient des donnees non reconnues,") + "\n"
                               + tr("ou..."));
    }
    else
    {
        QMessageBox::critical (this,
            tr("Erreur"),
            tr("Fichier : ") + fileName + "\n\n"
                + tr("Echec lors de l'ouverture.") + "\n\n"
                + tr("Ce fichier ne contient pas") + "\n"
                + tr("de donnees Courants"));
    }

    toolBar->update_gribBtn();

    updateTitle();
}

void MainWindow::updateTitle()
{
    QString ver="qtVlm "+QString().setNum(sizeof(int*)*8)+" bits "+Version::getVersion();

    if(isStartingUp) {
        setWindowTitle(ver);
        return;
    }

    DataManager * dataManager = my_centralWidget->get_dataManager();
    if(!dataManager) {
        setWindowTitle(ver);
        return;
    }

    QString dateStr="";

    if(dataManager->isOk()) {
        QDateTime startGribDate=QDateTime().fromTime_t(dataManager->get_minDate()).toUTC();
        startGribDate.setTimeSpec(Qt::UTC);
        QDateTime endGribDate=QDateTime().fromTime_t(dataManager->get_maxDate()).toUTC();
        endGribDate.setTimeSpec(Qt::UTC);
        dateStr = tr(" (du ")+
                startGribDate.toString(tr("dd/MM/yyyy hh:mm"))+tr(" au ")+
                endGribDate.toString(tr("dd/MM/yyyy hh:mm"))+")";
    }

    QString gribStr="",currentStr="";

    if(dataManager->isOk(DataManager::GRIB_GRIB)) {
        QFileInfo i(dataManager->get_fileName(DataManager::GRIB_GRIB));
        gribStr = tr(" grib: ")+ i.fileName();
    }

    if(dataManager->isOk(DataManager::GRIB_CURRENT)) {
        QFileInfo i(dataManager->get_fileName(DataManager::GRIB_CURRENT));
        currentStr = tr(" courant: ")+ i.fileName();
    }
    setWindowTitle(ver+gribStr+currentStr+dateStr);
}

//-------------------------------------------------
// SLOTS
//-------------------------------------------------

void MainWindow::slotUpdateOpponent(void)
{
    bool found=false;
    if(!selectedBoat || selectedBoat->get_boatType()!=BOAT_VLM)
    {
        my_centralWidget->getOppList()->clear();
        return;
    }

    for(int i=0;i<my_centralWidget->getRaces().size();++i)
    {
        if(my_centralWidget->getRaces()[i]->idrace ==  ((boatVLM *)selectedBoat)->getRaceId())
        {
            //qWarning() << "Set1";
            my_centralWidget->getOppList()->setBoatList(my_centralWidget->getRaces()[i]->oppList,my_centralWidget->getRaces()[i]->idrace,my_centralWidget->getRaces()[i]->showWhat,true,my_centralWidget->getRaces()[i]->showReal,my_centralWidget->getRaces()[i]->realFilter);
            my_centralWidget->drawNSZ(i);
            found=true;
            break;
        }
    }
    if(!found)
    {
        my_centralWidget->getOppList()->clear();
        my_centralWidget->drawNSZ(-1);
    }
}

void MainWindow::slotCreatePOI()
{
    this->slot_showPOI_input();
}

void MainWindow::slot_newPOI(void)
{
    this->slot_showPOI_input(NULL,true);
}

void MainWindow::slot_removePOI(void) {
    my_centralWidget->removePOI();
}
void MainWindow::slot_removeRoute(void) {
    my_centralWidget->removeRoute();
}

void MainWindow::slot_centerSelectedBoat()
{
    if(selectedBoat)
        selectedBoat->slot_centerOnBoat();
}


void MainWindow::slot_moveBoat(void)
{
    double lon, lat;
    proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
    emit moveBoat(lat,lon);
}

//-------------------------------------------------
void MainWindow::slotOptions_Language()
{
    QString lang;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acOptions_GroupLanguage->checkedAction();
    if (act == mb->acOptions_Lang_fr) {
        lang = "fr";
        Settings::setSetting(appLanguage, lang);
        QMessageBox::information (this,
            QString("Changement de langue"),
            QString("Langue : Français\n\n")
              + QString("Les modifications seront prises en compte\n")
              + QString("au prochain lancement du programme.")
        );
    }
    else if (act == mb->acOptions_Lang_en) {
        lang = "en";
        Settings::setSetting(appLanguage, lang);
        QMessageBox::information (this,
            QString("Application Language"),
            QString("Language : English\n\n")
              + QString("Please reload application to activate language.\n")
        );
    }
    else if (act == mb->acOptions_Lang_cz) {
        lang = "cz";
        Settings::setSetting(appLanguage, lang);
        QMessageBox::information (this,
            QString("Application Language"),
            QString("Language : Czech\n\n")
              + QString("Please reload application to activate language.\n")
        );
    }
    else if (act == mb->acOptions_Lang_es) {
        lang = "es";
        Settings::setSetting(appLanguage, lang);
        QMessageBox::information (this,
            QString("Application Language"),
            QString("Language : Spanish\n\n")
              + QString("Please reload application to activate language.\n")
        );
    }
}

//-------------------------------------------------
void MainWindow::slotHelp_Help() {
    QDesktopServices::openUrl(QUrl("http://wiki.v-l-m.org/index.php/QtVlm#L.27interface_de_qtVlm"));
}
void MainWindow::slotHelp_Forum() {
    QDesktopServices::openUrl(QUrl("http://www.virtual-winds.org/forum/index.php?showtopic=6638&view=getnewpost"));
}

//-------------------------------------------------
void MainWindow::slotHelp_APropos() {
    QMessageBox::about (this,
            tr("A propos"),
            tr("qtVlm : GUI pour Virtual loup de mer")
            +"\nhttp://www.v-l-m.org\n"+

            tr("Version : ")+Version::getVersion()
                    +"      "+Version::getDate()
            +"\n"+ tr("Licence : GNU GPL v3")
            +"\n"+ "http://qtvlm.sf.net"
            +"\n"+ "http://virtual-winds.org/forum"
            +"\n"+ tr("Grib part is originaly from zygrib project")
            +"\n"+ "http://www.zygrib.org"
        );
}
//-------------------------------------------------
void MainWindow::slotHelp_AProposQT() {
    QMessageBox::aboutQt (this);
}

//-------------------------------------------------
void MainWindow::slotFile_Lock(bool readOnly)
{
    if(!selectedBoat) return;

    if(!readOnly)
        selectedBoat->setLockStatus(!selectedBoat->getLockStatus());

    QString ic;
    if(selectedBoat->getLockStatus())
        ic=QString(appFolder.value("img")+"lock.png");
    else
        ic=QString(appFolder.value("img")+"unlock.png");

    emit updateLockIcon(ic);
}

void MainWindow::slotFile_Quit() {
    qWarning()<<"slotFile_Quit called";
    noSave=false;
    my_centralWidget->set_noSave(false);
    my_centralWidget->setAboutToQuit();
    QApplication::quit();
}

void MainWindow::slotFile_QuitNoSave() {
    qWarning()<<"slotFile_QuitNoSave called";
    noSave=true;
    my_centralWidget->set_noSave(true);
    my_centralWidget->setAboutToQuit();
    QApplication::quit();
}

using namespace std;
void MainWindow::slotCombineGrib() {

    QDir dirGrib(gribFilePath);
    if(!dirGrib.exists())
    {
        gribFilePath=appFolder.value("grib");
        Settings::setSetting(askGribFolder,1);
        Settings::setSetting(edtGribFolder,gribFilePath);
    }
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                tr("Select several Grib files to combine"),
                gribFilePath,
                tr("GRIB file (*.grb *.grib *.grb2 *.grib2)"));
    if(files.size()>1) {
        files.sort();
        QString str=QString().setNum(files.size()) + " " + tr("gribs to combine");
        QMessageBox::information(this,tr("Grib combination"),str,QMessageBox::Ok);
        /* select output file */
        QString filename = QFileDialog::getSaveFileName(this,
                           tr("Filename of destination file"), "", tr("GRIB file (*.grb *.grib *.grb2 *.grib2)"));
        if (filename != "") {
            ofstream fdest;
            fdest.open(filename.toUtf8().constData(),ios::out | ios::trunc | ios::binary);
            if(!fdest.is_open()) return;
            int added=0;
            for(int i=0;i<files.count();++i) {
                ifstream fread;
                fread.open(files.at(i).toUtf8().constData(),ios::in | ios::binary);
                if(!fdest.is_open()) continue;
                fdest << fread.rdbuf();

                /*if(!fread.eof()) {
                    qWarning() << i << ": incomplete read";
                    fread.close();
                    continue;
                }*/
                fread.close();
                if(!fdest.good()) {
                    qWarning() << i << ": dest not good";
                    fdest.close();
                    break;
                }
                added++;

            }
            fdest.close();
            str=QString().setNum(added) +" " + tr("files successfully combined")+"\n"+tr("Open generated file ?");
            if(QMessageBox::question(this,tr("Grib combination"),str)==QMessageBox::Yes) {
                bool zoom =  (Settings::getSetting(gribZoomOnLoad).toInt()==1);
                openGribFile(filename, zoom);
                if(my_centralWidget) my_centralWidget->fileInfo_GRIB(DataManager::GRIB_GRIB);
            }
        }

    }
    else
        QMessageBox::information(this,tr("Grib combination"),tr("Not enough file to combine"),QMessageBox::Ok);
}

//-------------------------------------------------

void MainWindow::slotFile_Open()
{
    QString filter;
    filter =  tr("Fichiers GRIB (*.grb *.grib *.grb.bz2 *.grib.bz2 *.grb.gz *.grib.gz *.grb2 *.grib2)")
            + tr(";;Autres fichiers (*)");
    QDir dirGrib(gribFilePath);
    if(!dirGrib.exists())
    {
        gribFilePath=appFolder.value("grib");
        Settings::setSetting(askGribFolder,1);
        Settings::setSetting(edtGribFolder,gribFilePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier GRIB"),
                         gribFilePath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        gribFilePath = finfo.absolutePath();
        bool zoom =  (Settings::getSetting(gribZoomOnLoad).toInt()==1);
        openGribFile(fileName, zoom);
        //my_centralWidget->get_terrain()->update_mapDataAndLevel();
        if(my_centralWidget) my_centralWidget->fileInfo_GRIB(DataManager::GRIB_GRIB);
    }
    updateTitle();
}
void MainWindow::slotFile_Reopen()
{
   if(!my_centralWidget->get_dataManager()->get_grib(DataManager::GRIB_GRIB)) return;
   openGribFile (my_centralWidget->get_dataManager()->get_grib(DataManager::GRIB_GRIB)->get_fileName(), (Settings::getSetting(gribZoomOnLoad).toInt() == 1));
   //my_centralWidget->get_terrain()->update_mapDataAndLevel();
   updateTitle();
}
void MainWindow::slotFile_Open_Current()
{
    QString filter;
    filter =  tr("Fichiers GRIB (*.grb *.grib *.grb.bz2 *.grib.bz2 *.grb.gz *.grib.gz *.grb2 *.grib2)")
            + tr(";;Autres fichiers (*)");
    QDir dirGrib(gribFilePath);
    if(!dirGrib.exists())
    {
        gribFilePath=appFolder.value("grib");
        Settings::setSetting(askGribFolder,1);
        Settings::setSetting(edtGribFolder,gribFilePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier GRIB"),
                         gribFilePath,
                         filter);
    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        gribFilePath = finfo.absolutePath();
        bool zoom =  (Settings::getSetting(gribZoomOnLoad).toInt()==1);
        openGribFile(fileName, zoom, true);
        //my_centralWidget->get_terrain()->update_mapDataAndLevel();
        if(my_centralWidget) my_centralWidget->fileInfo_GRIB(DataManager::GRIB_CURRENT);
    }
    updateTitle();
}
//-------------------------------------------------
void MainWindow::slotFile_Close_Current() {
    gribFileNameCurrent = "";
    my_centralWidget->closeGribFileCurrent();
    toolBar->update_gribBtn();
    updateTitle();
    my_centralWidget->get_terrain()->update_mapDataAndLevel();
    my_centralWidget->emitUpdateRoute(NULL);
}
//-------------------------------------------------
void MainWindow::slotFile_Close()
{
    gribFileName = "";
    my_centralWidget->closeGribFile();
    toolBar->update_gribBtn();
    updateTitle();
    my_centralWidget->get_terrain()->update_mapDataAndLevel();
    my_centralWidget->emitUpdateRoute(NULL);
}

//========================================================================
void MainWindow::slotDateStepChanged(int step)
{
    Settings::setSetting(gribDateStep,step);
    updatePrevNext();
}

void MainWindow::updatePrevNext(void)
{
    toolBar->update_gribBtn();
}

//-------------------------------------------------
void MainWindow::slotDateGribChanged_now(bool b)
{
    time_t tps=QDateTime::currentDateTime().toUTC().toTime_t();
    DataManager * dataManager=my_centralWidget->get_dataManager();
    if(dataManager && dataManager->isOk()) {
        time_t min=dataManager->get_minDate();
        time_t max=dataManager->get_maxDate();
        if(tps<min) tps=min;
        if(tps>max) tps=max;
        my_centralWidget->setCurrentDate( tps, b );
        updatePrevNext();
    }
}

void MainWindow::slotDateGribChanged_sel()
{
    my_centralWidget->choose_gribDate();
    updatePrevNext();
}


//-------------------------------------------------
void MainWindow::slotDateGribChanged_next()
{
    QApplication::processEvents();
    DataManager * dataManager=my_centralWidget->get_dataManager();
    if(dataManager && dataManager->isOk()) {
        time_t tps=dataManager->get_currentDate();
        time_t max=dataManager->get_maxDate();
        int step=toolBar->get_gribStep();
        if((tps+step)<=max)
        {
            my_centralWidget->setCurrentDate(tps+step);
            updatePrevNext();
        }
        else if(toolBar->isPlaying())
            toolBar->stopPlaying();
    }
}

//-------------------------------------------------
void MainWindow::slotDateGribChanged_prev() {
    DataManager * dataManager=my_centralWidget->get_dataManager();
    if(dataManager && dataManager->isOk()) {
        time_t tps=dataManager->get_currentDate();
        time_t min=dataManager->get_minDate();
        int step=toolBar->get_gribStep();
        if((tps-step)>=min)
            my_centralWidget->setCurrentDate(tps-step);
    }
    updatePrevNext();
}

void MainWindow::slotSetGribDate(time_t tps) {
    DataManager * dataManager=my_centralWidget->get_dataManager();
    if(dataManager && dataManager->isOk()) {
        time_t min=dataManager->get_minDate();
        time_t max=dataManager->get_maxDate();
        if(tps>=min && tps <=max)
            my_centralWidget->setCurrentDate(tps);
    }
}

//-------------------------------------------------

//-------------------------------------------------

void MainWindow::get_selectedBoatPos(double * lat,double* lon)
{
    if(lat)
        *lat=selectedBoat!=NULL?selectedBoat->getLat():-1;
    if(lon)
        *lon=selectedBoat!=NULL?selectedBoat->getLon():-1;
}
int MainWindow::get_selectedBoatVacLen()
{
    if(selectedBoat)
       return selectedBoat->getVacLen();
    else
        return 1;
}
void MainWindow::showDashBoard()
{
    bool shTdb=Settings::getSetting(show_DashBoard).toInt()==1;
    if(use_old_board)
    {
        if (myBoard)
            myBoard->showCurrentBoard(shTdb);
    }
    else
    {
        boardPlugin->setVisible(shTdb);
    }
    menuBar->acOptions_SH_Tdb->setChecked(shTdb);
}

void MainWindow::updatePilototo_Btn(boatVLM * boat)
{
    if(!selPOI_instruction)
    {
        emit selectPOI(false);
        /* compute nb Pilototo instructions */
        QStringList lst = boat->getPilototo();
        QString pilototo_txt=tr("Pilototo");
        QString pilototo_toolTip="";
        if(use_old_board)
            myBoard->VLMBoard()->set_style(myBoard->VLMBoard()->btn_Pilototo,QColor(255, 255, 127));
        if(boat->getHasPilototo())
        {
            int nbPending=0;
            int nb=0;
            for(int i=0;i<lst.count();++i)
                if(lst.at(i)!="none")
                {
                QStringList instr_buf = lst.at(i).split(",");
                int mode=instr_buf.at(2).toInt()-1;
                int pos =5;
                if(mode == 0 || mode == 1)
                    pos=4;
                if(instr_buf.at(pos) == "pending")
                    ++nbPending;
                ++nb;
            }
            if(nb!=0)
            {
                pilototo_txt=pilototo_txt+" ("+QString().setNum(nbPending)+"/"+QString().setNum(nb)+")";
                if(use_old_board)
                {
                    if(nbPending!=0)
                    {
                        myBoard->VLMBoard()->set_style(myBoard->VLMBoard()->btn_Pilototo,QColor(14,184,63),QColor(255, 255, 127));
                    }
                }
            }
        }
        else
        {

            pilototo_toolTip=tr("Imp. de lire le pilototo de VLM");
            pilototo_txt=pilototo_txt+" (!)";
        }
        menuBar->acPilototo->setText(pilototo_txt);
        menuBar->acPilototo->setToolTip(pilototo_toolTip);
        if(use_old_board)
        {
            myBoard->VLMBoard()->btn_Pilototo->setText(pilototo_txt);
            myBoard->VLMBoard()->btn_Pilototo->setToolTip(pilototo_toolTip);
        }
    }
    else
    {
        emit selectPOI(true);
        menuBar->acPilototo->setText(tr("Selection d'une marque"));
        if(use_old_board)
            myBoard->VLMBoard()->btn_Pilototo->setText(tr("Selection d'une marque"));
    }
}

void MainWindow::updateNxtVac(void)
{
    if(!selectedBoat || selectedBoat->get_boatType()!=BOAT_VLM)
    {
        nxtVac_cnt=0;
    }
    else
    {
        boatVLM * b=(boatVLM *)selectedBoat;
        nxtVac_cnt=lastUpdateTime+b->getNextVac()-QDateTime::currentDateTimeUtc().toTime_t();
        //qWarning()<<nxtVac_cnt<<b->getPrevVac()<<b->getNextVac()<<QDateTime::currentDateTimeUtc().toTime_t();
        if(nxtVac_cnt<0)
        {
            nxtVac_cnt=b->getVacLen()+(nxtVac_cnt%b->getVacLen());
            emit outDatedVlmData();
            if(use_old_board)
                myBoard->outdatedVLM();
        }
    }
    emit drawVacInfo();
}

QList<POI*> * MainWindow::getPois()
{
    return my_centralWidget->getPoisList();
}

void MainWindow::slotShowContextualMenu(QGraphicsSceneContextMenuEvent * e)
{
    showContextualMenu(e->scenePos().x(),e->scenePos().y());
}
void MainWindow::showContextualMenu(const int &xPos, const int &yPos, QPoint screenPos)
{
    if(screenPos==QPoint(0,0))
    {
        screenPos=QCursor::pos();
    }
    mouseClicX = xPos;
    mouseClicY = yPos;

    int compassMode = my_centralWidget->getCompassMode(mouseClicX,mouseClicY);
    my_centralWidget->set_cursorPositionOnPopup(QPoint(mouseClicX,mouseClicY));

    /*** Barrier ***/

    menuBar->ac_popupBarrier->setEnabled(true);
    if(my_centralWidget->get_barrierEditMode()==BARRIER_EDIT_NO_EDIT)
        menuBar->ac_popupBarrier->setText(tr("Create new barrier"));
    else
        menuBar->ac_popupBarrier->setText(tr("Stop barrier create"));


    switch(compassMode)
    {
        case COMPASS_NOTHING:
            menuBar->ac_compassLine->setText(tr("Tirer un cap"));
            menuBar->ac_compassLine->setEnabled(true);
            menuBar->ac_compassCenterBoat->setEnabled(true);
            menuBar->ac_compassCenterWp->setEnabled(true);
            break;
        case COMPASS_LINEON:
            /* showing text for compass line off*/
            menuBar->ac_compassLine->setText(tr("Arret du cap"));
            menuBar->ac_compassLine->setEnabled(true);
            menuBar->ac_compassCenterBoat->setEnabled(false);
            menuBar->ac_compassCenterWp->setEnabled(false);
            break;
        case COMPASS_UNDER:
            menuBar->ac_compassLine->setText(tr("Tirer un cap"));
            menuBar->ac_compassLine->setEnabled(true);
            menuBar->ac_compassCenterBoat->setEnabled(true);
            menuBar->ac_compassCenterWp->setEnabled(true);
            break;
    }
    if(my_centralWidget->getRouteToClipboard()!=NULL)
    {
        QString routeName=my_centralWidget->getRouteToClipboard()->getName();
        QPixmap iconI(20,10);
        iconI.fill(my_centralWidget->getRouteToClipboard()->getColor());
        QIcon icon(iconI);
        menuBar->ac_editRoute->setVisible(true);
        menuBar->ac_editRoute->setText(tr("Editer la route")+" "+routeName);
        menuBar->ac_editRoute->setIcon(icon);
        menuBar->ac_editRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->ac_poiRoute->setVisible(true);
        menuBar->ac_poiRoute->setText(tr("Montrer les POIs intermediaires de la route")+" "+routeName);
        menuBar->ac_poiRoute->setIcon(icon);
        menuBar->ac_poiRoute->setChecked(!my_centralWidget->getRouteToClipboard()->getHidePois());
        menuBar->ac_poiRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->mn_simplifyRoute->menuAction()->setVisible(true);
        menuBar->mn_simplifyRoute->setTitle(tr("Simplifier la route")+" "+routeName);
        menuBar->mn_simplifyRoute->setIcon(icon);
        menuBar->ac_simplifyRouteMax->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->ac_simplifyRouteMin->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->ac_optimizeRoute->setVisible(true);
        menuBar->ac_optimizeRoute->setText(tr("Optimiser la route")+" "+routeName);
        menuBar->ac_optimizeRoute->setIcon(icon);
        menuBar->ac_optimizeRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->ac_copyRoute->setVisible(true);
        menuBar->ac_copyRoute->setText(tr("Copier la route")+" "+routeName);
        menuBar->ac_copyRoute->setIcon(icon);
        menuBar->ac_copyRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
        menuBar->ac_zoomRoute->setVisible(true);
        menuBar->ac_zoomRoute->setText(tr("Zoom sur la route ")+" "+routeName);
        menuBar->ac_zoomRoute->setIcon(icon);
        menuBar->ac_deleteRoute->setVisible(true);
        menuBar->ac_deleteRoute->setText(tr("Supprimer la route")+" "+routeName);
        menuBar->ac_deleteRoute->setIcon(icon);
        menuBar->ac_deleteRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
    }
    else
    {
        menuBar->ac_editRoute->setVisible(false);
        menuBar->ac_editRoute->setData(QString());
        menuBar->ac_poiRoute->setVisible(false);
        menuBar->ac_poiRoute->setData(QString());
        menuBar->mn_simplifyRoute->menuAction()->setVisible(false);
        menuBar->ac_simplifyRouteMax->setData(QString());
        menuBar->ac_simplifyRouteMin->setData(QString());
        menuBar->ac_optimizeRoute->setVisible(false);
        menuBar->ac_optimizeRoute->setData(QString());
        menuBar->ac_copyRoute->setVisible(false);
        menuBar->ac_copyRoute->setData(QString());
        menuBar->ac_zoomRoute->setVisible(false);
        menuBar->ac_zoomRoute->setData(QString());
        menuBar->ac_deleteRoute->setVisible(false);
        menuBar->ac_deleteRoute->setData(QString());
    }
    QString clipboard=QApplication::clipboard()->text();
    if(clipboard.isEmpty() || !clipboard.contains("<kml") || !clipboard.contains("Placemark"))
        menuBar->ac_pasteRoute->setEnabled(false);
    else
        menuBar->ac_pasteRoute->setEnabled(true);
    menuPopupBtRight->setEnabled(true);
    menuPopupBtRight->exec(screenPos);
}
void MainWindow::slot_copyRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
        my_centralWidget->exportRouteFromMenuKML(route,"",true);
}
void MainWindow::slot_deleteRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
        my_centralWidget->myDeleteRoute(route);
}
void MainWindow::slot_editRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
        route->slot_edit();
}
void MainWindow::slot_poiRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
    {
        route->setHidePois(!route->getHidePois());
        menuBar->ac_poiRoute->setChecked(!route->getHidePois());
    }
}

void MainWindow::slot_simplifyRouteMax()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
    {
        route->setSimplify(true);
        route->set_strongSimplify(true);
        my_centralWidget->treatRoute(route);
    }
}
void MainWindow::slot_simplifyRouteMin()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
    {
        route->setSimplify(true);
        route->set_strongSimplify(false);
        my_centralWidget->treatRoute(route);
    }
}
void MainWindow::slot_optimizeRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
    {
        route->setOptimize(true);
        my_centralWidget->treatRoute(route);
    }
}
void MainWindow::slot_zoomRoute()
{
    ROUTE *route=NULL;
    for (int n=0;n<my_centralWidget->getRouteList().count();++n)
    {
        if(my_centralWidget->getRouteList().at(n)->getName()==menuBar->ac_copyRoute->data().toString())
        {
            route=my_centralWidget->getRouteList().at(n);
            break;
        }
    }
    if(route!=NULL)
        route->zoom();
}
void MainWindow::slot_pasteRoute()
{
    my_centralWidget->importRouteFromMenuKML("",true);
}
void MainWindow::slot_routeComparator()
{
    DialogRouteComparator * drc=new DialogRouteComparator(my_centralWidget);
    drc->exec();
    delete drc;
}
void MainWindow::slotInetUpdated(void)
{
    //qWarning() << "Inet Updated";
    emit updateInet();
    slotVLM_Sync();
}
void MainWindow::slot_positScale()
{
    qWarning() << "[slot_positScale()]";
    Settings::setSetting(scalePosX,this->mouseClicX);
    Settings::setSetting(scalePosY,this->mouseClicY);
    my_centralWidget->get_terrain()->setScalePos(this->mouseClicX,this->mouseClicY);
    my_centralWidget->get_terrain()->redrawGrib();
    qWarning() << "[slot_positScale()] done";
}

void MainWindow::slot_centerMap()
{
    proj->setCentralPixel(this->mouseClicX,this->mouseClicY);
}

void MainWindow::slotCompassLine(void)
{
    menuBar->ac_compassLine->setDisabled(true);
    emit showCompassLine((double)mouseClicX,(double)mouseClicY);
}
void MainWindow::slotCompassLineForced(double a, double b)
{
    emit showCompassLine(a,b);
}
void MainWindow::slotCompassCenterBoat(void)
{
    Settings::setSetting(compassCenterBoat, menuBar->ac_compassCenterBoat->isChecked()?1:0);
    emit showCompassCenterBoat();
}
void MainWindow::slotCompassCenterWp(void)
{
    menuBar->ac_compassCenterBoat->setChecked(false);
    Settings::setSetting(compassCenterBoat, 0);
    emit showCompassCenterWp();
}

void MainWindow::slotVLM_Param(void)
{
    param = new DialogParamVlm(this,my_centralWidget);
    if(use_old_board)
        connect(param,SIGNAL(paramVLMChanged()),myBoard,SLOT(paramChanged()));

    connect(param,SIGNAL(paramVLMChanged()),this,SLOT(slot_updateGribMono()));
    connect(param,SIGNAL(paramVLMChanged()),toolBar,SLOT(slot_loadEstimeParam()));
    param->slot_changeParam();
    param->exec();
    delete param;
}

void MainWindow::slotVLM_Sync(void)
{
    if (isStartingUp) return;

    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: slotVLM_Sync - empty boatList";
        return ;
    }

    //qWarning() << "Doing a synch with VLM";

    QList<boatVLM*> listBoats = *my_centralWidget->getBoats();
    for(int n=0;n<listBoats.count();++n)
    {
        if(listBoats[n]->getStatus()|| !listBoats.at(n)->isInitialized())
        {
            if(selectedBoat==NULL && listBoats.at(n)->getStatus())
            {
                //qWarning() << "Selecting boat " << listBoats[n]->getName();
                listBoats[n]->slot_selectBoat();
            }
            else
            {
                listBoats[n]->slot_getData(true);
            }
        }
    }
    /* sync finished, update grib date */
    slotDateGribChanged_now(false);
    //emit updateRoute();
 }

void MainWindow::VLM_Sync_sync(void)
{
    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: VLM_Sync_sync - empty boatList";
        closeProgress();
        return ;
    }

    QList<boatVLM*> listBoats = *my_centralWidget->getBoats();
    if(listBoats.isEmpty())
    {
        closeProgress();
        return;
    }
    toolBar->boatList->setEnabled(false);
    --nBoat;
    if(nBoat>=0)
    {
        int p=listBoats.count()-nBoat;
        progress->newStep(60+(19*p)/listBoats.count(),tr("Boats init"));
        acc = listBoats.at(nBoat);
        if(acc->getStatus() || !acc->isInitialized())
        {
            connect(acc,SIGNAL(hasFinishedUpdating(void)),this,SLOT(slot_boatHasUpdated()));
            acc->slot_getData(true);
        }
        else
        {
            VLM_Sync_sync();
        }
    }
    else
    {
        int lastBoatSelected=Settings::getSetting(LastBoatSelected).toInt();
        if(lastBoatSelected!=-10)
        {
            bool found=false;
            for(nBoat=0;nBoat<listBoats.count();++nBoat)
            {
                if(listBoats.at(nBoat)->getId()==lastBoatSelected)
                {
                    if(listBoats.at(nBoat)->getStatus())
                    {
                        listBoats.at(nBoat)->slot_selectBoat();
                        found=true;
                    }
                    break;
                }
            }
            if(!found)
            {
                lastBoatSelected=-10;
                Settings::setSetting(LastBoatSelected,lastBoatSelected);
            }
        }
        if(lastBoatSelected==-10)
        {
            for(nBoat=0;nBoat<listBoats.count();++nBoat)
            {
                if(listBoats.at(nBoat)->getStatus())
                {
                    listBoats.at(nBoat)->slot_selectBoat();
                    Settings::setSetting(LastBoatSelected,listBoats.at(nBoat)->getId());
                    break;
                }
            }
        }
        toolBar->boatList->setEnabled(true);
        closeProgress();
        isStartingUp=false;
        if(Settings::getSetting(centerOnBoatChange).toInt()==1)
            this->slot_centerSelectedBoat();
        my_centralWidget->emitUpdateRoute(NULL);
        updateTitle();
        my_centralWidget->update_menuRoute();
        my_centralWidget->update_menuRoutage();
    }
}
void MainWindow::slot_boatHasUpdated()
{
     disconnect(acc,SIGNAL(hasFinishedUpdating(void)),this,SLOT(slot_boatHasUpdated()));
     VLM_Sync_sync();
}

/*****************************************
 signal send by each boat after it has update
*****************************************/
void MainWindow::slotBoatUpdated(boat * upBoat,bool newRace,bool doingSync)
{
    //qWarning() << "Boat updated " << boat->getBoatPseudo();

    if(upBoat->get_boatType()==BOAT_VLM)
    {
        boatVLM * boat = (boatVLM *)upBoat;
        if(!boat->getStatus())
        {
            slotAccountListUpdated();
            boat->showNextGates();
            return;
        }

        if(boat == selectedBoat)
        {
            lastUpdateTime=QDateTime::currentDateTimeUtc().toTime_t();
            bool found=false;
            //qWarning() << "selected boat update: " << boat->getName();
            timer->stop();
            /* managing race data: opponnents position and trace*/
            int i=0;
            for(i=0;i<my_centralWidget->getRaces().size();++i)
            {
                if(my_centralWidget->getRaces()[i]->idrace == boat->getRaceId())
                {
                    found=true;
                    break;
                }
            }
            if(newRace || my_centralWidget->getOppList()->getRaceId() != boat->getRaceId() ||(found && my_centralWidget->getRaces()[i]->showWhat!=SHOW_MY_LIST))
            { /* load a new race */
                my_centralWidget->drawNSZ(-1);
                if(found)
                {
                    found=false;
                    if(my_centralWidget->getRaces()[i]->idrace == boat->getRaceId())
                    {
                        if(!my_centralWidget->getRaces()[i]->oppList.isEmpty() || my_centralWidget->getRaces()[i]->showWhat!=SHOW_MY_LIST || my_centralWidget->getRaces()[i]->showReal)
                        {
                            //qWarning() << "Set4";
                            my_centralWidget->getOppList()->setBoatList(my_centralWidget->getRaces()[i]->oppList,my_centralWidget->getRaces()[i]->idrace,my_centralWidget->getRaces()[i]->showWhat,false,my_centralWidget->getRaces()[i]->showReal,my_centralWidget->getRaces()[i]->realFilter);
                            found=true;
                        }
                        my_centralWidget->drawNSZ(i);
                    }
                }
                if(!found)
                {
                    my_centralWidget->getOppList()->clear();
                }
            }
            else /* race has not changed, just refreshing position */
            {
                //qWarning() << "Refresh 1";
                my_centralWidget->getOppList()->refreshData();
            }
            /* centering map on boat */
            if(!boat->getFirstSynch())
            {
                //qWarning() << "Not first synch - doingSync="<< doingSync;
                if(doingSync)
                {
                    if(Settings::getSetting(centerOnSynch).toInt()==1)
                        proj->setCenterInMap(boat->getLon(),boat->getLat());
                }
                else
                {
                    if(Settings::getSetting(centerOnBoatChange).toInt()==1)
                        proj->setScaleAndCenterInMap(boat->getZoom(),boat->getLon(),boat->getLat());
                }
            }

            /* updating Vac info */
            nxtVac_cnt=boat->getNextVac();
            //qWarning()<<nxtVac_cnt;
            emit drawVacInfo();
            timer->start(1000);


            /* Updating ETA */            
            QDateTime dtm =QDateTime::fromString(boat->getETA(),"yyyy-MM-ddTHH:mm:ssZ");
            if(!dtm.isValid())
                statusBar->clear_eta();
            else
                statusBar->update_eta(dtm);

            /* change data displayed in all pilototo buttons or menu entry: (nb of instructions passed / tot nb) */
            updatePilototo_Btn(boat);
            /* signal to Board and Pilototo that boat data have changed
           signal to pilototo is needed only when showing the pilototo dialog, the logic of this dialog:
           -> show wait msg box while doing a getData on current boat
           -> once boat send its hasUpdated signal, show the real dialog
        */
            emit boatHasUpdated(boat);

            /* disconnect this signal, it's used only once since after that routes will be updated from the sync slot*/
            disconnect(boat,SIGNAL(boatUpdated(boat*,bool,bool)),this,SIGNAL(updateRoute(boat*)));

            /* send to all POI the new WP, the corresponding WP if exists will draw in a different color*/
            emit WPChanged(boat->getWPLat(),boat->getWPLon());
        }
        emit updateRoute(boat);
        boat->showNextGates();
        if(boat==selectedBoat && boat->getShowNpd())
        {
            QMessageBox::information (this,
                QString(tr("Votre blocnote a change!")),
                boat->getNpd());
            boat->setShowNpd(false);
        }
    }
    else
    {
        /* Updating ETA */
        boatReal *boat=(boatReal *) upBoat;
        if((boat->getWPLat()==0 && boat->getWPLon()==0) ||boat->getEta()==-1)
            statusBar->clear_eta();
        else
        {
            QDateTime dtm =QDateTime::fromTime_t(boat->getEta()).toUTC();
            if(!dtm.isValid() || boat->getEta()<=0)
                statusBar->clear_eta();
            else
                statusBar->update_eta(dtm);
        }
        emit boatHasUpdated(upBoat);
        //emit WPChanged(upBoat->getWPLat(),upBoat->getWPLon());
    }

}

void MainWindow::slotSelectBoat(boat* newSelect)
{
    if(!newSelect || !newSelect->getStatus()) return;
    if(!my_centralWidget->getBoats() && newSelect->get_boatType() == BOAT_VLM)
    {
        qWarning() << "CRITICAL: slotSelectBoat - empty boatList";
        return ;
    }
    if(newSelect->get_boatType()!=BOAT_VLM)
    {
        selectedBoat=newSelect;
        emit boatSelected(selectedBoat);
        selectedBoat->slot_selectBoat();
        selectedBoat->setZoom(proj->getScale());
        emit updateRoute(selectedBoat);
        return;
    }

    if(newSelect != selectedBoat)
    {
        /* did we have already a selected boat ? */
        if(selectedBoat)
        {
            selectedBoat->unSelectBoat(true); /* ask for unselect + update */
            /* save the zoom factor */
            selectedBoat->setZoom(proj->getScale());
        }

        selectedBoat=newSelect;
        emit boatSelected(selectedBoat);
        selectedBoat->slot_selectBoat();

        /* change the board ? */
        //emit boatChanged(selectedBoat);

        if(newSelect->getStatus()) /* is selected boat activated ?*/
        {
            if(newSelect->get_boatType()==BOAT_VLM)
            {
                //qWarning() << "getData from slot_selectBoat";
                if(!this->isStartingUp)
                    ((boatVLM*)newSelect)->slot_getData(false);
                menuBar->acPilototo->setEnabled(true);
                slotFile_Lock(true);
            }

            slotDateGribChanged_now(false);
            //proj->setScaleAndCenterInMap(newSelect->getZoom(),newSelect->getLon(),newSelect->getLat());
        }
        else
        {
            if(newSelect->get_boatType()==BOAT_VLM)
                menuBar->acPilototo->setEnabled(false);
        }

        /* manage item of boat list */
        if(newSelect->get_boatType()==BOAT_VLM)
        {
            int cnt=0;
            for(int i=0;i<my_centralWidget->getBoats()->count();++i)
            {
                if(my_centralWidget->getBoats()->at(i) == newSelect)
                {
                    toolBar->setSelectedBoatIndex(cnt);
                    break;
                }
                if(my_centralWidget->getBoats()->at(i)->getStatus())
                    ++cnt;
            }
        }

    }
    emit paramVLMChanged();
}

/***********************************
  Called when boat list is changed
  *********************************/

void MainWindow::slotChgBoat(int num)
{
    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: slotChgBoat - empty boatList";
        return ;
    }

    QListIterator<boatVLM*> i (*my_centralWidget->getBoats());
    int cnt=0;

    //qWarning() << "Selecting boat " << num;

    while(i.hasNext())
    {
        boatVLM * acc = i.next();
        if(acc->getStatus())
        {
            if(cnt==num)
            {
                Settings::setSetting(LastBoatSelected, acc->getId());
                acc->slot_selectBoat();
                /* sync launched, update grib date */
                emit WPChanged(0,0);
                emit selectedBoatChanged();
                emit updateRoute(acc);
                for(int i=0;i<my_centralWidget->getRaces().size();++i)
                {
                    if(my_centralWidget->getRaces()[i]->idrace == acc->getRaceId())
                    {
                        my_centralWidget->drawNSZ(i);
                        break;
                    }
                }
                slotFile_Lock(true);
                if(use_old_board)
                    myBoard->VLMBoard()->setChangeStatus(!selectedBoat->getLockStatus());
                break;
            }
            ++cnt;
        }
    }
}

void MainWindow::slot_updPlayerFinished(bool res_ok, Player * player)
{
    disconnect(player,SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updPlayerFinished(bool,Player*)));
    if(!res_ok)
    {
        player->setWrong(true);
        qWarning() << "Erreur de MaJ player";
        isStartingUp=false;
        updateTitle();
        my_centralWidget->slot_playerSelected(player);
        my_centralWidget->loadPOI();
        closeProgress();
        my_centralWidget->emitUpdateRoute(NULL);
        return;
    }
    progress->setLabelText(tr("Updating players"));
    progress->setValue(55);

    my_centralWidget->updatePlayer(player);
    progress->setLabelText(tr("Selecting player"));
    progress->setValue(56);
    my_centralWidget->slot_playerSelected(player);
    progress->setLabelText(tr("loading POIs"));
    progress->setValue(57);
    my_centralWidget->loadPOI();
    nBoat=my_centralWidget->getBoats()->size();
    toBeCentered=-1;
    //qWarning()<<"ready to update boats";
    if(nBoat>0)
    {
        progress->setLabelText(tr("Updating boats"));
        progress->setValue(60);
        VLM_Sync_sync();
        return;
    }
    else
    {
        qWarning()<<"player has no boats";
        isStartingUp=false;
        updateTitle();
    }
    my_centralWidget->emitUpdateRoute(NULL);
    closeProgress();
    //qWarning()<<"Boat has finished updating";
}

void MainWindow::slotSelectPOI(DialogPilototoInstruction * instruction)
{
    selPOI_instruction=instruction;
    toolBar->boatList->setEnabled(false);
    updatePilototo_Btn((boatVLM*)selectedBoat);
    slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
    my_centralWidget->clearOtherSelected(NULL);
}

void MainWindow::slotSelectWP_POI()
{
    isSelectingWP=true;
    toolBar->boatList->setEnabled(false);
    my_centralWidget->clearOtherSelected(NULL);
    slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
}

bool MainWindow::get_selPOI_instruction()
{
    return ((selPOI_instruction!=NULL) || (isSelectingWP));
}

void MainWindow::slot_POIselected(POI* poi)
{
    qWarning()<<"main slot_Poiselected with"<<isSelectingWP;
    if(selPOI_instruction)
    {
        DialogPilototoInstruction * tmp=selPOI_instruction;
        selPOI_instruction=NULL;
        updatePilototo_Btn((boatVLM*)selectedBoat);
        toolBar->boatList->setEnabled(true);
        slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
        emit editInstructionsPOI(tmp,poi);
    }
    else if(isSelectingWP)
    {
        isSelectingWP=false;
        toolBar->boatList->setEnabled(true);
        slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
        emit editWP_POI(poi);
    }

}

void MainWindow::slotAccountListUpdated(void)
{

    selectedBoat=NULL;

    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: slotAccountListUpdated - empty boatList";
        return ;
    }

    //qWarning() << "Boat list updated: " << my_centralWidget->getBoats()->count() << " boats";

    toolBar->updateBoatList(*my_centralWidget->getBoats());

    emit accountListUpdated(my_centralWidget->getPlayer());

    slotVLM_Sync();
}

void MainWindow::slotChgWP(double lat,double lon, double wph)
{
    if(!selectedBoat || selectedBoat->getLockStatus()) return;

    selectedBoat->setWP(QPointF(lon,lat),wph);
    emit WPChanged(lat,lon);
    if(use_old_board)
    {
        if(this->selectedBoat->get_boatType()==BOAT_VLM)
        {
            if(myBoard->VLMBoard())
            {
                ((boatVLM*)selectedBoat)->setWph(wph);
                myBoard->VLMBoard()->setWP(lat,lon,wph);
            }
        }
        else
        {
            if(myBoard->realBoard())
            {
                myBoard->realBoard()->setWp(lat,lon,wph);
                emit this->WPChanged(lat,lon);
            }
        }
    }
}

void MainWindow::slotpastePOI()
{
    double lon, lat,wph;
    int tstamp;
    QString name;

    if(!Util::getWPClipboard(&name,&lat,&lon,&wph,&tstamp))
        return;

    emit addPOI(name,POI_TYPE_POI,lat,lon,wph,tstamp,tstamp!=-1);
}

void MainWindow::slotBoatLockStatusChanged(boat* boat,bool status)
{
    if(boat==selectedBoat)
    {
        if(selPOI_instruction)
        {
            emit setChangeStatus(true,true,false);
            if(use_old_board)
                myBoard->VLMBoard()->btn_Pilototo->setEnabled(true);
        }
        else if(isSelectingWP)
        {
            emit setChangeStatus(true,false,false);
            if(use_old_board)
                myBoard->VLMBoard()->btn_Pilototo->setEnabled(false);
        }
        else
        {
            emit setChangeStatus(status,!status,true);
            if(use_old_board)
                myBoard->VLMBoard()->btn_Pilototo->setEnabled(true);
        }
    }
}

bool MainWindow::getBoatLockStatus(void)
{
    if(!selectedBoat)
        return false;
    if(selPOI_instruction)
        return true;
    return selectedBoat->getLockStatus();
}
void MainWindow::slotShowPolar()
{
    if(!selectedBoat) return;
    DialogViewPolar *dvp=new DialogViewPolar(this->my_centralWidget);
    dvp->setBoat(selectedBoat);
    dvp->exec();
    if(dvp)
        delete dvp;
}

void MainWindow::slotPilototo(void)
{
    if(!selectedBoat || selectedBoat->get_boatType()!=BOAT_VLM) return;

    if(selPOI_instruction)
    {
        slot_POIselected(NULL);
    }
    else
    {
        emit editInstructions();
        ((boatVLM*)selectedBoat)->slot_getData(true);
    }
}
void MainWindow::setPilototoFromRoute(ROUTE *route)
{
    if(!route->getBoat() || route->getBoat()->get_boatType()!=BOAT_VLM)
    {
        route->setPilototo(false);
        return;
    }
    if(route->getPoiList().count()==0)
    {
        route->setPilototo(false);
        return;
    }
    if(route->getPoiList().count()==1)
    {
        route->getPoiList().at(0)->slot_setWP();
        route->setPilototo(false);
        return;
    }
    QList<POI *> pois;
    for (int n=0;n<route->getPoiList().count() && n<=5;++n)
    {
        POI * poi=route->getPoiList().at(n);
        if(poi->getRouteTimeStamp()==-1)
            break;
        if(poi->getRouteTimeStamp()+30<=(int)QDateTime().currentDateTimeUtc().toTime_t()) continue;
        if(n>0)
            poi->setPiloteDate(route->getPoiList().at(n-1)->getRouteTimeStamp());
        pois.append(poi);
    }
    route->setPilototo(false);
    emit setInstructions(route->getBoat(),pois);
}

void MainWindow::setPilototoFromRoute(QList<POI*> poiList)
{
    if(poiList.isEmpty()) return;
    if(poiList.at(0)->getRoute()==NULL) return;
    ROUTE * route=poiList.at(0)->getRoute();
    if(!route->getBoat() || route->getBoat()->get_boatType()!=BOAT_VLM)
    {
        route->setPilototo(false);
        return;
    }
#if 0
    if(poiList.count()==1)
    {
        poiList.at(0)->slot_setWP();
        poiList.at(0)->setPiloteDate(-1);
        route->setPilototo(false);
        return;
    }
#endif
    route->setPilototo(false);
    emit setInstructions(route->getBoat(),poiList);
}
void MainWindow::slot_clearPilototo()
{
    QList<POI*> vide;
    if(this->selectedBoat)
        emit setInstructions(this->selectedBoat,vide);
}

bool MainWindow::isBoat(QString idu)
{
    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: isBoat - empty boatList";
        return false;
    }

    for(int i=0;i<my_centralWidget->getBoats()->count();++i)
        if(my_centralWidget->getBoats()->at(i)->getBoatId() == idu)
        {
//            if(my_centralWidget->getBoats()->at(i)->getStatus()
//                && my_centralWidget->getBoats()->at(i)==selectedBoat)
                if(my_centralWidget->getBoats()->at(i)->getStatus())
                return true;
            else
                return false;
        }
    return false;
}

void MainWindow::slotParamChanged(void)
{
    emit paramVLMChanged();
}

void MainWindow::getBoatBvmg(double * up,double * down,double ws)
{
   boat *bateau;
   if(my_centralWidget->getCompassFollow()!=NULL)
       bateau=my_centralWidget->getCompassFollow()->getBoat();
   else
       bateau=selectedBoat;
   if(!bateau)
   {
       *up=-1;
       *down=-1;
   }
   else
       if(bateau->getPolarData())
       {
            *up=bateau->getBvmgUp(ws);
            *down=bateau->getBvmgDown(ws);
       }
       else
       {
            *up=-1;
            *down=-1;
       }
}
double MainWindow::getBoatPolarSpeed(double ws,double angle)
{
   boat *bateau;
   if(my_centralWidget->getCompassFollow()!=NULL)
       bateau=my_centralWidget->getCompassFollow()->getBoat();
   else
       bateau=selectedBoat;
   if(!bateau)
   {
       return 0;
   }
   else
       if(bateau->getPolarData())
       {
            return bateau->getPolarData()->getSpeed(ws,angle,false);
       }
       else
       {
            return 0;
       }
}
double MainWindow::getBoatPolarMaxSpeed()
{
   if(!selectedBoat)
   {
       return 0;
   }
   else
       if(selectedBoat->getPolarData())
       {
            return selectedBoat->getPolarData()->getMaxSpeed();
       }
       else
       {
            return 0;
       }
}

int MainWindow::get_boatType(void) {
    if(selectedBoat) return selectedBoat->get_boatType();
    else return BOAT_NOBOAT;
}

void MainWindow::getBoatWP(double * lat,double * lon)
{
   if(!lat || !lon)
       return;
   if(!selectedBoat || selectedBoat->get_boatType()!=BOAT_VLM)
   {
       *lat=-1;
       *lon=-1;
   }
   else
   {
       if(this->my_centralWidget->getPlayer()->getWrong())
       {
           *lat=-1;
           *lon=-1;
       }
       else
       {
           *lat = ((boatVLM*)selectedBoat)->getWPLat();
           *lon = ((boatVLM*)selectedBoat)->getWPLon();
       }
   }
}

void MainWindow::slotNewZoom(double zoom)
{
    if(selectedBoat)
        selectedBoat->setZoom(zoom);
}

void MainWindow::releasePolar(QString fname)
{
    polar_list->releasePolar(fname);
}

void MainWindow::slotLoadVLMGrib(void)
{
    loadVLM_grib = new DialogVlmGrib_ctrl(this,my_centralWidget,my_centralWidget->getInet());
}

/*************************************/
#ifdef __QTVLM_WITH_TEST

#include <QDomDocument>

void MainWindow::slotVLM_Test(void)
{
    QFile file("polar.xml");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString  errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn))
    {
        QMessageBox::warning(0,QObject::tr("Lecture de polaire ("),
                             QString("Erreur ligne %1, colonne %2:\n%3")
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return ;
    }

    QDomElement root = doc.documentElement();

    qWarning() << "Root: " << root.tagName();
    qWarning() << "Name: " << root.firstChild().toText().data().simplified();

    QDomNode subNode= root.firstChild().nextSibling();
    QMap <int,QMap<int,double>> dataMap;

    while(!subNode.isNull())
    {
        qWarning() << i;
        if(subNode.toElement().tagName() == "PolarCurve") {
            QDomNode polarCurve=subNode.firstChild();
            int curveIndex=0;
            bool hasCurveIndex=false;
            QMap<int,double> curveMap;
            while(!polarCurve.isNull())
            {
                if(polarCurve.toElement().tagName()=="PolarCurveIndex") {
                    curveIndex=polarCurve.toElement().attribute("value").toInt(&hasCurveIndex);
                }
                if(polarCurve.toElement().tagName()=="PolarItem") {
                    QDomNode polarItem=polarCurve.firstChild();
                    int angle;
                    bool hasAngle=true;
                    double val;
                    bool hasVal=false;
                    while(!polarItem.isNull()) {
                        if(polarItem.toElement().tagName()=="Angle") {
                            angle=polarItem.toElement().attribute("value").toInt(&hasAngle);
                        }
                        if(polarItem.toElement().tagName()=="Value") {
                            val=polarItem.toElement().attribute("value").toInt(&hasVal);
                        }
                        polarItem=polarItem.nextSibling();
                    }
                    if(hasAngle && hasVal) {
                        curveMap.insert(angle,val);
                    }
                }
                polarCurve = polarCurve.nextSibling();
            }
            if(hasCurveIndex) {
                dataMap.insert(curveIndex,curveMap);
            }
        }
        subNode = subNode.nextSibling();
    }

    if(!dataMap.isEmpty()) {

    }

}
#else
void MainWindow::slotVLM_Test(void)
{

}
#endif
void MainWindow::slotGribInterpolation(void)
{
//#ifdef __QTVLM_WITH_TEST
//    if(my_centralWidget->getGrib())
//    {
//        gribValidation_dialog->setMode(this->my_centralWidget->getGrib()->getInterpolationMode());
//        gribValidation_dialog->show();
//    }
//#endif
}

void MainWindow::slot_updateGribMono()
{
    //gribDrawingmethod (0=auto, 1=mono, 2=multi)
    if(my_centralWidget->get_mapDataDrawer())
    {
#ifdef __WIN_QTVLM
        if (QSysInfo::windowsVersion()==QSysInfo::WV_XP)
        {
            my_centralWidget->get_mapDataDrawer()->set_grib_monoCpu(true);
            return;
        }
#endif
        bool gribMulti=false;
        if(Settings::getSetting(gribDrawingMethod).toInt()==0)
            gribMulti=Settings::getSetting(gribBench1).toInt()<Settings::getSetting(gribBench2).toInt();
        else
            gribMulti=Settings::getSetting(gribDrawingMethod).toInt()==2;
        my_centralWidget->get_mapDataDrawer()->set_grib_monoCpu(!gribMulti);
    }

}
void MainWindow::slot_disablePopupMenu()
{
    this->menuPopupBtRight->setDisabled(true);
}
void MainWindow::slot_showPOI_input(POI* poi, const bool &fromMenu)
{
    bool creationMode=false;
    if(poi==NULL)
    {
        creationMode=true;
        double lon, lat;
        if(fromMenu)
        {
            lon=0;
            lat=0;
        }
        else
            proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
        poi=new POI(tr("POI"), POI_TYPE_POI,lat, lon, proj, this,
                    my_centralWidget, -1,-1,false);
    }
    DialogPoi dp(this,my_centralWidget);
    dp.initPOI(poi,creationMode);
    dp.exec();
}

void MainWindow::slot_POI_input() {
    DialogPoiInput * d=new DialogPoiInput(my_centralWidget);
    d->exec();
    d->deleteLater();
}

/****************************************************************
 * Barrier
 ***************************************************************/

void MainWindow::slot_newBarrierSet() {
    if(my_centralWidget->get_barrierEditMode()!=BARRIER_EDIT_NO_EDIT)
        my_centralWidget->escKey_barrier();

    BarrierSet * barrierSet = new BarrierSet(this);
    barrierSet->set_name(tr("New set"));

    DialogEditBarrier dialogEditBarrier(this);

    // auto add set to selected boat
    if(selectedBoat) {
        selectedBoat->add_barrierSet(barrierSet);
    }

    dialogEditBarrier.initDialog(barrierSet,my_centralWidget->get_boatList());

    if(dialogEditBarrier.exec() == QDialog::Rejected) {
        //if(selectedBoat) selectedBoat->rm_barrierSet(barrierSet); not needed as remove is done in barrierSet destructor
        delete barrierSet;
    }
    else {
        ::barrierSetList.append(barrierSet);
        barrierSet->set_key(barrierSet->get_name().toUtf8().toBase64()+Util::generateKey(10));
        for(int i=0;i<my_centralWidget->get_boatList().count();++i)
            my_centralWidget->get_boatList().at(i)->update_barrierKey(barrierSet);
    }
}

void MainWindow::slot_barrierAddPopup(void) {
    if(my_centralWidget->get_barrierEditMode()==BARRIER_EDIT_NO_EDIT)
        my_centralWidget->slot_newBarrier();
    else
        my_centralWidget->escKey_barrier();
}

void MainWindow::slot_barrierAddMenu(void) {
    if(my_centralWidget->get_barrierEditMode()==BARRIER_EDIT_NO_EDIT)
        my_centralWidget->slot_addBarrier();
    else
        my_centralWidget->escKey_barrier();
}

QVariant MainWindow::getSettingApp(const int &key) const
{
    return Settings::getSetting(key);
}

void MainWindow::setting_saveGeometry(QWidget * obj) {
    Settings::saveGeometry(obj);
}

bool MainWindow::getWPClipboard(QString * str,double * lat,double * lon, double * wph, int * tStamp) {
    return Util::getWPClipboard(str,lat,lon,wph,tStamp);
}

void MainWindow::setWPClipboard(double lat,double lon, double wph) {
    Util::setWPClipboard(lat,lon,wph);
}

QString MainWindow::pos2String(const int &type,const double &value) {
    return Util::pos2String(type,value);
}

QString MainWindow::formatLongitude(const double &x) {
    return Util::formatLongitude(x);
}

QString MainWindow::formatLatitude(const double &y) {
    return Util::formatLatitude(y);
}

QString MainWindow::get_folder(QString str) const {
    return appFolder.value(str);
}

QPalette MainWindow::getOriginalPalette() const
{
    return originalPalette;
}

void MainWindow::setFontDialog(QObject * o) {
    Util::setFontObject(o);
}
void MainWindow::manageWPDialog(BoatInterface * myBoat, QObject * boardPlugin)
{
    if(!myBoat) return;
    DialogWp * wpDialog = new DialogWp(this->my_centralWidget);
    wpDialog->setLocked(!myBoat->getLockStatus());
    connect(wpDialog,SIGNAL(selectPOI()),boardPlugin,SLOT(slot_selectWP_POI()));
    connect(this,SIGNAL(editWP_POI(POI*)),wpDialog,SLOT(slot_selectPOI(POI*)));
    connect(wpDialog,SIGNAL(finished(int)),wpDialog,SLOT(deleteLater()));
    wpDialog->show_WPdialog((boat*)myBoat);
}
