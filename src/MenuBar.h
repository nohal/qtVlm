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

#ifndef MENU_H
#define MENU_H

#include <QMenuBar>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

#include <set>

#include "class_list.h"
#include "dataDef.h"

//================================================================
// ActionGroup which allows 0 or 1 checked action
class ZeroOneActionGroup : public QObject
{ Q_OBJECT
        public:
                ZeroOneActionGroup(QWidget *parent) : QObject(parent) {}
                void addAction(QAction *);
                void  setCheckedAction(QAction *act, bool b);

        public slots:
                void slot_actionTrigerred(bool);
        signals:
                // Send the checked action, or NULL if there is no cheched action
                void triggered(QAction *);

        private:
                QList <QAction *> lsactions;
};
Q_DECLARE_TYPEINFO(ZeroOneActionGroup,Q_MOVABLE_TYPE);

class MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MenuBar(MainWindow *parent);

    void setCitiesNamesLevel(int level);
    void setMenubarColorMapMode(int colorMapMode);
    void setIsobarsStep(int step);
    void setIsotherms0Step(int step);

    void clearListeDates(void);
    void setSelectedBoatIndex(int index);
    void addMenuRoute(ROUTE * route);
    void addMenuRoutage(ROUTAGE * routage);
    QAction * addReleaseCompass();



    QMenu * createPopupBtRight(QWidget *parent);

    //---------------------------------------------------------
    // Actions des menus
    // Elements de l'interface (public c'est plus pratique)
    //---------------------------------------------------------
    QAction *acHorn;
    QAction *ac_CreatePOI;
    QAction *ac_pastePOI;

    QAction *acPOIAdd;
    QAction *acPOIRemove;

    QMenu * subMenuBarrier;
    QMenu * subSubMenuDelBarrierSet;
    QMenu * subSubMenuEditBarrierSet;
    QAction * ac_addBarrierSet;
    QAction * ac_barrierTool;
    QAction * ac_addBarrier;

    QAction *ac_twaLine;
    QAction *ac_compassLine;
    QAction *ac_compassCenterBoat;
    QAction *ac_compassCenterWp;
    QMenu   *mnCompassCenterRoute;
    QAction *ac_centerMap;
    QAction *ac_positScale;

    QAction * ac_moveBoat;
    QAction * ac_moveBoatSep;

    QAction * ac_copyRoute;
    QAction * ac_deleteRoute;
    QAction * ac_editRoute;
    QAction * ac_poiRoute;
    QAction * ac_simplifyRouteMax;
    QAction * ac_simplifyRouteMin;
    QMenu   * mn_simplifyRoute;
    QAction * ac_optimizeRoute;
    QAction * ac_pasteRoute;
    QAction * ac_zoomRoute;

    QAction *acFile_Open;
    QAction *acFile_Reopen;
    QAction *acFile_Close;
    QAction *acFile_Open_Current;
    QAction *acFile_Close_Current;
    QAction *acFile_Load_GRIB;
    QAction *acFile_Load_VLM_GRIB;
    QAction *acFile_Load_SAILSDOC_GRIB;
    QAction *acFile_Info_GRIB;
    QAction *acFile_Quit;
    QAction *acFile_Lock;
    QAction *separator1;
    QAction *acFile_QuitNoSave;
    QMenu   *mn_img;
    QAction *acImg_Open;
    QAction *acImg_Close;
    QMenu   *mn_fax;
    QAction *acFax_Open;
    QAction *acFax_Close;

    QAction *acRoute_add;
    QMenu   *mnRoute_edit;
    QMenu   *mnRoute_delete;
    QMenu   *mnRoute_import;
    QAction   *acRoute_import;
    QAction   *acRoute_import2;
    QMenu   *mnRoute_export;
    QAction   *acRoute_paste;

    QAction *acRoutage_add;
    QMenu   *mnRoutage_edit;
    QMenu   *mnRoutage_delete;

    QMenu   *menuGroupColorMap;
    ZeroOneActionGroup *acView_GroupColorMap;
    QAction *acView_WindColors;
    QAction *acView_CurrentColors;
    QAction *acView_RainColors;
    QAction *acView_CloudColors;
    QAction *acView_HumidColors;
    QAction *acView_TempColors;
    QAction *acView_TempPotColors;
    QAction *acView_DeltaDewpointColors;
    QAction *acView_SnowCateg;
    QAction *acView_SnowDepth;
    QAction *acView_FrzRainCateg;
    QAction *acView_CAPEsfc;

    QAction *acView_Isobars;
    QMenu   *menuIsobars;
    QMenu   *menuIsobarsStep;
    QActionGroup *acView_GroupIsobarsStep;
        QAction *acView_IsobarsStep1;
        QAction *acView_IsobarsStep2;
        QAction *acView_IsobarsStep3;
        QAction *acView_IsobarsStep4;
        QAction *acView_IsobarsStep5;
        QAction *acView_IsobarsStep6;
        QAction *acView_IsobarsStep8;
        QAction *acView_IsobarsStep10;
    QAction *acView_IsobarsLabels;

    QAction *acView_Isotherms0;
    QMenu   *menuIsotherms0;
    QMenu   *menuIsotherms0Step;
    QActionGroup *acView_GroupIsotherms0Step;
        QAction *acView_Isotherms0Step10;
        QAction *acView_Isotherms0Step20;
        QAction *acView_Isotherms0Step50;
        QAction *acView_Isotherms0Step100;
        QAction *acView_Isotherms0Step200;
        QAction *acView_Isotherms0Step500;
        QAction *acView_Isotherms0Step1000;
    QAction *acView_Isotherms0Labels;

    QAction *acView_PressureMinMax;
    QAction *acView_TemperatureLabels;

    QAction *acView_ColorMapSmooth;

    QAction *acView_WindArrow;
    QAction *acView_Barbules;

    QMenu * menuAltitude;
    QActionGroup *acAlt_GroupAltitude;
                QAction *acAlt_MSL;
                QAction *acAlt_sigma995;
                QAction *acAlt_GND;
                QAction *acAlt_GND_1m;
                QAction *acAlt_GND_2m;
                QAction *acAlt_GND_3m;
                QAction *acAlt_GND_10m;
                QAction *acAlt_850hpa;
                QAction *acAlt_700hpa;
                QAction *acAlt_500hpa;
                QAction *acAlt_300hpa;
                QAction *acAlt_200hpa;
                QAction *acAlt_Atmosphere;

    ZeroOneActionGroup *acAlt_GroupGeopotLine;
                QAction *acAlt_GeopotLine_850hpa;
                QAction *acAlt_GeopotLine_700hpa;
                QAction *acAlt_GeopotLine_500hpa;
                QAction *acAlt_GeopotLine_300hpa;
                QAction *acAlt_GeopotLine_200hpa;

        QMenu *menuGeopotStep;
    QActionGroup *acAlt_GroupGeopotStep;
        QAction *acAlt_GeopotStep_1;
        QAction *acAlt_GeopotStep_2;
        QAction *acAlt_GeopotStep_5;
        QAction *acAlt_GeopotStep_10;
        QAction *acAlt_GeopotStep_20;
        QAction *acAlt_GeopotStep_50;
        QAction *acAlt_GeopotStep_100;
    QAction *acAlt_GeopotLabels;

    QAction *acMap_Orthodromie;
    QAction *acMap_Rivers;
    QAction *acMap_CountriesBorders;
    QAction *acMap_CountriesNames;
#ifdef __QTVLM_WITH_TEST
    QAction * acVLMTest;
    QAction * acGribInterpolation;
#endif

    QActionGroup *acMap_GroupCitiesNames;
        QAction *acMap_CitiesNames0;
        QAction *acMap_CitiesNames1;
        QAction *acMap_CitiesNames2;
        QAction *acMap_CitiesNames3;
        QAction *acMap_CitiesNames4;

    QAction *acMap_Zoom_In;
    QAction *acMap_Zoom_Out;
    QAction *acMap_Zoom_Sel;
    QAction *acMap_Zoom_All;
    QAction *acMap_sel;

    QAction *acOptions_Proxy;
    QAction *acOptions_Units;
    QAction *acOptions_GraphicsParams;
    QActionGroup *acOptions_GroupLanguage;
    QAction *acOptions_Lang_fr;
    QAction *acOptions_Lang_en;
    QAction *acOptions_Lang_es;
    QAction *acOptions_Lang_cz;



     QActionGroup *acOptions_GroupShowHide;
     QAction *acOptions_SH_sAll;
     QAction *acOptions_SH_hAll;
     QAction *acOptions_SH_Opp;
     QAction *acOptions_SH_Poi;
     QAction *acOptions_SH_Com;
     QAction *acOptions_SH_ComBandeau;
     QAction *acOptions_SH_Rou;
     QAction *acOptions_SH_Routage;
     QAction *acOptions_SH_Por;
     QAction *acOptions_SH_Lab;
     QAction *acOptions_SH_Pol;
     QAction *acOptions_SH_Boa;
     QAction *acOptions_SH_Fla;
     QAction *acOptions_SH_Nig;
     QAction *acKeep;
     QAction *acReplay;
    QAction *acScreenshot;
    QAction *acShowLog;
    QAction *acGetTrack;

    QAction *acHelp_Help;
    QAction *acHelp_APropos;
    QAction *acHelp_AProposQT;
    QAction *acHelp_Forum;

    QAction *acVLMParam;
    QAction *acVLMParamBoat;
    QAction *acVLMParamPlayer;
    QAction *acPOIinput;
    QAction *acPOIimport;
    QAction *acPOIgeoData;
    QAction *acVLMSync;
    QAction *acPOISave;
    QAction *acPOIRestore;
    QAction *acPilototo;
    QAction *acShowPolar;
    QAction *acRace;

public slots:
    void slot_updateLockIcon(QIcon ic);
    void slot_setChangeStatus(bool status,bool pilototo,bool syncBtn);
    void slot_showViewMenu(void);


    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------


    void setMCW(myCentralWidget * mcw){my_CentralWidget=mcw;}

//------------------------------------------------------------------------
private:

    MainWindow * mainWindow;

    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuGrib;
    QMenu *menuOptions;
    QMenu *menuBoat;
    QMenu *menuRoute;
    QMenu *menuRoutage;
    QMenu *menuPOI;
    QMenu *menuHelp;

    QMenu * boardMenu;
    QMenu * toolBarMenu;

    //std::vector<time_t> listGribDates;

    QAction* addAction(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");

    QAction* addActionCheck(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");
    myCentralWidget * my_CentralWidget;


};
Q_DECLARE_TYPEINFO(MenuBar,Q_MOVABLE_TYPE);

#endif
