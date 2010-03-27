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
#include <QStatusBar>
#include <QToolBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QDockWidget>
#include <QRegExp>
#include <QDebug>
#include <QDesktopServices>

#include "MainWindow.h"
#include "Util.h"
#include "Orthodromie.h"
#include "Version.h"
#include "boatAccount_dialog.h"
#include "BoardVLM.h"
#include "poi_delete.h"
#include "mycentralwidget.h"
#include "settings.h"
#include "opponentBoat.h"
#include "MenuBar.h"
#include "GshhsReader.h"
#include "DialogProxy.h"
#include "DialogVLM_grib.h"
#include "Polar.h"
#include "POI_input.h"
#include "paramVLM.h"
#include "Pilototo.h"
#include "boatAccount.h"
#include "Grib.h"
#include "GribRecord.h"
#include "POI.h"
#include "Projection.h"

//-----------------------------------------------------------
void MainWindow::InitActionsStatus()
{

    menuBar->acMap_Rivers->setChecked(Settings::getSetting("showRivers", false).toBool());
    menuBar->acMap_CountriesBorders->setChecked(Settings::getSetting("showCountriesBorders", true).toBool());
    menuBar->acMap_Orthodromie->setChecked(Settings::getSetting("showOrthodromie", false).toBool());

    menuBar->acView_WindColors->setChecked(Settings::getSetting("showWindColorMap", true).toBool());
    menuBar->acView_ColorMapSmooth->setChecked(Settings::getSetting("colorMapSmooth", true).toBool());
    menuBar->acView_WindArrow->setChecked(Settings::getSetting("showWindArrows", true).toBool());
    menuBar->acView_Barbules->setChecked(Settings::getSetting("showBarbules", true).toBool());
    menuBar->setCitiesNamesLevel(Settings::getSetting("showCitiesNamesLevel", 0).toInt());
    QString lang = Settings::getSetting("appLanguage", "none").toString();
    if (lang == "fr")
        menuBar->acOptions_Lang_fr->setChecked(true);
    else if (lang == "en")
        menuBar->acOptions_Lang_en->setChecked(true);

    //----------------------------------------------------------------------
    // Set map quality

    int quality = Settings::getSetting("gshhsMapQuality", 1).toInt();

    for (int qual=4; qual>=0; qual--) {
        if (! my_centralWidget->get_gshhsReader()->gshhsFilesExists(qual)) {
            switch (qual) {
                case 0: menuBar->acMap_Quality1->setEnabled(false); break;
                case 1: menuBar->acMap_Quality2->setEnabled(false); break;
                case 2: menuBar->acMap_Quality3->setEnabled(false); break;
                case 3: menuBar->acMap_Quality4->setEnabled(false); break;
                case 4: menuBar->acMap_Quality5->setEnabled(false); break;
            }
            if (quality > qual)
                quality = qual-1;
        }
    }
    if (quality < 0) {
        QMessageBox::information (this,
            QString(tr("Erreur")),
            QString(tr("Cartes non trouvées.\n\n")
                    +tr("Vérifiez l'installation du programme."))
        );
        quality = 0;
    }

    menuBar->setQuality(quality);

    emit signalMapQuality(quality);
}


//-----------------------------------------------------------
// Connexions des signaux
//-----------------------------------------------------------
void MainWindow::connectSignals()
{
    MenuBar  *mb = menuBar;

    //-------------------------------------
    // Actions
    //-------------------------------------
    connect(mb->ac_CreatePOI, SIGNAL(triggered()), this, SLOT(slotCreatePOI()));
    connect(mb->ac_pastePOI, SIGNAL(triggered()), this, SLOT(slotpastePOI()));
    connect(mb->ac_delAllPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_delAllPOIs()));
    connect(mb->ac_delSelPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_delSelPOIs()));
    //Porte
    connect(mb->acFile_Open, SIGNAL(triggered()), this, SLOT(slotFile_Open()));
    connect(mb->acFile_Close, SIGNAL(triggered()), this, SLOT(slotFile_Close()));
    connect(mb->acFile_Load_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileLoad_GRIB()));
    connect(mb->acFile_Load_VLM_GRIB, SIGNAL(triggered()), this, SLOT(slotLoadVLMGrib()));
    connect(mb->acFile_Info_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileInfo_GRIB()));
    connect(mb->acFile_Quit, SIGNAL(triggered()), this, SLOT(slotFile_Quit()));

    //-------------------------------------------------------
    connect(mb->acMap_GroupQuality, SIGNAL(triggered(QAction *)),
            this, SLOT(slotMap_Quality()));

    connect(mb->acMap_Zoom_In, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_In()));
    connect(mb->acMap_Zoom_Out, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_Out()));
    connect(mb->acMap_Zoom_Sel, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_Sel()));
    connect(mb->acMap_Zoom_All, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_All()));
    //-------------------------------------------------------
    connect(mb->acView_WindArrow, SIGNAL(triggered(bool)),
            this,  SLOT(slotWindArrows(bool)));

    //-------------------------------------------------------

    connect(mb->acOptions_Proxy, SIGNAL(triggered()), dialogProxy, SLOT(exec()));

    connect(mb->acOptions_GroupLanguage, SIGNAL(triggered(QAction *)),
            this, SLOT(slotOptions_Language()));

    //-------------------------------------------------------
    connect(mb->acHelp_Help, SIGNAL(triggered()), this, SLOT(slotHelp_Help()));
    connect(mb->acHelp_APropos, SIGNAL(triggered()), this, SLOT(slotHelp_APropos()));
    connect(mb->acHelp_AProposQT, SIGNAL(triggered()), this, SLOT(slotHelp_AProposQT()));

    //-------------------------------------------------------
    connect(mb->acVLMParamBoat, SIGNAL(triggered()), my_centralWidget, SLOT(slot_boatDialog()));
    connect(mb->acRace, SIGNAL(triggered()), my_centralWidget, SLOT(slot_raceDialog()));
    connect(mb->acVLMParam, SIGNAL(triggered()), this, SLOT(slotVLM_Param()));
    connect(mb->acVLMSync, SIGNAL(triggered()), this, SLOT(slotVLM_Sync()));

    connect(mb->acPOIAdd, SIGNAL(triggered()), this, SLOT(slot_newPOI()));
    connect(mb->ac_compassLine,SIGNAL(triggered()), this, SLOT(slotCompassLine()));
    connect(mb->ac_compassCenterBoat,SIGNAL(triggered()), this, SLOT(slotCompassCenterBoat()));
    connect(mb->ac_compassCenterWp,SIGNAL(triggered()), this, SLOT(slotCompassCenterWp()));
    connect(mb->ac_centerMap,SIGNAL(triggered()), this, SLOT(slot_centerMap()));

#ifdef __QTVLM_WITH_TEST
    if(mb->acVLMTest)
        connect(mb->acVLMTest, SIGNAL(triggered()), this, SLOT(slotVLM_Test()));
#endif
    connect(mb->acPOIinput, SIGNAL(triggered()), poi_input_dialog, SLOT(slot_showPOI_input()));
    connect(mb->acPilototo, SIGNAL(triggered()), this, SLOT(slotPilototo()));

    connect(mb->acPOIimport, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIimport()));

    connect(dialogProxy, SIGNAL(proxyUpdated()), this, SLOT(slotInetUpdated()));

    connect(mb->acPOISave, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POISave()));

    //-------------------------------------
    // Autres objets de l'interface
    //-------------------------------------
    connect(mb->cbGribStep, SIGNAL(activated(int)),
            this, SLOT(slotDateStepChanged(int)));
    connect(mb->datesGrib_now, SIGNAL(clicked()),
            this, SLOT(slotDateGribChanged_now()));
    connect(mb->datesGrib_sel, SIGNAL(clicked()),
            this, SLOT(slotDateGribChanged_sel()));
    connect(mb->acDatesGrib_next, SIGNAL(triggered()),
            this, SLOT(slotDateGribChanged_next()));
    connect(mb->acDatesGrib_prev, SIGNAL(triggered()),
            this, SLOT(slotDateGribChanged_prev()));
    connect(mb->boatList, SIGNAL(activated(int)),
            this, SLOT(slotChgBoat(int)));
    connect(mb->estime, SIGNAL(valueChanged(int)),
            this, SLOT(slotEstime(int)));

    //-------------------------------------
    // Autres signaux
    //-------------------------------------
    connect(loadVLM_grib, SIGNAL(signalGribFileReceived(QString)),
            this,  SLOT(slot_gribFileReceived(QString)));
}

//----------------------------------------------------
void MainWindow::slot_gribFileReceived(QString fileName)
{
    bool zoom =  (Settings::getSetting("gribZoomOnLoad",0).toInt()==1);
    openGribFile(fileName, zoom);
}

//=============================================================
MainWindow::MainWindow(int w, int h, QWidget *parent)
    : QMainWindow(parent)
{
    float prcx,prcy,scale;
    setWindowTitle("QtVlm");
    selectedBoat = NULL;

    settings = new Settings();

    qWarning() <<  "Starting qtVlm";
    progress=new QProgressDialog();
    progress->setLabelText("Starting qtVLM");
    progress->setMaximum(100);
    progress->setMinimum(0);
    progress->show();

    /* timer de gestion des VAC */
    timer = new QTimer(this);
    timer->setSingleShot(false);
    nxtVac_cnt=0;
    connect(timer,SIGNAL(timeout()),this, SLOT(updateNxtVac()));

    prcx = (float) Settings::getSetting("projectionCX", 0.0).toDouble();
    prcy = (float) Settings::getSetting("projectionCY", 0.0).toDouble();
    proj = new Projection (width(), height(),prcx,prcy);

    scale = (float) Settings::getSetting("projectionScale", 0.5).toDouble();
    proj->setScale(scale);

    connect(proj,SIGNAL(newZoom(float)),this,SLOT(slotNewZoom(float)));

    dialogProxy = new DialogProxy();

    //--------------------------------------------------

    progress->setLabelText("initializing menus and toolbars");
    menuBar = new MenuBar(this);
    setMenuBar(menuBar);

    my_centralWidget = new myCentralWidget(proj,this,menuBar);
    this->setCentralWidget(my_centralWidget);
    connect(this,SIGNAL(addPOI_list(POI*)),my_centralWidget,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(addPOI(QString,int,float,float,float,int,bool,boatAccount*)),
            my_centralWidget,SLOT(slot_addPOI(QString,int,float,float,float,int,bool,boatAccount*)));
    connect(my_centralWidget,SIGNAL(POI_selectAborted(POI*)),this,SLOT(slot_POIselected(POI*)));
   // connect(this,SIGNAL(updateRoute()),my_centralWidget,SLOT(slot_updateRoute()));

    //--------------------------------------------------


    statusBar =new QStatusBar(this);
    QFontInfo finfo = statusBar->fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    font.setStretch(QFont::SemiCondensed);

    statusBar->setFont(font);
    statusBar->setStyleSheet("QStatusBar::item {border: 0px;}");

    stBar_label_1 = new QLabel("Welcome in QtVlm", statusBar);
    stBar_label_1->setFont(font);
    stBar_label_1->setStyleSheet("color: rgb(0, 0, 255);");
    statusBar->addWidget(stBar_label_1);

    stBar_label_2 = new QLabel("", statusBar);
    stBar_label_2->setFont(font);
    stBar_label_2->setStyleSheet("color: rgb(255, 0, 0);");
    statusBar->addWidget(stBar_label_2);

    stBar_label_3 = new QLabel("", statusBar);
    stBar_label_3->setFont(font);
    //stBar_label_3->setStyleSheet("color: rgb(255, 0, 0);");
    statusBar->addWidget(stBar_label_3);

    //--------------------------------------------------
    toolBar = addToolBar(tr("Outils"));
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->addAction(menuBar->acFile_Quit);
    toolBar->addSeparator();
    toolBar->addAction(menuBar->acFile_Open);
    toolBar->addAction(menuBar->acFile_Load_VLM_GRIB);
    toolBar->addAction(menuBar->acFile_Close);
    toolBar->addWidget(menuBar->datesGrib_sel);
    toolBar->addWidget(menuBar->datesGrib_now);
    //toolBar->addWidget(menuBar->cbDatesGrib);
    toolBar->addAction(menuBar->acDatesGrib_prev);
    toolBar->addWidget(menuBar->cbGribStep);
    toolBar->addAction(menuBar->acDatesGrib_next);
    toolBar->addSeparator();
    toolBar->addAction(menuBar->acMap_Zoom_In);
    toolBar->addAction(menuBar->acMap_Zoom_Out);
    toolBar->addAction(menuBar->acMap_Zoom_Sel);
    toolBar->addAction(menuBar->acMap_Zoom_All);

    menuBar->cbGribStep->setEnabled(false);
    menuBar->acDatesGrib_prev->setEnabled(false);
    menuBar->acDatesGrib_next->setEnabled(false);
    menuBar->datesGrib_sel->setEnabled(false);
    menuBar->datesGrib_now->setEnabled(false);

    menuBar->cbGribStep->setCurrentIndex(Settings::getSetting("gribDateStep", 2).toInt());

    toolBar->addSeparator();
    toolBar->addWidget(menuBar->boatList);
    toolBar->addSeparator();
    tool_ESTIME = new QLabel(" Estime ", toolBar);
    tool_ESTIME->setFont(font);
    tool_ESTIME->setStyleSheet("color: rgb(0, 0, 255);");
    toolBar->addWidget(tool_ESTIME);
    toolBar->addWidget(menuBar->estime);
    tool_ESTIMEUNIT = new QLabel("", toolBar);
    tool_ESTIMEUNIT->setFont(font);
    tool_ESTIMEUNIT->setStyleSheet("color: rgb(0, 0, 255);");
    toolBar->addWidget(tool_ESTIMEUNIT);
    slot_ParamVLMchanged();
    toolBar->addSeparator();
    /*btn_Pilototo = new QPushButton(tr("Pilototo"),toolBar);
    btn_Pilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    toolBar->addWidget(btn_Pilototo);
    connect(btn_Pilototo,SIGNAL(clicked()),this, SLOT(slotPilototo()));*/

    tool_ETA = new QLabel("", toolBar);
    tool_ETA->setFont(font);
    tool_ETA->setStyleSheet("color: rgb(0, 0, 255);");
    toolBar->addWidget(tool_ETA);

    //--------------------------------------------------

    setStatusBar(statusBar);
    progress->setValue(10);

    //--------------------------------------------------
    progress->setLabelText("opening grib");
    gribFilePath = Settings::getSetting("gribFilePath", "grib").toString();
    QString fname = Settings::getSetting("gribFileName", "").toString();
    if (fname != "" && QFile::exists(fname))
    {
   //     qWarning() << "Opening grib :" << fname;
        openGribFile(fname, false);
   //     qWarning() << "Grib opened";
    }
    progress->setValue(20);


    //---------------------------------------------------------
    // Menu popup : bouton droit de la souris
    //---------------------------------------------------------
    progress->setLabelText("creating context menus");
    menuPopupBtRight = menuBar->createPopupBtRight(this);
    progress->setValue(25);

    //---------------------------------------------------------
    // VLM init
    //---------------------------------------------------------
    /* list of polar structure */

    progress->setLabelText("loading polars list");
    polar_list = new polarList(my_centralWidget->getInet());
//    progress->setValue(30);
    progress->setLabelText("reading boats data and loading POIs");
    my_centralWidget->loadData();
    progress->setValue(60);

    progress->setLabelText("Drawing some");

    poi_input_dialog = new POI_input(my_centralWidget);

    selPOI_instruction=NULL;
    isSelectingWP=false;

    menuBar->updateBoatList(my_centralWidget->getBoats());

    VLMBoard = new boardVLM(this,my_centralWidget->getInet());
    VLMBoard->show();

    param = new paramVLM(this,my_centralWidget);
    connect(param,SIGNAL(paramVLMChanged()),VLMBoard,SLOT(paramChanged()));    
    connect(param,SIGNAL(paramVLMChanged()),this,SLOT(slot_ParamVLMchanged()));

    pilototo = new Pilototo(this,my_centralWidget,my_centralWidget->getInet());

    loadVLM_grib = new DialogVLM_grib(this,my_centralWidget,my_centralWidget->getInet());

    //---------------------------------------------------------
    // Active les actions
    //---------------------------------------------------------
    connectSignals();
    InitActionsStatus();



    //------------------------------------------------
    // sync all boat
    progress->setLabelText("Synchronizing all boats");
    slotVLM_Sync();
    progress->setLabelText("Drawing all");
    progress->setValue(90);

     //--------------------------------------------------
    if(Settings::getSetting("saveMainWindowGeometry","1").toInt())
    {
        resize( Settings::getSetting("mainWindowSize", QSize(w,h)).toSize() );
        move  ( Settings::getSetting("mainWindowPos", QPoint()).toPoint() );
    }
    else
        showMaximized ();
    timerprogress=new QTimer();
    timerprogress->setSingleShot(true);
    timerprogress->setInterval(5000);
    connect(timerprogress,SIGNAL(timeout()),this, SLOT(slot_deleteProgress()));
    timerprogress->start();
}

//-----------------------------------------------
MainWindow::~MainWindow()
{
    //--------------------------------------------------
    // Save global settings
    //--------------------------------------------------
    my_centralWidget->setAboutToQuit();
    if(Settings::getSetting("saveMainWindowGeometry","1").toInt())
    {
        Settings::setSetting("mainWindowSize", size());
        Settings::setSetting("mainWindowPos", pos());
    }
    Settings::setSetting("projectionCX", proj->getCX());
    Settings::setSetting("projectionCY", proj->getCY());
    Settings::setSetting("projectionScale",  proj->getScale());
    Settings::setSetting("gribFileName",  gribFileName);
    Settings::setSetting("gribFilePath",  gribFilePath);
    /*freeze all routes*/
    my_centralWidget->freezeRoutes(true);
    if(selectedBoat) /* save the zoom factor */
        selectedBoat->setZoom(proj->getScale());

    delete my_centralWidget;

}

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    qWarning() << "Key pressed in main: " << event->key();
}
void MainWindow::slot_deleteProgress (void)
{
    progress->close();
    delete progress;
    delete timerprogress;
}

//-------------------------------------------------
void MainWindow::openGribFile(QString fileName, bool zoom)
{
    my_centralWidget->loadGribFile(fileName, zoom);

    if (my_centralWidget->getGrib())
    {
        setWindowTitle(tr("qtVlm - ")+ QFileInfo(fileName).fileName());
        slotDateGribChanged_now();
        gribFileName = fileName;
    }
    else {
        QMessageBox::critical (this,
            tr("Erreur"),
            tr("Fichier : ") + fileName + "\n\n"
                + tr("Echec lors de l'ouverture.") + "\n\n"
                + tr("Le fichier ne peut pas être ouvert,") + "\n"
                + tr("ou ce n'est pas un fichier GRIB,") + "\n"
                + tr("ou le fichier est corrompu,") + "\n"
                + tr("ou il contient des données non reconnues,") + "\n"
                + tr("ou...")
        );
        setWindowTitle(tr("qtVlm"));

        menuBar->cbGribStep->setEnabled(false);
        menuBar->acDatesGrib_prev->setEnabled(false);
        menuBar->acDatesGrib_next->setEnabled(false);
        menuBar->datesGrib_sel->setEnabled(false);
        menuBar->datesGrib_now->setEnabled(false);
    }
}

//-------------------------------------------------
// SLOTS
//-------------------------------------------------

void MainWindow::slotUpdateOpponent(void)
{
    bool found=false;
    if(!selectedBoat)
    {
        my_centralWidget->getOppList()->clear();
        return;
    }

    for(int i=0;i<my_centralWidget->getRaces().size();i++)
        if(my_centralWidget->getRaces()[i]->idrace == selectedBoat->getRaceId())
        {
            my_centralWidget->getOppList()->setBoatList(my_centralWidget->getRaces()[i]->oppList,my_centralWidget->getRaces()[i]->idrace,true);
            found=true;
            break;
        }
    if(!found)
        my_centralWidget->getOppList()->clear();
}

void MainWindow::slotCreatePOI()
{
    double lon, lat;
    proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
    emit newPOI((float)lon,(float)lat,proj,selectedBoat);
}

void MainWindow::slot_newPOI(void)
{
    emit newPOI(0.0,0.0,proj,selectedBoat);
}

void MainWindow::slot_centerBoat()
{
    if(selectedBoat)
        proj->setCenterInMap(selectedBoat->getLon(),selectedBoat->getLat());
}
//-------------------------------------------------
void MainWindow::slotOptions_Language()
{
    QString lang;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acOptions_GroupLanguage->checkedAction();
    if (act == mb->acOptions_Lang_fr) {
        lang = "fr";
        Settings::setSetting("appLanguage", lang);
        QMessageBox::information (this,
            QString("Changement de langue"),
            QString("Langue : Français\n\n")
              + QString("Les modifications seront prises en compte\n")
              + QString("au prochain lancement du programme.")
        );
    }
    else if (act == mb->acOptions_Lang_en) {
        lang = "en";
        Settings::setSetting("appLanguage", lang);
        QMessageBox::information (this,
            QString("Application Language"),
            QString("Language : English\n\n")
              + QString("Please reload application to activate language.\n")
        );
    }
}
//-------------------------------------------------
void MainWindow::slotMap_Quality()
{
    int quality = 0;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acMap_GroupQuality->checkedAction();
    if (act == mb->acMap_Quality1)
        quality = 0;
    else if (act == mb->acMap_Quality2)
        quality = 1;
    else if (act == mb->acMap_Quality3)
        quality = 2;
    else if (act == mb->acMap_Quality4)
        quality = 3;
    else if (act == mb->acMap_Quality5)
        quality = 4;

    Settings::setSetting("gshhsMapQuality", quality);
    emit signalMapQuality(quality);
}
//-------------------------------------------------



//-------------------------------------------------
void MainWindow::slotHelp_Help() {
    QDesktopServices::openUrl(QUrl("http://wiki.virtual-loup-de-mer.org/index.php/QtVlm#L.27interface_de_qtVlm"));
}

//-------------------------------------------------
void MainWindow::slotHelp_APropos() {
    QMessageBox::information (this,
            tr("A propos"),
            tr("qtVlm : GUI pour Virtual loup de mer")
            +"\nhttp://www.virtual-loup-de-mer.org\n"+

            tr("Version : ")+Version::getVersion()
                    +"      "+Version::getDate()
            +"\n"+ tr("Licence : GNU GPL v3")
            +"\n"+ tr("http://qtvlm.sf.net")
            +"\n"+ tr("Grib part is originaly from zygrib project")
            +"\n"+ tr("http://www.zygrib.org")
        );
}
//-------------------------------------------------
void MainWindow::slotHelp_AProposQT() {
    QMessageBox::aboutQt (this);
}

//-------------------------------------------------
void MainWindow::slotFile_Quit() {
    my_centralWidget->setAboutToQuit();
    QApplication::quit();
}

void MainWindow::setBoardToggleAction(QAction * action)
{
    menuBar->menuFile->addAction(action);
}

//-------------------------------------------------
void MainWindow::slotFile_Open()
{
    QString filter;
    filter =  tr("Fichiers GRIB (*.grb *.grib *.grb.bz2 *.grib.bz2 *.grb.gz *.grib.gz)")
            + tr(";;Autres fichiers (*)");
    QDir dirGrib(gribFilePath);
    if(!dirGrib.exists())
    {
        gribFilePath=QApplication::applicationDirPath()+"/grib";
        Settings::setSetting("askGribFolder",1);
        Settings::setSetting("edtGribFolder",gribFilePath);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier GRIB"),
                         gribFilePath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        gribFilePath = finfo.absolutePath();
        bool zoom =  (Settings::getSetting("gribZoomOnLoad",0).toInt()==1);
        openGribFile(fileName, zoom);
    }
}
//-------------------------------------------------
void MainWindow::slotFile_Close() {
    gribFileName = "";
    my_centralWidget->loadGribFile("", false);

    menuBar->acDatesGrib_prev->setEnabled(false);
    menuBar->acDatesGrib_next->setEnabled(false);
    menuBar->cbGribStep->setEnabled(false);
    menuBar->datesGrib_sel->setEnabled(false);
    menuBar->datesGrib_now->setEnabled(false);

    setWindowTitle(tr("qtVlm"));

}



//-----------------------------------------------
QString MainWindow::dataPresentInGrib(Grib* grib, int type)
{
    if(type!=GRB_WIND_VX)
        return tr("non");

    if (grib->getNumberOfGribRecords(GRB_WIND_VX,LV_ABOV_GND,10) > 0)
        return tr("oui");
    else
        return tr("non");
}

//========================================================================
void MainWindow::slotDateStepChanged(int id)
{
    Settings::setSetting("gribDateStep",id);
    updatePrevNext();
}

int MainWindow::getGribStep()
{
    int stepTable[7]={900,1800,3600,7200,10800,21600,43200};
    return stepTable[menuBar->cbGribStep->currentIndex()];
}

void MainWindow::updatePrevNext(void)
{
    #warning utiliser au max des signals/slot (voir ttes les fctions suivantes)
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t tps=my_centralWidget->getCurrentDate();
        time_t min=grib->getMinDate();
        time_t max=grib->getMaxDate();
        int step=getGribStep();
        menuBar->acDatesGrib_prev->setEnabled( ((tps-step)>=min) );
        menuBar->acDatesGrib_next->setEnabled( ((tps+step)<=max) );
        menuBar->cbGribStep->setEnabled(true);
        menuBar->datesGrib_sel->setEnabled(true);
        menuBar->datesGrib_now->setEnabled(true);
    }
    else
    {
        menuBar->cbGribStep->setEnabled(false);
        menuBar->acDatesGrib_prev->setEnabled(false);
        menuBar->acDatesGrib_next->setEnabled(false);
        menuBar->datesGrib_sel->setEnabled(false);
        menuBar->datesGrib_now->setEnabled(false);
    }
}

//-------------------------------------------------
void MainWindow::slotDateGribChanged_now()
{
    time_t tps=QDateTime::currentDateTime().toUTC().toTime_t();
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t min=grib->getMinDate();
        time_t max=grib->getMaxDate();
        if(tps<min) tps=min;
        if(tps>max) tps=max;
        my_centralWidget->setCurrentDate( tps );
        updatePrevNext();
        emit updateRoute();
    }
}

void MainWindow::slotDateGribChanged_sel()
{
    my_centralWidget->showGribDate_dialog();
    updatePrevNext();
    emit updateRoute();
}


//-------------------------------------------------
void MainWindow::slotDateGribChanged_next()
{
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t tps=my_centralWidget->getCurrentDate();
        time_t max=grib->getMaxDate();
        int step=getGribStep();
        if((tps+step)<=max)
            my_centralWidget->setCurrentDate(tps+step);
    }
    updatePrevNext();
    emit updateRoute();
}
//-------------------------------------------------
void MainWindow::slotDateGribChanged_prev()
{
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t tps=my_centralWidget->getCurrentDate();
        time_t min=grib->getMinDate();
        int step=getGribStep();
        if((tps-step)>=min)
            my_centralWidget->setCurrentDate(tps-step);
    }
    updatePrevNext();
    emit updateRoute();
}

void MainWindow::slotSetGribDate(int tps)
{
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t min=grib->getMinDate();
        time_t max=grib->getMaxDate();
        if(tps>=min && tps <=max)
            my_centralWidget->setCurrentDate(tps);
    }
    emit updateRoute();
}
//-------------------------------------------------
void MainWindow::slotWindArrows(bool b)
{
    // pas de barbules sans flèches
    menuBar->acView_Barbules->setEnabled(b);
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
       return selectedBoat!=NULL?selectedBoat->getVacLen():5;
}
void MainWindow::updatePilototo_Btn(boatAccount * boat)
{
    if(!selPOI_instruction)
    {
        /* compute nb Pilototo instructions */
        QStringList * lst = boat->getPilototo();
        QString pilototo_txt=tr("Pilototo");
        QString pilototo_toolTip="";
        if(boat->getHasPilototo())
        {
            int nbPending=0;
            int nb=0;
            for(int i=0;i<lst->count();i++)
                if(lst->at(i)!="none")
                {
                QStringList instr_buf = lst->at(i).split(",");
                int mode=instr_buf.at(2).toInt()-1;
                int pos =5;
                if(mode == 0 || mode == 1)
                    pos=4;
                if(instr_buf.at(pos) == "pending")
                    nbPending++;
                nb++;
            }
            if(nb!=0)
                pilototo_txt=pilototo_txt+" ("+QString().setNum(nbPending)+"/"+QString().setNum(nb)+")";
        }
        else
        {
            pilototo_toolTip=tr("Imp. de lire le pilototo de VLM");
            pilototo_txt=pilototo_txt+" (!)";
        }
        menuBar->acPilototo->setText(pilototo_txt);
        menuBar->acPilototo->setToolTip(pilototo_toolTip);
        VLMBoard->btn_Pilototo->setText(pilototo_txt);
        VLMBoard->btn_Pilototo->setToolTip(pilototo_toolTip);
    }
    else
    {
        menuBar->acPilototo->setText(tr("Selection d'une marque"));
        VLMBoard->btn_Pilototo->setText(tr("Selection d'une marque"));
    }
}

void MainWindow::statusBar_showWindData(double x,double y)
{
    QString s, res;
    double a,b;

    statusBar->clearMessage();
    stBar_label_1->setText( Util::pos2String(TYPE_LAT,y) + ", " + Util::pos2String(TYPE_LON,x));

    Grib * grib = my_centralWidget->getGrib();

    if(grib && grib->getInterpolatedValue_byDates(x,y,grib->getCurrentDate(),&a,&b))
    {
        res = "- " + tr("Vent") + ": ";
        s.sprintf("%.0f", radToDeg(b));
        res += s+tr("°")+", ";
        s.sprintf("%.1f",a);
        res += s+" kts";
    }
    stBar_label_2->setText(res);
}

void MainWindow::statusBar_showSelectedZone(float x0, float y0, float x1, float y1)
{
    QString message =
            tr("Sélection: ")
            + Util::formatPosition(x0,y0)
            + " -> "
            + Util::formatPosition(x1,y1);

    Orthodromie orth(x0,y0, x1,y1);
    QString s;
    message = message+ "   "
                + tr("(dist.orthodromique:")
                + Util::formatDistance(orth.getDistance())
//                + tr("  init.dir: %1°").arg(qRound(orth.getAzimutDeg()))
                + tr("  init.dir: %1°").arg(s.sprintf("%.1f",orth.getAzimutDeg()))
                + ")";

    statusBar->showMessage(message);
}

void MainWindow::updateNxtVac(void)
{
    if(!selectedBoat)
    {
        nxtVac_cnt=0;
    }
    else
    {
        nxtVac_cnt--;
        if(nxtVac_cnt<0)
            nxtVac_cnt=selectedBoat->getVacLen();
    }

    if(statusBar->currentMessage().isEmpty())
        drawVacInfo();
}

void MainWindow::drawVacInfo(void)
{
    if(selectedBoat)
    {
        QDateTime lastVac_date;
        lastVac_date.setTime_t(selectedBoat->getPrevVac());
        lastVac_date.setTimeSpec(Qt::UTC);
        stBar_label_3->setText("- "+ tr("Vacation de la dernière synchro") + ": " + lastVac_date.toString("dd-MM-yyyy, HH:mm:ss") + " - "+
                               tr("Prochaine vac dans") + ": " + QString().setNum(nxtVac_cnt) + "s");
    }
}

void MainWindow::slotShowContextualMenu(QGraphicsSceneContextMenuEvent * e)
{
    mouseClicX = e->scenePos().x();
    mouseClicY = e->scenePos().y();
    int compassMode = my_centralWidget->getCompassMode(mouseClicX,mouseClicY);

    if(my_centralWidget->isSelecting() && compassMode==0)
    {
        menuBar->ac_delAllPOIs->setEnabled(true);
        menuBar->ac_delSelPOIs->setEnabled(true);
    }
    else
    {
        menuBar->ac_delAllPOIs->setEnabled(false);
        menuBar->ac_delSelPOIs->setEnabled(false);
    }

    switch(compassMode)
    {
        case COMPASS_NOTHING:
            /* not showing menu line, default text*/
            menuBar->ac_compassLine->setText("Tirer un cap");
            menuBar->ac_compassLine->setEnabled(false);
            menuBar->ac_compassCenterBoat->setEnabled(false);
            menuBar->ac_compassCenterWp->setEnabled(false);
            break;
        case COMPASS_LINEON:
            /* showing text for compass line off*/
            menuBar->ac_compassLine->setText("Arret du cap");
            menuBar->ac_compassLine->setEnabled(true);
            menuBar->ac_compassCenterBoat->setEnabled(false);
            menuBar->ac_compassCenterWp->setEnabled(false);
            break;
        case COMPASS_UNDER:
            menuBar->ac_compassLine->setText("Tirer un cap");
            menuBar->ac_compassLine->setEnabled(true);
            menuBar->ac_compassCenterBoat->setEnabled(true);
            menuBar->ac_compassCenterWp->setEnabled(true);
            break;
        }

    menuPopupBtRight->exec(QCursor::pos());
}

void MainWindow::slot_centerMap()
{
    proj->setCentralPixel(this->mouseClicX,this->mouseClicY);
}

void MainWindow::slotCompassLine(void)
{
    QPoint cursor_pos=my_centralWidget->mapFromGlobal(cursor().pos());
    emit showCompassLine(cursor_pos.x(),cursor_pos.y());
}
void MainWindow::slotCompassCenterBoat(void)
{
    emit showCompassCenterBoat();
}
void MainWindow::slotCompassCenterWp(void)
{
    emit showCompassCenterWp();
}

void MainWindow::slotVLM_Param(void)
{
    param->changeParam();
    param->exec();
}

void MainWindow::slotVLM_Sync(void)
{
    bool hasFirstActivated = (selectedBoat!=NULL);
    QListIterator<boatAccount*> i (my_centralWidget->getBoats());

    qWarning() << "Doing a synch with VLM";

    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        if(acc->getStatus())
        {
            if(!hasFirstActivated)
            {
                /* no selected boat => select the first activated boat
                   no need to do getData as it will be done in selectBoat*/
                hasFirstActivated=true;
                acc->slot_selectBoat();
            }
            else
            {
                acc->slot_getData(true);
            }
        }
    }
    /* sync finished, update grib date */
    slotDateGribChanged_now();
    emit updateRoute();
}

/*****************************************
 signal send by each boat after it has update
*****************************************/
void MainWindow::slotBoatUpdated(boatAccount * boat,bool newRace,bool doingSync)
{
    //qWarning() << "Boat updated " << boat->getLogin();

    if(!boat->getStatus())
    {
        slotAccountListUpdated();
        boat->showNextGates();
        return;
    }

    if(boat == selectedBoat)
    {
        bool found=false;
        timer->stop();
        emit updateRoute();
        /* managing race data: opponnents position and trace*/
        if(newRace || my_centralWidget->getOppList()->getRaceId() != boat->getRaceId())
        { /* load a new race */
            for(int i=0;i<my_centralWidget->getRaces().size();i++)
                if(my_centralWidget->getRaces()[i]->idrace == boat->getRaceId())
                {
                    if(!my_centralWidget->getRaces()[i]->oppList.isEmpty())
                    {
                        my_centralWidget->getOppList()->setBoatList(my_centralWidget->getRaces()[i]->oppList,my_centralWidget->getRaces()[i]->idrace,false);
                        found=true;
                    }
                    break;
                }
            if(!found)
            {
                my_centralWidget->getOppList()->clear();
            }
        }
        else /* race has not changed, just refreshing position */
        {
            my_centralWidget->getOppList()->refreshData();
        }
        /* centering map on boat */
        if(doingSync)
        {
            if(Settings::getSetting("centerOnSynch",0).toInt()==1)
                proj->setCenterInMap(boat->getLon(),boat->getLat());
        }
        else
        {
            if(Settings::getSetting("centerOnSynch",0).toInt()==1)
                proj->setScaleAndCenterInMap(boat->getZoom(),boat->getLon(),boat->getLat());
        }

        /* updating Vac info */
        nxtVac_cnt=boat->getNextVac();
        drawVacInfo();
        timer->start(1000);

        /* Updating ETA */
        int nbS,j,h,m;
        QString txt;
        QString Eta = boat->getETA();
        QDateTime dtm =QDateTime::fromString(Eta,"yyyy-MM-ddTHH:mm:ssZ");
        dtm.setTimeSpec(Qt::UTC);
        QDateTime now = (QDateTime::currentDateTime()).toUTC();
        nbS=now.secsTo(dtm);
        j = nbS/(24*3600);
        nbS-=j*24*3600;
        h=nbS/3600;
        nbS-=h*3600;
        m=nbS/60;
        nbS-=m*60;
        txt.sprintf("(%dj %02dh%02dm%02ds)",j,h,m,nbS);
        tool_ETA->setText(tr(" Arrivée WP")+": " +dtm.toString("dd-MM-yyyy, HH:mm:ss")+ " " +txt);

        /* change data displayed in all pilototo buttons or menu entry: (nb of instructions passed / tot nb) */
        updatePilototo_Btn(boat);
        /* signal to Board and Pilototo that boat data have changed
           signal to pilototo is needed only when showing the pilototo dialog, the logic of this dialog:
           -> show wait msg box while doing a getData on current boat
           -> once boat send its hasUpdated signal, show the real dialog
        */
        emit boatHasUpdated(boat);

        /* disconnect this signal, it's used only once since after that routes will be updated from the sync slot*/
        disconnect(boat,SIGNAL(boatUpdated(boatAccount*,bool,bool)),this,SIGNAL(updateRoute()));

        /* send to all POI the new WP, the corresponding WP if exists will draw in a different color*/
        emit WPChanged(boat->getWPLat(),boat->getWPLon());
    }
    boat->showNextGates();

}

void MainWindow::slotSelectBoat(boatAccount* newSelect)
{
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
        if(newSelect->getStatus()) /* is selected boat activated ?*/
        {
            newSelect->slot_getData(false);
            //proj->setScaleAndCenterInMap(newSelect->getZoom(),newSelect->getLon(),newSelect->getLat());
            menuBar->acPilototo->setEnabled(!newSelect->getLockStatus());
        }
        else
            menuBar->acPilototo->setEnabled(false);

        /* manage item of boat list */
        int cnt=0;
        for(int i=0;i<my_centralWidget->getBoats().count();i++)
        {
            if(my_centralWidget->getBoats()[i] == newSelect)
            {
                menuBar->setSelectedBoatIndex(cnt);
                break;
            }
            if(my_centralWidget->getBoats()[i]->getStatus())
                cnt++;
        }

    }
}

/***********************************
  Called when boat list is changed
  *********************************/

void MainWindow::slotChgBoat(int num)
{
    QListIterator<boatAccount*> i (my_centralWidget->getBoats());
    int cnt=0;
    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        if(acc->getStatus())
        {
            if(cnt==num)
            {
                acc->slot_selectBoat();
                /* sync lunched, update grib date */
                //slotDateGribChanged_now();
                emit selectedBoatChanged();
                break;
            }
            cnt++;
        }
    }
}

void MainWindow::slotSelectPOI(Pilototo_instruction * instruction)
{
    selPOI_instruction=instruction;
    menuBar->boatList->setEnabled(false);
    updatePilototo_Btn(selectedBoat);
    slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
}

void MainWindow::slotSelectWP_POI()
{
    isSelectingWP=true;
    menuBar->boatList->setEnabled(false);
    slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
}

bool MainWindow::get_selPOI_instruction()
{
    return ((selPOI_instruction!=NULL) || (isSelectingWP));
}

void MainWindow::slot_POIselected(POI* poi)
{
    if(selPOI_instruction)
    {
        Pilototo_instruction * tmp=selPOI_instruction;
        selPOI_instruction=NULL;
        updatePilototo_Btn(selectedBoat);
        menuBar->boatList->setEnabled(true);
        slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
        emit editInstructionsPOI(tmp,poi);
    }
    else if(isSelectingWP)
    {
        isSelectingWP=false;
        menuBar->boatList->setEnabled(true);
        slotBoatLockStatusChanged(selectedBoat,selectedBoat->getLockStatus());
        emit editWP_POI(poi);
    }

}

void MainWindow::slotInetUpdated(void)
{
    qWarning() << "Inet Updated";
    emit updateInet();
    slotVLM_Sync();
}

void MainWindow::slotAccountListUpdated(void)
{
    selectedBoat=NULL;
    menuBar->updateBoatList(my_centralWidget->getBoats());
    slotVLM_Sync();
}

void MainWindow::slotChgWP(float lat,float lon, float wph)
{
    if(VLMBoard)
        VLMBoard->setWP(lat,lon,wph);
}

void MainWindow::slotpastePOI()
{
    float lon, lat,wph;
    int tstamp;
    QString name;

    if(!Util::getWPClipboard(&name,&lat,&lon,&wph,&tstamp))
        return;

    emit addPOI(name,POI_TYPE_POI,lat,lon,wph,tstamp,tstamp!=-1, selectedBoat);
}



void MainWindow::slotBoatLockStatusChanged(boatAccount* boat,bool status)
{
    if(boat==selectedBoat)
    {
        if(selPOI_instruction)
        {
            emit setChangeStatus(true);
            menuBar->acPilototo->setEnabled(true);
            menuBar->acVLMSync->setEnabled(false);
            VLMBoard->btn_Pilototo->setEnabled(true);
        }
        else if(isSelectingWP)
        {
            emit setChangeStatus(true);
            menuBar->acPilototo->setEnabled(false);
            menuBar->acVLMSync->setEnabled(false);
            VLMBoard->btn_Pilototo->setEnabled(false);
        }
        else
        {
            emit setChangeStatus(status);
            menuBar->acPilototo->setEnabled(!status);
            menuBar->acVLMSync->setEnabled(true);
            VLMBoard->btn_Pilototo->setEnabled(!status);
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

void MainWindow::slotPilototo(void)
{
    if(!selectedBoat) return;

    if(selPOI_instruction)
    {
        slot_POIselected(NULL);
    }
    else
    {
        if(!getBoatLockStatus())
        {
            emit editInstructions();
            selectedBoat->slot_getData(true);
        }
    }
}

bool MainWindow::isBoat(QString idu)
{
    for(int i=0;i<my_centralWidget->getBoats().count();i++)
        if(my_centralWidget->getBoats()[i]->getBoatId() == idu)
        {
            if(my_centralWidget->getBoats()[i]->getStatus())
                return true;
            else
                return false;
        }
    return false;
}

void MainWindow::slotParamChanged(void)
{
#warning voir pour mettre un signal direct
    emit paramVLMChanged();
}

void MainWindow::getBoatBvmg(float * up,float * down,float ws)
{
   if(!selectedBoat)
   {
       *up=-1;
       *down=-1;
   }
   else
       if(selectedBoat->getPolarData())
       {
            *up=selectedBoat->getBvmgUp(ws);
            *down=selectedBoat->getBvmgDown(ws);
       }
       else
       {
            *up=-1;
            *down=-1;
       }
}
float MainWindow::getBoatPolarSpeed(float ws,float angle)
{
   if(!selectedBoat)
   {
       return 0;
   }
   else
       if(selectedBoat->getPolarData())
       {
            return selectedBoat->getPolarData()->getSpeed(ws,angle);
       }
       else
       {
            return 0;
       }
}
float MainWindow::getBoatPolarMaxSpeed()
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

void MainWindow::getBoatWP(double * lat,double * lon)
{
   if(!lat || !lon)
       return;
   if(!selectedBoat)
   {
       *lat=-1;
       *lon=-1;
   }
   else
   {
       *lat = selectedBoat->getWPLat();
       *lon = selectedBoat->getWPLon();
   }
}

void MainWindow::slotNewZoom(float zoom)
{
    if(selectedBoat)
        selectedBoat->setZoom(zoom);
}

void MainWindow::getPolar(QString fname,Polar ** ptr)
{
    qWarning() << "Get polar: " << fname;
    if(ptr)
    {
        *ptr=polar_list->needPolar(fname);
    }
    //polar_list->stats();
}

void MainWindow::releasePolar(QString fname)
{
    polar_list->releasePolar(fname);
    //polar_list->stats();
}

void MainWindow::slotLoadVLMGrib(void)
{
    loadVLM_grib->showDialog();
}

void MainWindow::slotValidationDone(bool ok)
{
    VLMBoard->validationDone(ok);
}

/*************************************/

void MainWindow::slotVLM_Test(void)
{
    Grib * myGrib=this->my_centralWidget->getGrib();
    double u,v;
    //time_t tps=QDateTime::currentDateTime().toUTC().toTime_t();
    time_t tps=myGrib->getCurrentDate();

    //double val =  degToRad(120) ;

    //qWarning() << "Now = " << QDateTime::currentDateTime().toUTC();

    myGrib->getInterpolatedValue_byDates(120.4,30.2,tps,&u,&v,true);
    //qWarning() << "120,35 => " << u << " " << radToDeg(v);
    /*myGrib->getInterpolatedValue_byDates(-20,-20,tps,&u,&v,true);
    qWarning() << "-20,-20 => " << u << " " << radToDeg(v);
    myGrib->getInterpolatedValue_byDates(0,0,tps,&u,&v,true);
    qWarning() << "0,0 => " << u << " " << radToDeg(v);*/

}

void MainWindow::slotEstime(int valeur)
{
    switch(Settings::getSetting("estimeType","0").toInt())
    {
        case 0:
            Settings::setSetting("estimeTime", valeur);
            break;
        case 1:
            Settings::setSetting("estimeVac", valeur);
            break;
        case 2:
            Settings::setSetting("estimeLen", valeur);
    }
    emit paramVLMChanged();
}
void MainWindow::slot_ParamVLMchanged()
{
    switch(Settings::getSetting("estimeType","0").toInt())
    {
        case 0:
            tool_ESTIMEUNIT->setText(" mins");
            menuBar->estime->setValue(Settings::getSetting("estimeTime","60").toInt());
            break;
        case 1:
            tool_ESTIMEUNIT->setText(" vacs");
            menuBar->estime->setValue(Settings::getSetting("estimeVac","12").toInt());
            break;
        case 2:
            tool_ESTIMEUNIT->setText(" milles");
            menuBar->estime->setValue(Settings::getSetting("estimeLen","50").toInt());
            break;
        default:
            tool_ESTIMEUNIT->setText(" mins");
            menuBar->estime->setValue(0);
    }

}
