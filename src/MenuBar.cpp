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
    menuFile = new QMenu(tr("QtVlm"));

        acFile_Quit = addAction(menuFile,
                    tr("Quitter"), tr("Ctrl+Q"), tr("Bye"), "img/exit.png");
    addMenu(menuFile);

    //-------------------------------------
    menuView = new QMenu(tr("Fichier GRIB"));
        acFile_Open = addAction(menuView, tr("Ouvrir"),
                    tr("Ctrl+O"),
                    tr("Ouvrir un fichier GRIB"), "img/fileopen.png");
        acFile_Close = addAction(menuView, tr("Fermer"),
                    tr("Ctrl+W"),
                    tr("Fermer"), "img/fileclose.png");        
        acFile_Load_GRIB = addAction(menuView, tr("Téléchargement"),
                    tr("Ctrl+D"),
                    tr("Téléchargement"), "img/network.png");
        menuView->addSeparator();
        acFile_Info_GRIB = addAction(menuView, tr("Informations sur le fichier"),
                    tr("Ctrl+I"),
                    tr("Informations sur le fichier GRIB"), "img/info.png");
        menuView->addSeparator();
        acView_WindColors = addActionCheck(menuView, tr("Carte du vent"), tr(""),
                    tr(""));
        acView_ColorMapSmooth = addActionCheck(menuView, tr("Dégradés de couleurs"), tr(""),
                    tr(""));
        acView_WindArrow = addActionCheck(menuView, tr("Flèches du vent"), tr(""),
                    tr("Afficher les flèches de direction du vent"));
        acView_Barbules = addActionCheck(menuView, tr("Barbules"), tr(""),
                    tr("Afficher les barbules sur les flèches de vent"));
    addMenu(menuView);

    //-------------------------------------
    menuBoat = new QMenu(tr("Bateau"));
        acVLMParamBoat = addAction(menuBoat,tr("Paramètres des bateaux"),"","","");
        acRace = addAction(menuBoat,tr("Paramètres des courses"),"","","");
        acVLMSync = addAction(menuBoat,tr("VLM Sync"),"","","");
        acPilototo = addAction(menuBoat,tr("Pilototo"),"","","");
    addMenu(menuBoat);

    //-------------------------------------
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
    menuOptions = new QMenu(tr("Options"));
        acOptions_Proxy = addAction(menuOptions, tr("Proxy Internet"),tr(""),tr(""),"");
        acOptions_Units = addAction(menuOptions, tr("Unités"),tr("Ctrl+U"),tr(""),"");
        acOptions_GraphicsParams = addAction(menuOptions,
                            tr("Paramètres graphiques"),tr("Ctrl+G"),tr(""),"");
        acVLMParam = addAction(menuOptions,tr("Paramètres VLM"),"","","");


        QMenu *menuMap = new QMenu(tr("Planisphère"));
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
        menuOptions->addMenu(menuMap);

        QMenu *menuLanguage = new QMenu(tr("Language"));
            acOptions_GroupLanguage = new QActionGroup(menuLanguage);
                acOptions_Lang_fr = addActionCheck(menuLanguage, tr("Français"), tr(""), tr(""));
                acOptions_Lang_en = addActionCheck(menuLanguage, tr("English"), tr(""), tr(""));
                acOptions_GroupLanguage->addAction(acOptions_Lang_fr);
                acOptions_GroupLanguage->addAction(acOptions_Lang_en);
        menuOptions->addMenu(menuLanguage);

    addMenu(menuOptions);

    //-------------------------------------
    menuHelp = new QMenu(tr("Aide"));
        acHelp_Help = addAction(menuHelp, tr("Aide"),tr("Ctrl+H"),tr(""),"img/help.png");
        acHelp_APropos = addAction(menuHelp, tr("A propos de qtVlm"),tr(""),tr(""),"");
        acHelp_AProposQT = addAction(menuHelp, tr("A propos de QT"),tr(""),tr(""),"");
    addMenu(menuHelp);


    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------
    acMap_Zoom_In = addAction(NULL,  tr("Augmenter l'échelle de la carte"), tr(""),
                              tr("Augmenter l'échelle de la carte"), "img/viewmag+.png");
    acMap_Zoom_Out = addAction(NULL, tr("Diminuer l'échelle de la carte"), tr(""),
                               tr("Diminuer l'échelle de la carte"), "img/viewmag-.png");
    acMap_Zoom_Sel = addAction(NULL,
                               tr("Zoom (sélection ou fichier Grib)"),
                               tr("Ctrl+Z"),
                               tr("Zoomer sur la zone sélectionnée ou sur la surface du fichier Grib"),
                               "img/viewmagfit.png");
    acMap_Zoom_All = addAction(NULL, tr("Afficher la carte entière"), tr(""),
                               tr("Afficher la carte entière"), "img/viewmag1.png");

    acDatesGrib_prev = addAction( NULL,
            tr("Prévision précédente [page préc]"),tr("PgUp"),tr(""),"img/1leftarrow.png");
    acDatesGrib_next = addAction( NULL,
            tr("Prévision suivante [page suiv]"),tr("PgDown"),tr(""),"img/1rightarrow.png");

    datesGrib_now = new QPushButton(tr("Now"));
    datesGrib_sel = new QPushButton(tr("Select"));

    cbGribStep = new QComboBox();

    QFontInfo finfo = cbGribStep->fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setStretch(QFont::SemiCondensed);

    cbGribStep->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cbGribStep->setFont(font);
    cbGribStep->addItem(tr("15 m"));
    cbGribStep->addItem(tr("30 m"));
    cbGribStep->addItem(tr("1 h"));
    cbGribStep->addItem(tr("2 h"));
    cbGribStep->addItem(tr("3 h"));
    cbGribStep->addItem(tr("6 h"));
    cbGribStep->addItem(tr("12 h"));


    boatList = new QComboBox();
    boatList->setSizeAdjustPolicy(QComboBox::AdjustToContents);   
    boatList->setFont(font);

}



//---------------------------------------------------------
// Menu popup : bouton droit de la souris
//---------------------------------------------------------
QMenu * MenuBar::createPopupBtRight(QWidget *parent)
{
    QMenu *popup = new QMenu(parent);

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
#if 0
void MenuBar::updateListeDates(std::set<time_t> *setDates)
{
    listGribDates.clear();
    // Construit un vector �  partir du set (plus pratique)
    std::set<time_t>::iterator its;
    for (its=setDates->begin(); its!=setDates->end(); its++) {
        listGribDates.push_back(*its);
    }

    // Met �  jour les item du QComboBox
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

void MenuBar::clearListeDates(void)
{
    listGribDates.clear();
    cbDatesGrib->clear();
}


//------------------------------------------------------------
time_t  MenuBar::getDateGribById(int id)
{
    if (listGribDates.size() > (uint)id)
        return listGribDates[id];
    else
        return (time_t)0;
}

int MenuBar::getNearestDateGrib(time_t tm)
{
    int id=0;
    if(listGribDates[0]>tm) return 0;
    while((uint)id<listGribDates.size() && listGribDates[id] < tm) id++;
    return id-1;
}
#endif
//------------------------------------------------------------

void MenuBar::updateBoatList(QList<boatAccount*> & acc_list)
{
    //qWarning() << "Boat list cnt " << boatList->count();
    while(boatList->count())
        boatList->removeItem(0);
    //boatList->clear();

    QListIterator<boatAccount*> i (acc_list);

    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        //qWarning() << "Boat: " << acc->getLogin();
        if(acc->getStatus())
        {
            //qWarning() << "Adding it";
            if(acc->getAliasState())
                boatList->addItem(acc->getAlias() + "(" + acc->getLogin() + ")");
            else
                boatList->addItem(acc->getLogin());
        }
    }

    //qWarning() << "Boat list cnt after: " << boatList->count() << " Status " << boatList->isEnabled();
}

void MenuBar::setSelectedBoatIndex(int index)
{
    boatList->setCurrentIndex(index);
    //qWarning() << "Current index: " << index << " " << boatList->itemText(index);
}
