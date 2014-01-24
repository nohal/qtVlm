/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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
***********************************************************************/

#include <QMenu>
#include <QDebug>

#include "MainWindow.h"
#include "mycentralwidget.h"
#include "ToolBar.h"
#include "settings.h"
#include "Terrain.h"
#include "boatVLM.h"
#include "Util.h"
#include <QDesktopWidget>
//#include "Board.h"

/**********************************************************************/
/*                         Gen fct                                    */
/**********************************************************************/

ToolBar::ToolBar(MainWindow *mainWindow)
{
    /*********************/
    /* init toolbar      */
    /*********************/
    this->mainWindow=mainWindow;
    centralWidget = mainWindow->getMy_centralWidget();
    connect(centralWidget,SIGNAL(geometryChanged()),this,SLOT(manageToolbarBreak()));
//    double ppi=QApplication::desktop()->physicalDpiX();
//    int s=24*ppi/72;
//    iconSize=QSize(s,s);
    iconSize=QToolBar().iconSize();
    //qWarning()<<"iconSize="<<iconSize;
    QPixmap add("img/add.png");
#ifdef __ANDROID__
    add=add.scaled(iconSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
#endif
    add.save("img/addResized.png");

//    miscToolBar=new MyToolBar("Misc",tr("Misc"),this,mainWindow);
//    toolBarList.append(miscToolBar);

    gribToolBar=new MyToolBar("Grib",tr("Grib"),this,mainWindow);
    toolBarList.append(gribToolBar);

    mapToolBar=new MyToolBar("Selection",tr("Selection"),this,mainWindow);
    toolBarList.append(mapToolBar);

    estimeToolBar=new MyToolBar("Estime",tr("Estime"),this,mainWindow);
    toolBarList.append(estimeToolBar);

    boatToolBar=new MyToolBar("Boat",tr("Boat"),this,mainWindow);
    toolBarList.append(boatToolBar);
    barrierToolBar=new MyToolBar("BarrierSet",tr("Barrier Set"),this,mainWindow);
    toolBarList.append(barrierToolBar);

    /* adding all toolBar to mainWindow dock */
    for(int i=0;i<toolBarList.count();++i)
        mainWindow->addToolBar(Qt::TopToolBarArea,toolBarList.at(i));

    /* font */
    QFontInfo finfo = gribToolBar->fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setStretch(QFont::SemiCondensed);

    /*********************/
    /* init items        */
    /*********************/

    /* Misc toolBar */
//    acQuit = init_Action(tr("Quitter"), "", tr("Bye"), appFolder.value("img")+"exit.png",miscToolBar);
//    miscToolBar->addAction(acQuit);

    /* Grib toolBar */
    gribDwnld = new QToolButton(gribToolBar);
    mySetIcon(gribDwnld,QString(appFolder.value("img")+"wind.png"));
    gribDwnld->setToolTip(tr("Hold to select download method"));
    gribSubMenu = new QMenu(gribDwnld);
    acWindZygrib = init_Action(tr("Telechargement zyGrib"),tr(""),tr(""),appFolder.value("img")+ "network.png",gribToolBar);
    acWindVlm = init_Action(tr("Telechargement VLM"),tr(""),tr(""), appFolder.value("img")+"VLM_mto.png",gribToolBar);
    acWindSailsDoc = init_Action(tr("Telechargement SailsDoc"),tr(""),tr(""),appFolder.value("img")+ "kmail.png",gribToolBar);
    acOpenGrib = init_Action(tr("Open a grib"),tr(""),tr(""),appFolder.value("img")+ "fileopen.png",gribToolBar);
    acWindZygrib->setMenuRole(QAction::NoRole);
    acWindVlm->setMenuRole(QAction::NoRole);
    acWindSailsDoc->setMenuRole(QAction::NoRole);
    gribSubMenu->addAction(acWindZygrib);
    gribSubMenu->addAction(acWindVlm);
    gribSubMenu->addAction(acWindSailsDoc);
    //gribSubMenu->addAction(acOpenGrib);
    gribDwnld->setMenu(gribSubMenu);

    update_gribDownloadBtn();

    acDatesGrib_prev = init_Action(tr("Prevision precedente [page prec]"),
                                   tr("PgUp"),tr(""),appFolder.value("img")+"1leftarrow.png",gribToolBar);
    acDatesGrib_next = init_Action(tr("Prevision suivante [page suiv]"),
                                   tr("PgDown"),tr(""),appFolder.value("img")+"1rightarrow.png",gribToolBar);
    acGrib_play = init_Action(tr("Animation du grib"),tr(""),tr(""),appFolder.value("img")+"player_play.png",gribToolBar);

    datesGrib_now = init_Action(tr("Now"),tr(""),tr(""),appFolder.value("img")+"now.png",gribToolBar);
    datesGrib_sel = init_Action(tr("Select date"),tr(""),tr(""),appFolder.value("img")+"clock.png",gribToolBar);
    acGrib_dialog = init_Action(tr("Grib drawing config"),tr(""),
                              tr("Grib drawing config"),appFolder.value("img")+"wind.png",gribToolBar);
    acGrib_dialog->setCheckable(true);
    cbGribStep = new QComboBox(gribToolBar);
    cbGribStep->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cbGribStep->setFont(font);
    cbGribStep->addItem(tr("5 m"),300);
    cbGribStep->addItem(tr("15 m"),900);
    cbGribStep->addItem(tr("30 m"),1800);
    cbGribStep->addItem(tr("1 h"),3600);
    cbGribStep->addItem(tr("2 h"),7200);
    cbGribStep->addItem(tr("3 h"),10800);
    cbGribStep->addItem(tr("6 h"),21600);
    cbGribStep->addItem(tr("12 h"),43200);
    //FontManagement::setFontDialog(cbGribStep);
    cbGribStep->setCurrentIndex(Settings::getSetting(gribDateStep).toInt());

    gribToolBar->addWidget(gribDwnld);
    gribToolBar->addAction(acOpenGrib);
    gribToolBar->addSeparator();
    gribToolBar->addAction(datesGrib_sel);
    gribToolBar->addAction(datesGrib_now);
    gribToolBar->addSeparator();
    gribToolBar->addAction(acGrib_dialog);
    gribToolBar->addSeparator();
    gribToolBar->addAction(acDatesGrib_prev);
    gribToolBar->addWidget(cbGribStep);    
    gribToolBar->addAction(acDatesGrib_next);
    gribToolBar->addAction(acGrib_play);

    /* Map toolBar */

    acMap_Zoom_In = init_Action(tr("Augmenter l'echelle de la carte"), tr(""),
                              tr("Augmenter l'echelle de la carte"), appFolder.value("img")+"viewmag+.png",mapToolBar);
    acMap_Zoom_Out = init_Action( tr("Diminuer l'echelle de la carte"), tr(""),
                               tr("Diminuer l'echelle de la carte"), appFolder.value("img")+"viewmag-.png",mapToolBar);
    acMap_Zoom_Sel = init_Action(tr("Zoom (selection ou fichier Grib)"),
                               tr("Ctrl+Z"),tr("Zoomer sur la zone selectionnee ou sur la surface du fichier Grib"),
                               appFolder.value("img")+"viewmagfit.png",mapToolBar);
    acMap_Zoom_All = init_Action( tr("Afficher la carte entiere"), tr(""),
                               tr("Afficher la carte entiere"), appFolder.value("img")+"viewmag1.png",mapToolBar);

    selectionMode = init_Action(tr("Selection"),tr(""),tr(""),appFolder.value("img")+"selection.png",mapToolBar);
    selectionMode->setCheckable(true);
    magnify = init_Action(tr("Loupe"),tr(""),tr(""),appFolder.value("img")+"magnify.png",mapToolBar);
    magnify->setCheckable(true);
    mapToolBar->addAction(acMap_Zoom_In);
    mapToolBar->addAction(acMap_Zoom_Out);
    mapToolBar->addAction(acMap_Zoom_Sel);
    mapToolBar->addAction(acMap_Zoom_All);
    mapToolBar->addSeparator();
    mapToolBar->addAction(selectionMode);
    mapToolBar->addSeparator();
    mapToolBar->addAction(magnify);

    /* Estime toolBar */
    lbEstime=new QLabel(tr("Estime"));
    spnEstime=new QSpinBox(estimeToolBar);
    spnEstime->setMaximum(999);
    spnEstime->setMinimum(1);
    spnEstime->setAlignment(Qt::AlignRight);
    cbEstime=new QComboBox(estimeToolBar);
    cbEstime->addItem(tr("mins"));
    cbEstime->addItem(tr("vacs"));
    cbEstime->addItem(tr("NM"));
    slot_loadEstimeParam();
    chkEstime = new QCheckBox(estimeToolBar);
    chkEstime->setToolTip(tr("Si cette option est cochee<br>l'estime calcule la vitesse du bateau<br>a la prochaine vac.<br>Sinon elle utilise la vitesse du bateau<br>telle que donnee par VLM"));
    chkEstime->setChecked(Settings::getSetting(startSpeedEstime).toInt()==1);

    estimeToolBar->addWidget(lbEstime);
    estimeToolBar->addWidget(spnEstime);
    estimeToolBar->addWidget(cbEstime);
    estimeToolBar->addWidget(chkEstime);

    /* Boat toolBar */
    acLock=init_Action(tr("Verrouiller"), "", tr("Verrouiller l'envoi d'ordre a VLM"), appFolder.value("img")+"unlock.png",boatToolBar);
    boatList = new QComboBox();
    boatList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    boatToolBar->addAction(acLock);
    boatToolBar->addWidget((boatList));

    /* BarrierSet toolBar */
    barrierAdd = init_Action(tr("Add Barrier"),tr(""),tr(""),appFolder.value("img")+"add_barrier.png",barrierToolBar);
    barrierAdd->setCheckable(true);

    barrierToolBar->addAction(barrierAdd);

    /*********************/
    /* init signal/slots */
    /*********************/

    /* Misc toolBar */
    //connect(acQuit, SIGNAL(triggered()), mainWindow, SLOT(slotFile_Quit()));

    /* Grib ToolBar */
    connect(gribDwnld,SIGNAL(clicked()),this,SLOT(slot_gribDwnld()));
    connect(acWindZygrib,SIGNAL(triggered()),this,SLOT(slot_gribZygrib()));
    connect(acWindVlm,SIGNAL(triggered()),this,SLOT(slot_gribVlm()));
    connect(acWindSailsDoc,SIGNAL(triggered()),this,SLOT(slot_gribSailsDoc()));
    connect(acOpenGrib,SIGNAL(triggered()),mainWindow,SLOT(slotFile_Open()));

    connect(this, SIGNAL(gribZygrib()), centralWidget, SLOT(slot_fileLoad_GRIB()));
    connect(this, SIGNAL(gribVlm()), mainWindow, SLOT(slotLoadVLMGrib()));
    connect(this, SIGNAL(gribSailsDoc()), centralWidget, SLOT(slotLoadSailsDocGrib()));

    connect(cbGribStep, SIGNAL(activated(int)),mainWindow, SLOT(slotDateStepChanged(int)));
    connect(datesGrib_now, SIGNAL(triggered()),mainWindow, SLOT(slotDateGribChanged_now()));
    connect(acGrib_dialog,SIGNAL(triggered()),centralWidget, SLOT(slot_gribDialog()));
    connect(datesGrib_sel, SIGNAL(triggered()),mainWindow, SLOT(slotDateGribChanged_sel()));
    connect(acDatesGrib_next, SIGNAL(triggered()),mainWindow, SLOT(slotDateGribChanged_next()));
    connect(acDatesGrib_prev, SIGNAL(triggered()),mainWindow, SLOT(slotDateGribChanged_prev()));
    connect(acGrib_play,SIGNAL(triggered()),this,SLOT(slot_gribPlay()));

    /* Map ToolBar */
    connect(acMap_Zoom_In, SIGNAL(triggered()),centralWidget,  SLOT(slot_Zoom_In()));
    connect(acMap_Zoom_Out, SIGNAL(triggered()),centralWidget,  SLOT(slot_Zoom_Out()));
    connect(acMap_Zoom_Sel, SIGNAL(triggered()),centralWidget,  SLOT(slot_Zoom_Sel()));
    connect(acMap_Zoom_All, SIGNAL(triggered()),centralWidget,  SLOT(slot_Zoom_All()));
    connect(selectionMode,SIGNAL(triggered()),centralWidget,SLOT(slot_selectionTool()));
    connect(magnify,SIGNAL(triggered()),centralWidget,SLOT(slot_magnify()));

    /* Estime ToolBar */
    connect(spnEstime, SIGNAL(valueChanged(int)),this, SLOT(slot_estimeValueChanged(int)));
    connect(cbEstime,SIGNAL(currentIndexChanged(int)),this,SLOT(slot_estimeTypeChanged(int)));
    connect(chkEstime,SIGNAL(stateChanged(int)),this,SLOT(slot_estimeStartChanged(int)));

    /* Boat ToolBar */
    connect(acLock, SIGNAL(triggered()), mainWindow, SLOT(slotFile_Lock()));
    connect(mainWindow,SIGNAL(updateLockIcon(QString)),this,SLOT(slot_updateLockIcon(QString)));
    connect(boatList, SIGNAL(activated(int)),mainWindow, SLOT(slotChgBoat(int)));

    /* BarrierSet ToolBar */
    connect(barrierAdd,SIGNAL(triggered()),mainWindow,SLOT(slot_barrierAddMenu()));

    //load_settings();
    Util::setFontDialog(this);
}
void ToolBar::mySetIcon(QToolButton * button,QString iconFile)
{
    QPixmap pix=QPixmap(iconFile);
#ifdef __ANDROID__
    pix=pix.scaled(iconSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
#endif
    button->setIconSize(iconSize);
    button->setIcon(QIcon(pix));
}
void ToolBar::mySetIcon(QAction * action,QString iconFile)
{
    QPixmap pix=QPixmap(iconFile);
#ifdef __ANDROID__
    pix=pix.scaled(iconSize,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
#endif
    action->setIcon(QIcon(pix));
}

int ToolBar::build_showHideMenu(QMenu *menu) {
    if(!menu) return 0;
    int nbEntry=0;

    for(int i=0;i<toolBarList.count();++i) {
        MyToolBar* tool = toolBarList.at(i);
        if(tool->get_canHide()) {
            ++nbEntry;
            tool->toggleViewAction()->setMenuRole(QAction::NoRole);
            menu->addAction(tool->toggleViewAction());
        }
    }
    return nbEntry;
}

void ToolBar::chgBoatType(int boatType) {
    switch(boatType) {
        case BOAT_VLM:
            boatToolBar->chgVisibilty(true);
            break;
        case BOAT_REAL:
        case BOAT_NOBOAT:
            boatToolBar->chgVisibilty(false);
            break;
    }
}

void ToolBar::slot_updateLockIcon(QString ic) {
    mySetIcon(acLock,ic);
}

QAction* ToolBar::init_Action(QString title, QString shortcut, QString statustip,QString iconFileName,QToolBar * toolBar)
{
    QAction *action;
    action = new QAction(title,toolBar);
    action->setShortcut  (shortcut);
    action->setShortcutContext (Qt::ApplicationShortcut);
    action->setStatusTip (statustip);
    if (iconFileName != "")
        mySetIcon(action,QString(iconFileName));
    return action;
}

void ToolBar::load_settings(void) {
    for(int i=0;i<toolBarList.count();++i) {
        MyToolBar * toolBar = toolBarList.at(i);
        bool myVisible=true;
#ifndef __ANDROID__
        QString key = "TB_" + toolBar->get_name();
        myVisible=Settings::getSettingOld(key,true,"ToolBar").toBool();
#endif
        toolBar->setVisible(myVisible);
        toolBar->setEnabled(myVisible);
        toolBar->set_displayed(myVisible);
        toolBar->initCanHide();
//#ifdef __ANDROID__
        mainWindow->addToolBar(Qt::TopToolBarArea,toolBar);
//#endif
        manageToolbarBreak();
    }
}
void ToolBar::manageToolbarBreak()
{
    //return;
    int Wmain=mainWindow->width();
    int currentWidth=0;
    for(int i=0;i<toolBarList.count();++i)
    {
        MyToolBar * tb=toolBarList.at(i);
        if(mainWindow->toolBarBreak(tb))
            mainWindow->removeToolBarBreak(tb);
        if(tb->isFloating()) continue;
        currentWidth+=tb->layout()->sizeHint().width();
        //qWarning()<<currentWidth<<tb->layout()->sizeHint().width()<<tb->get_name()<<1;
        if(currentWidth>Wmain)
        {
            currentWidth=tb->layout()->sizeHint().width();
            mainWindow->insertToolBarBreak(tb);
        }
        //qWarning()<<currentWidth<<tb->layout()->sizeHint().width()<<tb->get_name()<<2;
    }
    //qWarning()<<"exit manageToolBarBreak";
}

void ToolBar::save_settings(void) {
    for(int i=0;i<toolBarList.count();++i) {
        MyToolBar * toolBar = toolBarList.at(i);
        QString key = "TB_" + toolBar->get_name();
        Settings::setSettingOld(key,toolBar->get_displayed(),"ToolBar");
    }
}


/**********************************************************************/
/*                         Estime                                     */
/**********************************************************************/

void ToolBar::slot_estimeValueChanged(int value) {
    switch(Settings::getSetting(estimeType).toInt())
    {
        case 0:
            Settings::setSetting(estimeTime,value);
            break;
        case 1:
            Settings::setSetting(estimeVac,value);
            break;
        case 2:
            Settings::setSetting(estimeLen,value);
            break;
        default:
            break;
    }
    emit estimeParamChanged();
}

void ToolBar::slot_estimeTypeChanged(int num) {
    Settings::setSetting(estimeType,num);

    switch(num)
    {
        case 0:
            spnEstime->setValue(Settings::getSetting(estimeTime).toInt());
            break;
        case 1:
            spnEstime->setValue(Settings::getSetting(estimeVac).toInt());
            break;
        case 2:
            spnEstime->setValue(Settings::getSetting(estimeLen).toInt());
            break;
        default:
            spnEstime->setValue(0);
    }
    emit estimeParamChanged();
}

void ToolBar::slot_loadEstimeParam(void) {
    spnEstime->blockSignals(true);
    cbEstime->blockSignals(true);

    cbEstime->setEnabled(true);

    if(Settings::getSetting(scalePolar).toInt()!=1) {
        switch(Settings::getSetting(estimeType).toInt())
        {
            case 0:
                cbEstime->setCurrentIndex(0);
                spnEstime->setValue(Settings::getSetting(estimeTime).toInt());
                break;
            case 1:
                cbEstime->setCurrentIndex(1);
                spnEstime->setValue(Settings::getSetting(estimeVac).toInt());
                break;
            case 2:
                cbEstime->setCurrentIndex(2);
                spnEstime->setValue(Settings::getSetting(estimeLen).toInt());
                break;
            default:
                Settings::setSetting(estimeType,0);
                cbEstime->setCurrentIndex(0);
                spnEstime->setValue(0);
        }
    }
    else {
        cbEstime->setCurrentIndex(1);
        spnEstime->setValue(Settings::getSetting(estimeVac).toInt());
        cbEstime->setEnabled(false);
    }

    spnEstime->blockSignals(false);
    cbEstime->blockSignals(false);

}

void ToolBar::slot_estimeStartChanged(int state) {
    if(state>1) state=1;
    Settings::setSetting(startSpeedEstime, state);
    emit estimeParamChanged();
}

/**********************************************************************/
/*                         Grib                                       */
/**********************************************************************/

void ToolBar::update_gribBtn(void) {
    DataManager * dataManager=centralWidget->get_dataManager();

    if(dataManager && dataManager->isOk())     {
        time_t tps=dataManager->get_currentDate();
        time_t min=dataManager->get_minDate();
        time_t max=dataManager->get_maxDate();
        int step=get_gribStep();
        acDatesGrib_prev->setEnabled( ((tps-step)>=min) );
        acDatesGrib_next->setEnabled( ((tps+step)<=max) );
        acGrib_play->setEnabled( ((tps+step)<=max) );
        cbGribStep->setEnabled(true);
        datesGrib_sel->setEnabled(true);
        datesGrib_now->setEnabled(true);
    }
    else {
        acDatesGrib_prev->setEnabled(false);
        acDatesGrib_next->setEnabled(false);
        acGrib_play->setEnabled(false);
        cbGribStep->setEnabled(false);
        datesGrib_sel->setEnabled(false);
        datesGrib_now->setEnabled(false);
    }
}

int ToolBar::get_gribStep(void) {
    int stepTable[8]={300,900,1800,3600,7200,10800,21600,43200};
    return stepTable[cbGribStep->currentIndex()];
}

void ToolBar::slot_gribPlay(void) {
    if(acGrib_play->data().toInt()==0)
    {
         DataManager * dataManager=centralWidget->get_dataManager();
        if(dataManager && dataManager->isOk())
        {
            time_t tps=dataManager->get_currentDate();
            time_t max=dataManager->get_maxDate();
            int step=get_gribStep();
            if((tps+step)<=max)
            {
                mySetIcon(acGrib_play,QString(appFolder.value("img")+"player_end.png"));
                acGrib_play->setData(1);
                connect(centralWidget->get_terrain(),SIGNAL(terrainUpdated()),mainWindow,SLOT(slotDateGribChanged_next()));
                mainWindow->slotDateGribChanged_next();
            }
        }
    }
    else
        stopPlaying();
}

void ToolBar::stopPlaying(void) {
    mySetIcon(acGrib_play,QString(appFolder.value("img")+"player_play.png"));
    acGrib_play->setData(0);
    disconnect(centralWidget->get_terrain(),SIGNAL(terrainUpdated()),mainWindow,SLOT(slotDateGribChanged_next()));
}

void ToolBar::slot_gribDwnld(void) {
    switch(Settings::getSetting(defaultGribDwnld).toInt()) {
        case GRIB_DWNLD_ZYGRIB: slot_gribZygrib(); break;
        case GRIB_DWNLD_VLM: slot_gribVlm(); break;
        case GRIB_DWNLD_SAILSDOC: slot_gribSailsDoc(); break;
    }
}

void ToolBar::slot_gribZygrib(void) {
    Settings::setSetting(defaultGribDwnld,GRIB_DWNLD_ZYGRIB);
    update_gribDownloadBtn();
    emit gribZygrib();
}

void ToolBar::slot_gribVlm(void) {
    Settings::setSetting(defaultGribDwnld,GRIB_DWNLD_VLM);
    update_gribDownloadBtn();
    emit gribVlm();
}

void ToolBar::slot_gribSailsDoc(void) {
    Settings::setSetting(defaultGribDwnld,GRIB_DWNLD_SAILSDOC);
    update_gribDownloadBtn();
    emit gribSailsDoc();
}

void ToolBar::update_gribDownloadBtn(void) {
    QString iconString;
    switch(Settings::getSetting(defaultGribDwnld).toInt()) {
        case GRIB_DWNLD_ZYGRIB:
            iconString=appFolder.value("img")+ "network.png";
            break;
        case GRIB_DWNLD_VLM:
            iconString=appFolder.value("img")+"VLM_mto.png";
            break;
        case GRIB_DWNLD_SAILSDOC:
            iconString=appFolder.value("img")+ "kmail.png";
            break;
        default:
            iconString=appFolder.value("img")+ "wind.png";
            break;
    }

    if(!iconString.isEmpty())
        mySetIcon(gribDwnld,QString(iconString));
}

/**********************************************************************/
/*                         Boat                                       */
/**********************************************************************/

void ToolBar::updateBoatList(QList<boatVLM*> & boat_list) {
    while(boatList->count())
        boatList->removeItem(0);
    QListIterator<boatVLM*> i (boat_list);
    while(i.hasNext()) {
        boatVLM * acc = i.next();
        if(acc->getStatus()) {
            if(acc->getAliasState())
                boatList->addItem(acc->getAlias() + "(" + acc->getBoatPseudo() + ")");
            else
                boatList->addItem(acc->getBoatPseudo());
        }
    }
}

void ToolBar::setSelectedBoatIndex(int index) {
    boatList->setCurrentIndex(index);
}

/**********************************************************************/
/*                         BarrierSet                                 */
/**********************************************************************/

void ToolBar::chg_barrierAddState(bool state) {
    barrierAdd->blockSignals(true);
    barrierAdd->setChecked(state);
    barrierAdd->blockSignals(false);
}

/****************************************************/
/* MyToolBar                                        */
/****************************************************/

MyToolBar::MyToolBar(QString name,QString title,ToolBar *toolBar, QWidget *parent,bool canHide): QToolBar(title,parent)
{
    this->name=name;
    setObjectName(name);
    this->toolBar=toolBar;
    this->setIconSize(toolBar->getIconSize());
    displayed=true;
    forceMenuHide=false;
    this->canHide=canHide;
#ifdef __ANDROID__
    this->canHide=false;
#endif
    if(!canHide) {
        setFloatable(false);
        setMovable(false);
    }
    hide();
//    QString style=QString().sprintf("QToolButton#qt_toolbar_ext_button{min-width: %dpx; min-height: %dpx;}",toolBar->getIconSize().width()/2,toolBar->getIconSize().width()/2);
//    style+=QString().sprintf("QToolButton#qt_toolbar_ext_button{width: %dpx; height: %dpx;}",toolBar->getIconSize().width()/2,toolBar->getIconSize().width()/2);
//    style+="QToolButton#qt_toolbar_ext_button {qproperty-icon: url(img/addResized.png);}";
//    this->setStyleSheet(style);
    connect(this,SIGNAL(visibilityChanged(bool)),this,SLOT(slot_visibilityChanged(bool)));
}
/*
void MyToolBar::closeEvent ( QCloseEvent * event ) {
    qWarning() << "Closing tool " << name;
    displayed=false;
    QToolBar::closeEvent(event);
}
*/

/* we follow here visibility status */
void MyToolBar::slot_visibilityChanged(bool visibility) {
    //qWarning() << "visibilty of " << name << " : " << visibility;
    set_displayed(visibility);
    setEnabled(visibility);
}

/* manage visibilty without signals, so we can keep displayed status
 * visibilty = false => keep current status in displayed var and hide
 * toolBar entry in menu by setting forceMenuHide to TRUE
 * visibility = true => restore toolBar state using displayed var and
 * set forceMenuHide to FALSE in order to use only canHide var to hide/show
 * menu entry
 */
void MyToolBar::chgVisibilty(bool visibility) {
   blockSignals(true);
   forceMenuHide=!visibility;
   if(visibility && !displayed)
           visibility=false;

   setEnabled(visibility);
   setVisible(visibility);

   blockSignals(false);

}

void MyToolBar::initCanHide(void) {
    setFloatable(canHide);
    setMovable(canHide);
}
