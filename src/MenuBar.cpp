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

#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QIcon>


#include "MenuBar.h"
#include "boatVLM.h"
#include "Util.h"
#include "route.h"
#include "routage.h"
#include "Terrain.h"
#include "settings.h"
#include "ToolBar.h"
//#include "Board.h"
#include "BarrierSet.h"
#include "MapDataDrawer.h"


//===================================================================================
MenuBar::MenuBar(MainWindow *parent)
    : QMenuBar(parent)
{
    mainWindow=parent;
    Util::setFontObject(this);
    this->setAccessibleName("mainMenuQtvlm");

    //-------------------------------------
    // Menu + Actions
    //-------------------------------------
    menuFile = new QMenu(tr("QtVlm"));
    Util::setFontObject(menuFile);

        acFile_Quit = addAction(menuFile,
                    tr("Quitter"), tr("Ctrl+Q"), tr("Bye"), appFolder.value("img")+"exit.png");

        acFile_QuitNoSave = addAction(menuFile,
                                      tr("Quitter sans sauver"), "", "", appFolder.value("img")+"exit2.png");
        //acFile_QuitNoSave->setMenuRole(QAction::ApplicationSpecificRole);
        menuFile->addSeparator();
        acFile_Lock = addAction(menuFile,
                    tr("Verrouiller"), tr("Ctrl+L"), tr("Verrouiller l'envoi d'ordre a VLM"), appFolder.value("img")+"unlock.png");
        separator1=menuFile->addSeparator();
        mn_img=new QMenu(tr("Gestion des fichiers KAP"));
        Util::setFontObject(mn_img);
        acImg_Open = addAction(mn_img, tr("Ouvrir un fichier KAP"), "K", tr(""));
        mn_img->addAction(acImg_Open);
        acImg_Close = addAction(mn_img, tr("Fermer le fichier KAP"), "Shift+K", tr(""));
        mn_img->addAction(acImg_Close);
        menuFile->addMenu(mn_img);
        menuFile->addSeparator();



        acHorn=addAction(menuFile,tr("Configurer la corne de brume"),"","",tr(""));
        //acHorn->setMenuRole(QAction::ApplicationSpecificRole);
        acReplay=addAction(menuFile,tr("Rejouer l'historique des traces"),"Y","",tr(""));
        acScreenshot=addAction(menuFile,tr("Photo d'ecran"),"Ctrl+E","",tr(""));

    mainMenu.append(menuFile);

    menuView = new QMenu(tr("View"));
    Util::setFontObject(menuView);
    connect(menuView,SIGNAL(aboutToShow()),this,SLOT(slot_showViewMenu()));
    /*boardMenu = new QMenu(tr("Board"));
    menuView->addMenu(boardMenu);*/
    toolBarMenu = new QMenu(tr("ToolBar"));
    Util::setFontObject(toolBarMenu);
    menuView->addMenu(toolBarMenu);
    menuView->addSeparator();
    acOptions_SH_sAll = addAction(menuView, tr("Tout montrer"), "S", tr(""));
    acOptions_SH_hAll = addAction(menuView, tr("Tout cacher sauf les bateaux actifs"), "H", tr(""));
    menuView->addSeparator();
    QMenu * boatPoiSH = new QMenu(tr("Show/Hide boat and POI"));
    Util::setFontObject(boatPoiSH);
        acOptions_SH_Boa = addAction(boatPoiSH, tr("Centrer sur le bateau actif"), "B", tr(""));
        acKeep=addAction(boatPoiSH,tr("Conserver la position du bateau dans l'ecran lors de zoom +/-"),"Z","",tr(""));
        acKeep->setCheckable(true);
        separator2=boatPoiSH->addSeparator();
        acOptions_SH_Fla = addAction(boatPoiSH, tr("Montrer les pavillons sur la carte"), "F", tr(""));
        acOptions_SH_Fla->setCheckable(true);
        acOptions_SH_Opp = addAction(boatPoiSH, tr("Montrer les bateaux opposants"), "O", tr(""));
        acOptions_SH_Opp->setCheckable(true);
        boatPoiSH->addSeparator();
        acOptions_SH_Por = addAction(boatPoiSH, tr("Montrer les portes et WPs"), "W", tr(""));
        acOptions_SH_Por->setCheckable(true);
        acOptions_SH_Poi = addAction(boatPoiSH, tr("Montrer les POIs"), "P", tr(""));
        acOptions_SH_Poi->setCheckable(true);
        acOptions_SH_Rou = addAction(boatPoiSH, tr("Montrer les routes"), "R", tr(""));
        acOptions_SH_Rou->setCheckable(true);
        acOptions_SH_Lab = addAction(boatPoiSH, tr("Montrer les etiquettes"), "E", tr(""));
        acOptions_SH_Lab->setCheckable(true);
        acOptions_SH_barSet = addAction(boatPoiSH, tr("Montrer les barrieres"), "D", tr(""));
        acOptions_SH_barSet->setCheckable(true);
        acOptions_SH_trace = addAction(boatPoiSH, tr("Montrer les traces"), "A", tr(""));
        acOptions_SH_trace->setCheckable(true);
    menuView->addMenu(boatPoiSH);
    QMenu * compasSH = new QMenu(tr("Show/Hide compas"));
    Util::setFontObject(compasSH);
    acOptions_SH_Com = addAction(compasSH, tr("Cacher/Montrer le compas"), "C", tr(""));
    acOptions_SH_Com->setCheckable(true);
    acOptions_SH_Pol = addAction(compasSH, tr("Cacher/Montrer la polaire"), "L", tr(""));
    acOptions_SH_Pol->setCheckable(true);

    menuView->addMenu(compasSH);

    menuView->addSeparator();

    acOptions_SH_Scale = addAction(menuView, tr("Montrer l'echelle"), "", tr(""));
    acOptions_SH_Scale->setCheckable(true);

    acOptions_SH_Nig = addAction(menuView, tr("Montrer les zones de jour et nuit"), "N", tr(""));
    acOptions_SH_Nig->setCheckable(true);

    acOptions_SH_Tdb = addAction(menuView, tr("Montrer le tableau de bord"), "T", tr(""));
    acOptions_SH_Tdb->setCheckable(true);

    mainMenu.append(menuView);

    //-------------------------------------
    menuGrib = new QMenu(tr("Fichier GRIB"));
    Util::setFontObject(menuGrib);
        acFile_Open = addAction(menuGrib, tr("Ouvrir"),
                    tr("Ctrl+O"),
                    tr("Ouvrir un fichier GRIB"), appFolder.value("img")+"fileopen.png");
        acFile_Reopen = addAction(menuGrib, tr("Recharcher"),
                    "",
                    tr("Recharger le fichier GRIB actuel"), appFolder.value("img")+"fileopen.png");
        acFile_Close = addAction(menuGrib, tr("Fermer"),
                    tr("Ctrl+W"),
                    tr("Fermer"), appFolder.value("img")+"fileclose.png");
        acFile_Load_GRIB = addAction(menuGrib, tr("Telechargement"),
                    tr("Ctrl+D"),
                    tr("Telechargement"), appFolder.value("img")+"network.png");
        acFile_Load_VLM_GRIB = addAction(menuGrib, tr("Telechargement VLM"),
                    tr(""),
                    tr("Telechargement VLM"), appFolder.value("img")+"VLM_mto.png");
        acFile_Load_SAILSDOC_GRIB = addAction(menuGrib, tr("Telechargement SailsDoc"),
                                                tr(""),
                                                tr("Telechargement SailsDoc"), appFolder.value("img")+"kmail.png");
        acFile_Info_GRIB_main = addAction(menuGrib, tr("Informations sur le fichier"),
                    tr("Ctrl+I"),
                    tr("Informations sur le fichier GRIB"), appFolder.value("img")+"info.png");
        menuGrib->addSeparator();
        acFile_Open_Current = addAction(menuGrib, tr("Ouvrir un GRIB Courants"),
                    tr(""),
                    tr("Ouvrir un fichier GRIB Courants"), appFolder.value("img")+"fileopen.png");
        acFile_Close_Current = addAction(menuGrib, tr("Fermer le GRIB Courants"),
                    tr(""),
                    tr("Fermer le GRIB Courants"), appFolder.value("img")+"fileclose.png");
        acFile_Info_GRIB_current = addAction(menuGrib, tr("Informations sur le fichier"),
                    tr(""),
                    tr("Informations sur le fichier GRIB"), appFolder.value("img")+"info.png");
        menuGrib->addSeparator();


        acGrib_dialog = addAction(menuGrib, tr("Grib drawing config"),
                                  "",
                                  tr("Grib drawing config"), appFolder.value("img")+"wind.png");
        acGrib_dialog->setCheckable(true);

        menuGrib->addSeparator();
        mn_fax=new QMenu(tr("Fax meteo"));
        Util::setFontObject(mn_fax);
        acFax_Open = addAction(mn_fax, tr("Ouvrir un fax meteo"), "", tr(""));
        mn_fax->addAction(acFax_Open);
        acFax_Close = addAction(mn_fax, tr("Fermer le fax meteo"), "", tr(""));
        mn_fax->addAction(acFax_Close);
        menuGrib->addMenu(mn_fax);
        menuGrib->addSeparator();
        acCombineGrib = addAction(menuGrib,tr("Combine grib files"),"","","");

    mainMenu.append(menuGrib);

    //-------------------------------------
    menuBoat = new QMenu(tr("Bateau"));
    Util::setFontObject(menuBoat);
        acVLMParamPlayer = addAction(menuBoat,tr("Gestion des comptes"),"","","");
 #ifdef __REAL_BOAT_ONLY
        acVLMParamPlayer->setEnabled(false);
#endif
        acVLMParamBoat = addAction(menuBoat,tr("Parametres du/des bateaux"),"","","");
        acRace = addAction(menuBoat,tr("Parametres des courses"),"","","");
        acVLMSync = addAction(menuBoat,tr("VLM Sync"),"F5","","");
        acPilototo = addAction(menuBoat,tr("Pilototo"),"","","");
        acShowLog=addAction(menuBoat,tr("Historique VLM"),"Ctrl+Shift+E","",tr(""));
        acGetTrack=addAction(menuBoat,tr("Telecharger trace"),"Ctrl+Shift+T","",tr(""));
        acShowPolar=addAction(menuBoat,tr("Etudier la polaire"),"","");
    mainMenu.append(menuBoat);

    //-------------------------------------
    //Porte
    menuRoute = new QMenu(tr("Routes"));
    Util::setFontObject(menuRoute);

        acRoute_add = addAction(menuRoute,
                    tr("Creer une route"),"", "", "");
        mnRoute_delete = new QMenu(tr("Supprimer une route"));
        Util::setFontObject(mnRoute_delete);
        mnRoute_edit = new QMenu(tr("Editer une route"));
        Util::setFontObject(mnRoute_edit);
        mnRoute_export = new QMenu(tr("Exporter une route"));
        Util::setFontObject(mnRoute_export);
        menuRoute->addMenu(mnRoute_edit);
        menuRoute->addMenu(mnRoute_export);
        mnRoute_import=new QMenu(tr("Importer une route"));
        Util::setFontObject(mnRoute_import);
        acRoute_import = addAction(mnRoute_import,
                    tr("En mode VB-VMG"),"", "", "");
        acRoute_import2 = addAction(mnRoute_import,
                    tr("En mode Ortho"),"", "", "");
        acRoute_import3 = addAction(mnRoute_import,
                    tr("Depuis le pilototo VLM"),"", "", "");
        acRoute_paste = addAction(menuRoute,
                    tr("Coller une route"),"Ctrl+V", "", "");
        menuRoute->addMenu(mnRoute_import);
        acRoute_comparator= addAction(menuRoute,
                                    tr("Routes comparator"),"", "", "");
        menuRoute->addSeparator();
        menuRoute->addMenu(mnRoute_delete);
        acRouteRemove = addAction(menuRoute,tr("Supprimer des routes"),"","","");
    mainMenu.append(menuRoute);

    menuRoutage = new QMenu(tr("Routages"));
    Util::setFontObject(menuRoutage);
        acRoutage_add = addAction(menuRoutage,
                    tr("Creer un routage"),"", "", "");
        mnRoutage_delete = new QMenu(tr("Supprimer un routage"));
        Util::setFontObject(mnRoutage_delete);
        mnRoutage_edit = new QMenu(tr("Editer un routage"));
        Util::setFontObject(mnRoutage_edit);
        mnRoutage_edit->setEnabled(false);
        mnRoutage_delete->setEnabled(false);
        menuRoutage->addMenu(mnRoutage_edit);
        menuRoutage->addSeparator();
        menuRoutage->addMenu(mnRoutage_delete);
    mainMenu.append(menuRoutage);

    menuPOI = new QMenu(tr("Marques"));
    Util::setFontObject(menuPOI);
        acPOIinput = addAction(menuPOI,tr("Ajout en masse"),"","","");
        acPOISave = addAction(menuPOI,tr("Sauvegarder POIs et routes"),"Ctrl+S","","");
        acPOIRestore = addAction(menuPOI,tr("Recharger POIs et routes"),"Ctrl+R","","");
        menuPOI->addSeparator();
        QMenu *menuImportPoi = new QMenu(tr("Importer"));
        Util::setFontObject(menuImportPoi);
        acPOIimport = addAction(menuImportPoi,tr("Importer de zyGrib"),"","","");
        acPOIgeoData = addAction(menuImportPoi,tr("Importer un fichier GeoData"),"","","");
        menuPOI->addMenu(menuImportPoi);
        acPOIAdd = addAction(menuPOI,tr("Ajouter une marque"),"","","");
        acPOIRemove = addAction(menuPOI,tr("Supprimer des marques"),"","","");
        acPOIRemoveByType = addAction(menuPOI,tr("Remove marks by type..."),"","","");
        menuPOI->addSeparator();

        subMenuBarrier= new QMenu(tr("Barrier set"));
        Util::setFontObject(subMenuBarrier);
        connect(subMenuBarrier,SIGNAL(aboutToShow()),this,SLOT(slot_showBarrierMenu()));
        menuPOI->addMenu(subMenuBarrier);
        ac_addBarrierSet= addAction(subMenuBarrier,tr("Add barrier set"), tr(""), tr(""), "");
        subMenuBarrier->addSeparator();
        ac_addBarrier= addAction(subMenuBarrier,tr("Add barrier"), tr(""), tr(""), "");        
        subSubMenuEditBarrierSet= new QMenu(tr("Parameters"));
        Util::setFontObject(subSubMenuEditBarrierSet);
        subMenuBarrier->addMenu(subSubMenuEditBarrierSet);
        subSubMenuDelBarrierSet= new QMenu(tr("Delete"));
        Util::setFontObject(subSubMenuDelBarrierSet);
        subMenuBarrier->addMenu(subSubMenuDelBarrierSet);

    mainMenu.append(menuPOI);



    //-------------------------------------
    menuOptions = new QMenu(tr("Options"));
    Util::setFontObject(menuOptions);
        acOptions_Proxy = addAction(menuOptions, tr("Proxy Internet"),tr(""),tr(""),"");
        acOptions_GraphicsParams = addAction(menuOptions,
                            tr("Parametres graphiques"),tr("Ctrl+G"),tr(""),"");
        acVLMParam = addAction(menuOptions,tr("Parametres"),"","","");


        QMenu *menuMap = new QMenu(tr("Planisphere"));
        Util::setFontObject(menuMap);
        acMap_Orthodromie = addActionCheck(menuMap, tr("Distance orthodromique"), tr(""), tr(""));
        acMap_Orthodromie->setChecked(Settings::getSetting(showOrthodromie).toBool());

        menuMap->addSeparator();
        acMap_CountriesBorders = addActionCheck(menuMap, tr("Frontieres"), tr(""), tr("Afficher les frontieres"));
        acMap_CountriesBorders->setChecked(Settings::getSetting(show_countriesBorders).toBool());
        acMap_Rivers = addActionCheck(menuMap, tr("Rivieres"), tr(""), tr("Afficher les rivieres"));
        acMap_Rivers->setChecked(Settings::getSetting(show_rivers).toBool());
        acMap_CountriesNames = addActionCheck(menuMap, tr("Noms des pays"), tr(""), tr("Afficher les noms des pays"));
        acMap_CountriesNames->setChecked(Settings::getSetting(show_countriesNames).toBool());


        QMenu *menuCitiesNames = new QMenu(tr("Nom des villes"));
        Util::setFontObject(menuCitiesNames);
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
        menuOptions->addMenu(menuMap);
        setCitiesNamesLevel(Settings::getSetting(show_citiesNamesLevel).toInt());

        QMenu *menuLanguage = new QMenu(tr("Language"));
        Util::setFontObject(menuLanguage);
            acOptions_GroupLanguage = new QActionGroup(menuLanguage);
                acOptions_Lang_fr = addActionCheck(menuLanguage, tr("Francais"), tr(""), tr(""));
                acOptions_Lang_en = addActionCheck(menuLanguage, tr("English"), tr(""), tr(""));
                acOptions_Lang_es = addActionCheck(menuLanguage, tr("Spanish"), tr(""), tr(""));
                acOptions_Lang_cz = addActionCheck(menuLanguage, tr("Czech"), tr(""), tr(""));
                acOptions_GroupLanguage->addAction(acOptions_Lang_fr);
                acOptions_GroupLanguage->addAction(acOptions_Lang_en);
                acOptions_GroupLanguage->addAction(acOptions_Lang_es);
                acOptions_GroupLanguage->addAction(acOptions_Lang_cz);
        menuOptions->addMenu(menuLanguage);
        QString lang = Settings::getSetting(appLanguage).toString();
        if (lang == "fr")
            acOptions_Lang_fr->setChecked(true);
        else if (lang == "en")
            acOptions_Lang_en->setChecked(true);
        else if (lang == "cz")
            acOptions_Lang_cz->setChecked(true);
        else if (lang == "es")
            acOptions_Lang_es->setChecked(true);



#ifdef __QTVLM_WITH_TEST
        menuOptions->addSeparator();
        acVLMTest = addAction(menuOptions,"Test","","","");
        acGribInterpolation = addAction(menuOptions,"Test interpolation","","","");
#endif

        mainMenu.append(menuOptions);

    //-------------------------------------
    menuHelp = new QMenu(tr("Aide"));
    Util::setFontObject(menuHelp);
        acHelp_Help = addAction(menuHelp, tr("Aide"),"F1",tr(""),appFolder.value("img")+"help.png");
        acHelp_APropos = addAction(menuHelp, tr("A propos de qtVlm"),tr(""),tr(""),"");
        acHelp_AProposQT = addAction(menuHelp, tr("A propos de QT"),tr(""),tr(""),"");
        acHelp_Forum = addAction(menuHelp, tr("QtVlm forum"),tr(""),tr(""),"");
    mainMenu.append(menuHelp);
    foreach (QAction * act, this->actions())
    {
        setRules(act);
        Util::setFontObject(act);
    }
    acFile_Quit->setMenuRole(QAction::QuitRole);
    for(int n=0;n<mainMenu.size();++n)
        this->addMenu(mainMenu.at(n));
}

void MenuBar::setRules(QAction * act)
{
    Util::setFontObject(act);
    QMenu *menu=act->menu();
    if(menu)
    {
        foreach(QAction * subAct,menu->actions())
            setRules(subAct); //recursion
    }
    else
    {
        //qWarning()<<act->text();
        act->setMenuRole(QAction::NoRole);
    }
}

//---------------------------------------------------------
// Menu popup : bouton droit de la souris
//---------------------------------------------------------
QMenu * MenuBar::createPopupBtRight(QWidget *parent)
{
    QMenu *popup = new QMenu(parent);

    ac_CreatePOI = addAction(popup, tr("Positionner une nouvelle Marque"),tr(""),tr(""),"");
    ac_pastePOI = addAction(popup, tr("Coller une marque"),tr(""),tr(""),"");
    popup->addSeparator();
    ac_popupBarrier = new QAction(tr("Barrier not in edit mode"),popup);
    popup->addAction(ac_popupBarrier);

    popup->addSeparator();
    ac_twaLine=addAction(popup,tr("Tracer une estime TWA"),tr(""),tr(""),"");
    ac_compassLine = addAction(popup, tr("Tirer un cap"),tr(""),tr(""),"");
    popup->addSeparator();
    ac_compassCenterBoat = addAction(popup, tr("Centrer le compas sur le bateau actif"),tr(""),tr(""),"");
    ac_compassCenterBoat->setCheckable(true);
    ac_compassCenterBoat->setChecked(Settings::getSetting(compassCenterBoat).toString()=="1"?Qt::Checked:Qt::Unchecked);
    ac_compassCenterWp = addAction(popup, tr("Centrer le compas sur le WP VLM"),tr(""),tr(""),"");
    mnCompassCenterRoute=new QMenu(tr("Centrer le compass sur l'interpolation de la route"));
    popup->addMenu(mnCompassCenterRoute);
    popup->addSeparator();
    ac_centerMap = addAction(popup, tr("Centrer la carte ici"),tr(""),tr(""),"");
    ac_positScale = addAction(popup, tr("Positionner l'echelle ici"),tr(""),tr(""),"");

    ac_moveBoatSep = popup->addSeparator();
    ac_moveBoat = addAction(popup, tr("Deplacer le bateau ici"),tr(""),tr(""),"");

    ac_pasteRoute=addAction(popup,tr("Coller une route"),"","","");
//    popup->setStyle(qApp->style());
//    popup->setStyleSheet(qApp->styleSheet());
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
    action->setToolTip (statustip);
    if (iconFileName != "")
        action->setIcon(QIcon(iconFileName));
    if (menu != NULL)
        menu->addAction(action);
    Util::setFontObject(action);
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
    Util::setFontObject(action);
    return action;
}

void MenuBar::addMenuRoute(ROUTE* route)
{
    QAction *action1;
    QAction *action2;
    QAction *action3;
    QAction *action4;
    QPixmap iconI(20,10);
    iconI.fill(route->getColor());
    QIcon icon(iconI);
    action1=addAction(mnRoute_edit,route->getName(),"","","");
    connect(action1, SIGNAL(triggered()), route, SLOT(slot_edit()));
    action2=addAction(mnRoute_delete,route->getName(),"","","");
    action2->setData(QVariant(QMetaType::VoidStar, &route));
    connect(action2, SIGNAL(triggered()), my_CentralWidget, SLOT(slot_deleteRoute()));
    action3=addAction(mnRoute_export,route->getName(),"","","");
    connect(action3, SIGNAL(triggered()), route, SLOT(slot_export()));
    action4=addAction(mnCompassCenterRoute,route->getName(),"","","");
    connect(action4, SIGNAL(triggered()), route, SLOT(slot_compassFollow()));
    action1->setIcon(icon);
    action2->setIcon(icon);
    action3->setIcon(icon);
    action4->setIcon(icon);
    Util::setFontObject(action1);
    Util::setFontObject(action2);
    Util::setFontObject(action3);
    Util::setFontObject(action4);
}
QAction * MenuBar::addReleaseCompass()
{
    QAction *action;
    action=addAction(mnCompassCenterRoute,tr("Aucune"),"","","");
    Util::setFontObject(action);
    return action;
}
void MenuBar::addMenuRoutage(ROUTAGE* routage)
{
    QAction *action1;
    QAction *action2;
    QPixmap iconI(20,10);
    iconI.fill(routage->getColor());
    QIcon icon(iconI);
    action1=addAction(mnRoutage_edit,routage->getName(),"","","");
    connect(action1, SIGNAL(triggered()), routage, SLOT(slot_edit()));
    action2=addAction(mnRoutage_delete,routage->getName(),"","","");
    action2->setData(QVariant(QMetaType::VoidStar, &routage));
    connect(action2, SIGNAL(triggered()), my_CentralWidget, SLOT(slot_deleteRoutage()));
    action1->setIcon(icon);
    action2->setIcon(icon);
    Util::setFontObject(action1);
    Util::setFontObject(action2);
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


void MenuBar::slot_updateLockIcon(QString ic) {
    acFile_Lock->setIcon(QIcon(ic));
}

void MenuBar::slot_setChangeStatus(bool ,bool pilototo,bool syncBtn) {
    acPilototo->setEnabled(pilototo);
    acVLMSync->setEnabled(syncBtn);
}

void MenuBar::slot_showViewMenu(void) {
    //boardMenu->clear();
    //mainWindow->get_board()->build_showHideMenu(boardMenu);

    toolBarMenu->clear();
    mainWindow->get_toolBar()->build_showHideMenu(toolBarMenu);

    acKeep->setChecked(Settings::getSetting(keepBoatPosOnScreen).toInt()==1);
    acOptions_SH_Nig->setChecked(Settings::getSetting(showNight).toInt()==1);
    acOptions_SH_Scale->setChecked(Settings::getSetting(showScale).toInt()==1);

    acOptions_SH_Fla->setChecked(Settings::getSetting(showFlag).toInt()==1);
    acOptions_SH_Pol->setChecked(Settings::getSetting(showPolar).toInt()==1);
    acOptions_SH_Com->setChecked(Settings::getSetting(showCompass).toInt()==1);

    acOptions_SH_Opp->setChecked(Settings::getSetting(hideOpponent).toInt()==0);
    acOptions_SH_Por->setChecked(Settings::getSetting(hidePorte).toInt()==0);
    acOptions_SH_Poi->setChecked(Settings::getSetting(hidePoi).toInt()==0);
    acOptions_SH_Rou->setChecked(Settings::getSetting(hideRoute).toInt()==0);
    acOptions_SH_Lab->setChecked(Settings::getSetting(hideLabel).toInt()==0);
    acOptions_SH_barSet->setChecked(Settings::getSetting(hideBarrierSet).toInt()==0);
    acOptions_SH_trace->setChecked(Settings::getSetting(hideTrace).toInt()==0);
}

void MenuBar::slot_showBarrierMenu(void) {
    subSubMenuDelBarrierSet->clear();
    subSubMenuEditBarrierSet->clear();
    if(::barrierSetList.isEmpty()) {
        subSubMenuDelBarrierSet->setEnabled(false);
        subSubMenuEditBarrierSet->setEnabled(false);
        ac_addBarrier->setEnabled(false);
    }
    else {
        qSort(::barrierSetList.begin(),::barrierSetList.end(),BarrierSet::myLessThan);

        subSubMenuDelBarrierSet->setEnabled(true);
        subSubMenuEditBarrierSet->setEnabled(true);
        ac_addBarrier->setEnabled(true);

        QListIterator<BarrierSet*> i (::barrierSetList);
        while(i.hasNext()) {
            BarrierSet * barrierSet=i.next();
            QAction * action;
            action = addAction(subSubMenuEditBarrierSet,barrierSet->get_name(),"","","");
            connect(action,SIGNAL(triggered()),barrierSet,SLOT(slot_editBarrierSet()));
            action = addAction(subSubMenuDelBarrierSet,barrierSet->get_name(),"","","");
            connect(action,SIGNAL(triggered()),barrierSet,SLOT(slot_delBarrierSet()));
        }
    }

}


void MenuBar::setPlayerType(const int &type)
{
    bool real=type!=BOAT_VLM;
    ac_moveBoat->setVisible(real);
    ac_moveBoatSep->setVisible(real);
    acRace->setVisible(!real);
    acVLMSync->setVisible(!real);
    acPilototo->setVisible(!real);
    acFile_Lock->setEnabled(!real);
    acFile_Lock->setVisible(!real);
    separator1->setVisible(!real);
    separator2->setVisible(!real);
    acOptions_SH_Opp->setVisible(!real);
    acOptions_SH_Fla->setVisible(!real);
    acOptions_SH_Por->setVisible(!real);
    acShowLog->setVisible(!real);
    acGetTrack->setVisible(!real);
    acRoute_import3->setVisible(!real);
}

void ZeroOneActionGroup::clear(void) {
    while(!lsactions.isEmpty()) {
        QAction * act = lsactions.takeFirst();
        disconnect(act,SIGNAL(triggered(bool)),
                   this,  SLOT(slot_actionTrigerred(bool)));
    }
}

//===================================================================================
void ZeroOneActionGroup::addAction(QAction *act)
{
        lsactions.append(act);
        connect(act, SIGNAL(triggered(bool)),
                this,  SLOT(slot_actionTrigerred(bool)));
}
//------------------------------------------------------------------
void ZeroOneActionGroup::slot_actionTrigerred(bool b)
{
        QAction *act = (QAction *) sender();
        setCheckedAction(act, b);
}
//------------------------------------------------------------------
void ZeroOneActionGroup::setCheckedAction(QAction *act, bool b)
{
        for (int i=0; i<lsactions.size(); i++) {
                if (lsactions.at(i)== act) {
                        lsactions.at(i)->setChecked(b);
                }
                else {
                        lsactions.at(i)->setChecked(false);
                }
        }
        if (b)
                emit triggered( act );
        else
                emit triggered( NULL );
}


