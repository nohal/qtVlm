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

#include "MenuBar.h"
#include "Util.h"

//===================================================================================
MenuBar::MenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    //-------------------------------------
    // Menu + Actions
    //-------------------------------------
    menuFile = new QMenu(tr("Fichier"));
        acFile_Open = addAction(menuFile, tr("Ouvrir"),
                    tr("Ctrl+O"),
                    tr("Ouvrir un fichier GRIB"), "img/fileopen.png");
        acFile_Close = addAction(menuFile, tr("Fermer"),
                    tr("Ctrl+W"),
                    tr("Fermer"), "img/fileclose.png");
        menuFile->addSeparator();
        acFile_Load_GRIB = addAction(menuFile, tr("Téléchargement"),
                    tr("Ctrl+D"),
                    tr("Téléchargement"), "img/network.png");
        acFile_Info_GRIB = addAction(menuFile, tr("Informations sur le fichier"),
                    tr("Ctrl+I"),
                    tr("Informations sur le fichier GRIB"), "img/info.png");
        menuFile->addSeparator();
        acFile_Quit = addAction(menuFile,
                    tr("Quitter"), tr("Ctrl+Q"), tr("Bye"), "img/exit.png");
    addMenu(menuFile);

    //-------------------------------------
    menuView = new QMenu(tr("Données GRIB"));

        acView_WindColors = addActionCheck(menuView, tr("Carte du vent"), tr(""),
                    tr(""));
        acView_RainColors = addActionCheck(menuView, tr("Carte des précipitations"), tr(""),
                    tr(""));
        acView_CloudColors = addActionCheck(menuView, tr("Couverture nuageuse"), tr(""),
                    tr(""));
        acView_HumidColors = addActionCheck(menuView, tr("Carte de l'humidité relative"), tr(""),
                    tr(""));

        //--------------------------------
        menuView->addSeparator();
        acView_ColorMapSmooth = addActionCheck(menuView, tr("Dégradés de couleurs"), tr(""),
                    tr(""));
        acView_WindArrow = addActionCheck(menuView, tr("Flèches du vent"), tr(""),
                    tr("Afficher les flèches de direction du vent"));
        acView_Barbules = addActionCheck(menuView, tr("Barbules"), tr(""),
                    tr("Afficher les barbules sur les flèches de vent"));
        //--------------------------------
        menuView->addSeparator();
        acView_TemperatureLabels = addActionCheck(menuView, tr("Température"), tr("Ctrl+T"),
                    tr(""));
        //--------------------------------
        menuView->addSeparator();
        acView_Isobars = addActionCheck(menuView, tr("Isobares"), tr(""),
                            tr("Afficher les isobares"));
            QMenu *menuIsobarsStep = new QMenu(tr("Espacement (hPa)"));
            acView_GroupIsobarsStep = new QActionGroup(menuIsobarsStep);
                acView_IsobarsStep1 = addActionCheck(menuIsobarsStep, tr("1"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep2 = addActionCheck(menuIsobarsStep, tr("2"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep3 = addActionCheck(menuIsobarsStep, tr("3"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep4 = addActionCheck(menuIsobarsStep, tr("4"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep5 = addActionCheck(menuIsobarsStep, tr("5"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep6 = addActionCheck(menuIsobarsStep, tr("6"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep8 = addActionCheck(menuIsobarsStep, tr("8"), tr(""), tr("Espacement des isobares"));
                acView_IsobarsStep10 = addActionCheck(menuIsobarsStep, tr("10"), tr(""), tr("Espacement des isobares"));
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep1);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep2);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep3);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep4);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep5);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep6);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep8);
                acView_GroupIsobarsStep->addAction(acView_IsobarsStep10);
            menuView->addMenu(menuIsobarsStep);
        acView_IsobarsLabels = addActionCheck(menuView, tr("Etiquettes des isobares"), tr(""),
                            tr("Afficher les étiquettes des isobares"));
        acView_PressureMinMax = addActionCheck(menuView, tr("Pression Mini(L) Maxi(H)"), tr(""),
                            tr("Afficher les points de pression mini et maxi"));
        menuView->addSeparator();
        acView_GribGrid = addActionCheck(menuView, tr("Grille GRIB"), tr(""),
                            tr("Montrer les positions des données du fichier GRIB"));
    addMenu(menuView);

    //-------------------------------------
    menuMap = new QMenu(tr("Planisphère"));
        acMap_GroupQuality = new QActionGroup(menuMap);
            acMap_Quality1 = addActionCheck(menuMap, tr("Qualité 1"), tr(""), tr("Niveau de détail de la carte"));
            acMap_Quality2 = addActionCheck(menuMap, tr("Qualité 2"), tr(""), tr("Niveau de détail de la carte"));
            acMap_Quality3 = addActionCheck(menuMap, tr("Qualité 3"), tr(""), tr("Niveau de détail de la carte"));
            acMap_Quality4 = addActionCheck(menuMap, tr("Qualité 4"), tr(""), tr("Niveau de détail de la carte"));
            acMap_Quality5 = addActionCheck(menuMap, tr("Qualité 5"), tr(""), tr("Niveau de détail de la carte"));
            acMap_GroupQuality->addAction(acMap_Quality1);
            acMap_GroupQuality->addAction(acMap_Quality2);
            acMap_GroupQuality->addAction(acMap_Quality3);
            acMap_GroupQuality->addAction(acMap_Quality4);
            acMap_GroupQuality->addAction(acMap_Quality5);

        menuMap->addSeparator();
        acMap_Orthodromie = addActionCheck(menuMap, tr("Distance orthodromique"), tr(""), tr(""));

     menuMap->addSeparator();
        acMap_CountriesBorders = addActionCheck(menuMap, tr("Frontières"), tr(""), tr("Afficher les frontières"));
        acMap_Rivers = addActionCheck(menuMap, tr("Rivières"), tr(""), tr("Afficher les rivières"));
        acMap_CountriesNames = addActionCheck(menuMap, tr("Noms des pays"), tr(""), tr("Afficher les noms des pays"));

        QMenu *menuCitiesNames = new QMenu(tr("Nom des villes"));
        acMap_GroupCitiesNames = new QActionGroup(menuMap);
            acMap_CitiesNames0 = addActionCheck(menuCitiesNames, tr("Aucun"), tr(""), tr(""));
            acMap_CitiesNames1 = addActionCheck(menuCitiesNames, tr("Niveau 1"), tr(""), tr(""));
            acMap_CitiesNames2 = addActionCheck(menuCitiesNames, tr("Niveau 2"), tr(""), tr(""));
            acMap_CitiesNames3 = addActionCheck(menuCitiesNames, tr("Niveau 3"), tr(""), tr(""));
            acMap_CitiesNames4 = addActionCheck(menuCitiesNames, tr("Niveau 4"), tr(""), tr(""));
            acMap_GroupCitiesNames->addAction(acMap_CitiesNames0);
            acMap_GroupCitiesNames->addAction(acMap_CitiesNames1);
            acMap_GroupCitiesNames->addAction(acMap_CitiesNames2);
            acMap_GroupCitiesNames->addAction(acMap_CitiesNames3);
            acMap_GroupCitiesNames->addAction(acMap_CitiesNames4);
            menuMap->addMenu(menuCitiesNames);

        acMap_ShowPOIs = addActionCheck(menuMap, tr("Points d'intérêt"), tr(""), tr("Afficher les Points d'intérêt"));

        menuMap->addSeparator();
        acMap_Zoom_In = addAction(menuMap,  tr("Augmenter l'échelle de la carte"), tr(""),
                        tr("Augmenter l'échelle de la carte"), "img/viewmag+.png");
        acMap_Zoom_Out = addAction(menuMap, tr("Diminuer l'échelle de la carte"), tr(""),
                        tr("Diminuer l'échelle de la carte"), "img/viewmag-.png");
        acMap_Zoom_Sel = addAction(menuMap,
                        tr("Zoom (sélection ou fichier Grib)"),
                        tr("Ctrl+Z"),
                        tr("Zoomer sur la zone sélectionnée ou sur la surface du fichier Grib"),
                        "img/viewmagfit.png");
        acMap_Zoom_All = addAction(menuMap, tr("Afficher la carte entière"), tr(""),
                        tr("Afficher la carte entière"), "img/viewmag1.png");
        menuMap->addSeparator();
        acMap_Go_Left = addAction(menuMap,  tr("Vers la gauche"), tr("LEFT"),
                        tr("Déplacement"), "img/back.png");
        acMap_Go_Right = addAction(menuMap,  tr("Vers la droite"), tr("RIGHT"),
                        tr("Déplacement"), "img/forward.png");
        acMap_Go_Up   = addAction(menuMap,  tr("Vers le haut"), tr("UP"),
                        tr("Déplacement"), "img/up.png");
        acMap_Go_Down = addAction(menuMap,  tr("Vers le bas"), tr("DOWN"),
                        tr("Déplacement"), "img/down.png");

    addMenu(menuMap);

    //-------------------------------------
    menuOptions = new QMenu(tr("Options"));
        acOptions_Proxy = addAction(menuOptions, tr("Proxy Internet"),tr(""),tr(""),"");
        acOptions_Units = addAction(menuOptions, tr("Unités"),tr("Ctrl+U"),tr(""),"");
        acOptions_GraphicsParams = addAction(menuOptions,
                            tr("Paramètres graphiques"),tr("Ctrl+G"),tr(""),"");
        acVLMParam = addAction(menuOptions,tr("Param�tres VLM"),"","","");

        QMenu *menuLanguage = new QMenu(tr("Language"));
            acOptions_GroupLanguage = new QActionGroup(menuLanguage);
                acOptions_Lang_fr = addActionCheck(menuLanguage, tr("Français"), tr(""), tr(""));
                acOptions_Lang_en = addActionCheck(menuLanguage, tr("English"), tr(""), tr(""));
                acOptions_GroupLanguage->addAction(acOptions_Lang_fr);
                acOptions_GroupLanguage->addAction(acOptions_Lang_en);
        menuOptions->addMenu(menuLanguage);

    addMenu(menuOptions);


    menuBoat = new QMenu(tr("Bateau"));
        acVLMParamBoat = addAction(menuBoat,tr("Boat settings"),"","","");
        acVLMSync = addAction(menuBoat,tr("Boat sync"),"","","");
        acPilototo = addAction(menuBoat,tr("Pilototo"),"","","");
    addMenu(menuBoat);

    menuPOI = new QMenu(tr("POI"));
        acPOIinput = addAction(menuPOI,tr("POI en masse"),"","","");
        acPOISave = addAction(menuPOI,tr("Sauvegarder"),"","","");

        QMenu *menuImportPoi = new QMenu(tr("Importer"));
        //acPOI_import = new QActionGroup(menuPOI);
        acPOIimport = addAction(menuImportPoi,tr("Importer de zyGrib"),"","","");

        menuPOI->addMenu(menuImportPoi);




#if 0
        acVLMTest = addAction(menuPOI,"Test","","","");
#else
        acVLMTest = NULL;
#endif
    addMenu(menuPOI);


    //-------------------------------------
    menuHelp = new QMenu(tr("Aide"));
        acHelp_Help = addAction(menuHelp, tr("Aide"),tr("Ctrl+H"),tr(""),"img/help.png");
        acHelp_APropos = addAction(menuHelp, tr("A propos de qtVlm"),tr(""),tr(""),"");
        acHelp_AProposQT = addAction(menuHelp, tr("A propos de QT"),tr(""),tr(""),"");
    addMenu(menuHelp);


    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------
    acDatesGrib_prev = addAction( NULL,
            tr("Prévision précédente [page préc]"),tr("PgUp"),tr(""),"img/1leftarrow.png");
    acDatesGrib_next = addAction( NULL,
            tr("Prévision suivante [page suiv]"),tr("PgDown"),tr(""),"img/1rightarrow.png");

    cbDatesGrib = new QComboBox();
    cbDatesGrib->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    QFontInfo finfo = cbDatesGrib->fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setStretch(QFont::SemiCondensed);
    cbDatesGrib->setFont(font);



    cbBoatList = new QComboBox();
    cbBoatList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    finfo = cbBoatList->fontInfo();
    QFont font2("", finfo.pointSize(), QFont::Normal, false);
    font2.setStyleHint(QFont::TypeWriter);
    font2.setStretch(QFont::SemiCondensed);
    cbBoatList->setFont(font2);
}



//---------------------------------------------------------
// Menu popup : bouton droit de la souris
//---------------------------------------------------------
QMenu * MenuBar::createPopupBtRight(QWidget *parent)
{
    QMenu *popup = new QMenu(parent);

    ac_OpenMeteotable = addAction(popup, tr("Météotable"),tr(""),tr(""),"");
    ac_CreatePOI = addAction(popup, tr("Marquer un Point d'Intérêt"),tr(""),tr(""),"");
    ac_pastePOI = addAction(popup, tr("Paste un POI"),tr(""),tr(""),"");
    ac_delPOIs = addAction(popup, tr("Effacer les POI"),tr(""),tr(""),"");

    return popup;
}

//===================================================================================
QAction* MenuBar::addAction(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName)
{
    QAction *action;
    action = new QAction(title, menu);
    action->setShortcut  (shortcut);
    action->setShortcutContext (Qt::ApplicationShortcut);
    action->setStatusTip (statustip);
    if (iconFileName != "")
        action->setIcon(QIcon(iconFileName));
    if (menu != NULL)
        menu->addAction(action);
    return action;
}
//-------------------------------------------------
QAction* MenuBar::addActionCheck(QWidget *menu,
                    QString title, QString shortcut, QString statustip,
                    QString iconFileName)
{
    QAction *action;
    action = addAction(menu, title, shortcut, statustip, iconFileName);
    action->setCheckable  (true);
    return action;
}

//-------------------------------------------------
void MenuBar::setQuality(int q) {
    switch (q) {
        case 0: acMap_Quality1->setChecked(true); break;
        case 1: acMap_Quality2->setChecked(true); break;
        case 2: acMap_Quality3->setChecked(true); break;
        case 3: acMap_Quality4->setChecked(true); break;
        case 4: acMap_Quality5->setChecked(true); break;
    }
}
//-------------------------------------------------
void MenuBar::setIsobarsStep(int step) {
    switch (step) {
        case 1: acView_IsobarsStep1->setChecked(true); break;
        case 2: acView_IsobarsStep2->setChecked(true); break;
        case 3: acView_IsobarsStep3->setChecked(true); break;
        case 4: acView_IsobarsStep4->setChecked(true); break;
        case 5: acView_IsobarsStep5->setChecked(true); break;
        case 6: acView_IsobarsStep6->setChecked(true); break;
        case 8: acView_IsobarsStep8->setChecked(true); break;
        case 10: acView_IsobarsStep10->setChecked(true); break;
    }
}
//-------------------------------------------------
void MenuBar::setCitiesNamesLevel(int level) {
    switch (level) {
        case 0: acMap_CitiesNames0->setChecked(true); break;
        case 1: acMap_CitiesNames1->setChecked(true); break;
        case 2: acMap_CitiesNames2->setChecked(true); break;
        case 3: acMap_CitiesNames3->setChecked(true); break;
        case 4: acMap_CitiesNames4->setChecked(true); break;
    }
}

//------------------------------------------------------------
// Génère la liste des dates des Records du fichier GRIB
void MenuBar::updateListeDates(std::set<time_t> *setDates)
{
    listGribDates.clear();
    // Construit un vector à partir du set (plus pratique)
    std::set<time_t>::iterator its;
    for (its=setDates->begin(); its!=setDates->end(); its++) {
        listGribDates.push_back(*its);
    }

    // Met à jour les item du QComboBox
    while (cbDatesGrib->count() > 0) {
        cbDatesGrib->removeItem(0);
    }
    std::vector<time_t>::iterator it;
    for (it=listGribDates.begin(); it!=listGribDates.end(); it++) {
        time_t tps = *it;
       QString str = Util::formatDateTimeLong(tps);
       //printf("%s\n", qPrintable(str));
        cbDatesGrib->addItem(str);
    }
}
//------------------------------------------------------------
time_t  MenuBar::getDateGribById(int id)
{
    if (listGribDates.size() > (uint)id)
        return listGribDates[id];
    else
        return (time_t)0;
}

//------------- VLM part

void MenuBar::getBoatList(QList<boatAccount*> & acc_list)
{
    while (cbBoatList->count() > 0)
        cbBoatList->removeItem(0);

    QListIterator<boatAccount*> i (acc_list);

    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        if(acc->getStatus())
        {
            if(acc->getAliasState())
                cbBoatList->addItem(acc->getAlias() + "(" + acc->getLogin() + ")");
            else
                cbBoatList->addItem(acc->getLogin());
        }
    }
}







