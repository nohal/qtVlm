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
                void clear(void);

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
    void setMenubarColorMapMode(int colorMapMode, bool withoutEvent=false);

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
    QAction *acPOIRemoveByType;

    /*** Barrier ***/
    QAction * ac_popupBarrier; // popup
    QMenu * subMenuBarrier;
    QMenu * subSubMenuDelBarrierSet;
    QMenu * subSubMenuEditBarrierSet;
    QAction * ac_addBarrierSet;
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
    QAction * ac_pasteRoute;

    QAction *acFile_Open;
    QAction *acFile_Reopen;
    QAction *acFile_Close;
    QAction *acFile_Open_Current;
    QAction *acFile_Close_Current;
    QAction *acFile_Load_GRIB;
    QAction *acFile_Load_VLM_GRIB;
    QAction *acFile_Load_SAILSDOC_GRIB;
    QAction *acFile_Info_GRIB_main;
    QAction *acFile_Info_GRIB_current;
    QAction * acCombineGrib;
    QAction *acFile_Quit;
    QAction *acFile_Lock;
    QAction *separator1;
    QAction *separator2;
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
    QAction * acRouteRemove;
    QMenu   *mnRoute_import;
    QAction   *acRoute_import;
    QAction   *acRoute_import2;
    QAction   *acRoute_import3;
    QMenu   *mnRoute_export;
    QAction   *acRoute_paste;
    QAction *acRoute_comparator;

    QAction *acRoutage_add;
    QMenu   *mnRoutage_edit;
    QMenu   *mnRoutage_delete;

    QAction * acGrib_dialog;

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

    QAction *acOptions_Proxy;
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
     QAction *acOptions_SH_barSet;
     QAction *acOptions_SH_trace;
     QAction *acOptions_SH_Pol;
     QAction *acOptions_SH_Boa;
     QAction *acOptions_SH_Fla;
     QAction *acOptions_SH_Nig;
     QAction *acOptions_SH_Scale;
     QAction *acOptions_SH_Tdb;
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

    void setPlayerType(const int &type);
    void setNewMenu(QList<QMenu*> *newMenu);
public slots:
    void slot_updateLockIcon(QString ic);
    void slot_setChangeStatus(bool status,bool pilototo,bool syncBtn);
    void slot_showViewMenu(void);
    void slot_showBarrierMenu(void);   


    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------


    void setMCW(myCentralWidget * mcw){my_CentralWidget=mcw;}

    QAction* addAction(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");

    QAction* addActionCheck(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");

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
    QList<QMenu*> mainMenu;
    QList<QMenu*> * currentMenu;
    //QMenu * boardMenu;
    QMenu * toolBarMenu;

    //std::vector<time_t> listGribDates;


    myCentralWidget * my_CentralWidget;


    void setRules(QAction *act);
};
Q_DECLARE_TYPEINFO(MenuBar,Q_MOVABLE_TYPE);

#endif
