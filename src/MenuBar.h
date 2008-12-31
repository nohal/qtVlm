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

#include <set>

#include "boatAccount.h"

class MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MenuBar(QWidget *parent);

    void setQuality(int q);
    void setIsobarsStep(int step);
    void setCitiesNamesLevel(int level);

    void updateListeDates(std::set<time_t> *setDates);
    time_t  getDateGribById(int id);

    void getBoatList(QList<boatAccount*> & acc_list);


    QMenu * createPopupBtRight(QWidget *parent);

    //---------------------------------------------------------
    // Actions des menus
    // Elements de l'interface (public c'est plus pratique)
    //---------------------------------------------------------
    QAction *ac_OpenMeteotable;
    QAction *ac_CreatePOI;
    QAction *ac_pastePOI;
    QAction *ac_delPOIs;

    QAction *acFile_Open;
    QAction *acFile_Close;
    QAction *acFile_Load_GRIB;
    QAction *acFile_Info_GRIB;
    QAction *acFile_Quit;

    QAction *acView_WindColors;
    QAction *acView_RainColors;
    QAction *acView_CloudColors;
    QAction *acView_HumidColors;
    QAction *acView_ColorMapSmooth;

    QAction *acView_WindArrow;
    QAction *acView_Barbules;
    QAction *acView_Isobars;
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
    QAction *acView_PressureMinMax;
    QAction *acView_TemperatureLabels;

    QAction *acView_GribGrid;

    QAction *acMap_Orthodromie;
    QAction *acMap_Rivers;
    QAction *acMap_CountriesBorders;
    QAction *acMap_CountriesNames;

    QActionGroup *acMap_GroupCitiesNames;
        QAction *acMap_CitiesNames0;
        QAction *acMap_CitiesNames1;
        QAction *acMap_CitiesNames2;
        QAction *acMap_CitiesNames3;
        QAction *acMap_CitiesNames4;
    QAction *acMap_ShowPOIs;

    QActionGroup *acMap_GroupQuality;
        QAction *acMap_Quality1;
        QAction *acMap_Quality2;
        QAction *acMap_Quality3;
        QAction *acMap_Quality4;
        QAction *acMap_Quality5;
    QAction *acMap_Zoom_In;
    QAction *acMap_Zoom_Out;
    QAction *acMap_Zoom_Sel;
    QAction *acMap_Zoom_All;
    QAction *acMap_Go_Left;
    QAction *acMap_Go_Right;
    QAction *acMap_Go_Up;
    QAction *acMap_Go_Down;

    QAction *acOptions_Proxy;
    QAction *acOptions_Units;
    QAction *acOptions_GraphicsParams;
    QActionGroup *acOptions_GroupLanguage;
    QAction *acOptions_Lang_fr;
    QAction *acOptions_Lang_en;

    QAction *acHelp_Help;
    QAction *acHelp_APropos;
    QAction *acHelp_AProposQT;

    QAction *acVLMParam;
    QAction *acVLMParamBoat;
    QAction *acPOIinput;
	QAction *acPOIimport;
    QAction *acVLMSync;
    QAction *acVLMTest;
    QAction *acPOISave;


    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------
    QComboBox *cbDatesGrib;      // Choix de la date à afficher
    QAction *acDatesGrib_prev;
    QAction *acDatesGrib_next;


    QComboBox *cbBoatList;
    QAction   *acVLMSync_menu;

//------------------------------------------------------------------------
private:
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuMap;
    QMenu *menuOptions;
    QMenu *menuBoat;
    QMenu *menuPOI;
    QMenu *menuHelp;

    std::vector<time_t> listGribDates;

    QAction* addAction(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");

    QAction* addActionCheck(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName = "");


};

#endif
