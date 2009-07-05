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

#include "MainWindow.h"
#include "Util.h"
#include "Orthodromie.h"
#include "Version.h"
#include "DialogLoadGrib.h"
#include "boatAccount_dialog.h"
#include "BoardVLM.h"


//-----------------------------------------------------------
void MainWindow::InitActionsStatus()
{

    menuBar->acMap_Rivers->setChecked(Util::getSetting("showRivers", false).toBool());
    menuBar->acMap_CountriesBorders->setChecked(Util::getSetting("showCountriesBorders", true).toBool());
    menuBar->acMap_Orthodromie->setChecked(Util::getSetting("showOrthodromie", false).toBool());

    menuBar->acView_WindColors->setChecked(Util::getSetting("showWindColorMap", true).toBool());
    menuBar->acView_ColorMapSmooth->setChecked(Util::getSetting("colorMapSmooth", true).toBool());
    menuBar->acView_WindArrow->setChecked(Util::getSetting("showWindArrows", true).toBool());
    menuBar->acView_Barbules->setChecked(Util::getSetting("showBarbules", true).toBool());
    menuBar->setCitiesNamesLevel(Util::getSetting("showCitiesNamesLevel", 0).toInt());
    terre->setCitiesNamesLevel(Util::getSetting("showCitiesNamesLevel", 0).toInt());

    //----------------------------------------------------------------------
    QString lang = Util::getSetting("appLanguage", "none").toString();
    if (lang == "fr")
        menuBar->acOptions_Lang_fr->setChecked(true);
    else if (lang == "en")
        menuBar->acOptions_Lang_en->setChecked(true);

    //----------------------------------------------------------------------
    // Set map quality
    int quality = Util::getSetting("gshhsMapQuality", 1).toInt();
    for (int qual=4; qual>=0; qual--) {
        if (! gshhsReader->gshhsFilesExists(qual)) {
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
    terre->setMapQuality(quality);
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
    connect(mb->ac_delPOIs, SIGNAL(triggered()), this, SLOT(slotDelPOIs()));

    connect(mb->acFile_Open, SIGNAL(triggered()), this, SLOT(slotFile_Open()));
    connect(mb->acFile_Close, SIGNAL(triggered()), this, SLOT(slotFile_Close()));
    connect(mb->acFile_Load_GRIB, SIGNAL(triggered()), this, SLOT(slotFile_Load_GRIB()));
    connect(mb->acFile_Load_VLM_GRIB, SIGNAL(triggered()), this, SLOT(slotLoadVLMGrib()));
    connect(mb->acFile_Info_GRIB, SIGNAL(triggered()), this, SLOT(slotFile_Info_GRIB()));
    connect(mb->acFile_Quit, SIGNAL(triggered()), this, SLOT(slotFile_Quit()));

    //-------------------------------------------------------
    connect(mb->acMap_GroupQuality, SIGNAL(triggered(QAction *)),
            this, SLOT(slotMap_Quality()));

    connect(mb->acMap_Rivers, SIGNAL(triggered(bool)),
            terre,  SLOT(setDrawRivers(bool)));
    connect(mb->acMap_CountriesBorders, SIGNAL(triggered(bool)),
            terre,  SLOT(setDrawCountriesBorders(bool)));
    connect(mb->acMap_Orthodromie, SIGNAL(triggered(bool)),
            terre,  SLOT(setDrawOrthodromie(bool)));
    connect(mb->acMap_CountriesNames, SIGNAL(triggered(bool)),
            terre,  SLOT(setCountriesNames(bool)));
    connect(mb->acMap_GroupCitiesNames, SIGNAL(triggered(QAction *)),
            this, SLOT(slotMap_CitiesNames()));

    connect(mb->acMap_Zoom_In, SIGNAL(triggered()),
            terre,  SLOT(slot_Zoom_In()));
    connect(mb->acMap_Zoom_Out, SIGNAL(triggered()),
            terre,  SLOT(slot_Zoom_Out()));
    connect(mb->acMap_Zoom_Sel, SIGNAL(triggered()),
            terre,  SLOT(slot_Zoom_Sel()));
    connect(mb->acMap_Zoom_All, SIGNAL(triggered()),
            terre,  SLOT(slot_Zoom_All()));
    //-------------------------------------------------------
    connect(mb->acView_WindColors, SIGNAL(triggered(bool)),
            this,  SLOT(slotWindColors(bool)));

    connect(mb->acView_ColorMapSmooth, SIGNAL(triggered(bool)),
            terre,  SLOT(setColorMapSmooth(bool)));

    connect(mb->acView_WindArrow, SIGNAL(triggered(bool)),
            terre,  SLOT(setDrawWindArrows(bool)));
    connect(mb->acView_WindArrow, SIGNAL(triggered(bool)),
            this,  SLOT(slotWindArrows(bool)));
    connect(mb->acView_Barbules, SIGNAL(triggered(bool)),
            terre,  SLOT(setBarbules(bool)));

    //-------------------------------------------------------
    connect(mb->acOptions_Units, SIGNAL(triggered()), &dialogUnits, SLOT(exec()));
    connect(&dialogUnits, SIGNAL(accepted()), terre, SLOT(slotMustRedraw()));

    connect(mb->acOptions_GraphicsParams, SIGNAL(triggered()), &dialogGraphicsParams, SLOT(exec()));
    connect(&dialogGraphicsParams, SIGNAL(accepted()), terre, SLOT(updateGraphicsParameters()));

    connect(mb->acOptions_Proxy, SIGNAL(triggered()), &dialogProxy, SLOT(exec()));

    connect(mb->acOptions_GroupLanguage, SIGNAL(triggered(QAction *)),
            this, SLOT(slotOptions_Language()));

    //-------------------------------------------------------
    connect(mb->acHelp_Help, SIGNAL(triggered()), this, SLOT(slotHelp_Help()));
    connect(mb->acHelp_APropos, SIGNAL(triggered()), this, SLOT(slotHelp_APropos()));
    connect(mb->acHelp_AProposQT, SIGNAL(triggered()), this, SLOT(slotHelp_AProposQT()));

    //-------------------------------------------------------
    connect(mb->acVLMParamBoat, SIGNAL(triggered()), this, SLOT(slotVLM_ParamBoat()));
    connect(mb->acRace, SIGNAL(triggered()), this, SLOT(slotVLM_ParamRace()));
    connect(mb->acVLMParam, SIGNAL(triggered()), this, SLOT(slotVLM_Param()));
    connect(mb->acVLMSync, SIGNAL(triggered()), this, SLOT(slotVLM_Sync()));
#ifdef __QTVLM_WITH_TEST
    if(mb->acVLMTest)
        connect(mb->acVLMTest, SIGNAL(triggered()), this, SLOT(slotVLM_Test()));
#endif
    connect(mb->acPOIinput, SIGNAL(triggered()), this, SLOT(slotPOIinput()));
    connect(mb->acPilototo, SIGNAL(triggered()), this, SLOT(slotPilototo()));

    connect(mb->acPOIimport, SIGNAL(triggered()), this, SLOT(slotPOIimport()));

    connect(&dialogProxy, SIGNAL(proxyUpdated()), this, SLOT(slotInetUpdated()));

    connect(mb->acPOISave, SIGNAL(triggered()), this, SLOT(slotBoatSave()));

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

    //-------------------------------------
    // Autres signaux
    //-------------------------------------
    connect(this, SIGNAL(signalMapQuality(int)),
            terre,  SLOT(setMapQuality(int)));

    //-----------------------------------------------------------
    connect(terre, SIGNAL(mouseClicked(QMouseEvent *)),
            this,  SLOT(slotMouseClicked(QMouseEvent *)));
    connect(terre, SIGNAL(mouseMoved(QMouseEvent *)),
            this,  SLOT(slotMouseMoved(QMouseEvent *)));
    connect(terre,SIGNAL(mouseDblClicked(QMouseEvent *)),
            this, SLOT(slotMouseDblClicked(QMouseEvent *)));
    connect(terre,SIGNAL(showContextualMenu(QContextMenuEvent *)),
            this, SLOT(slotShowContextualMenu(QContextMenuEvent *)));
    connect(terre,SIGNAL(POI_selectAborted(POI*)),
            this, SLOT(slotPOIselected(POI*)));
    //-----------------------------------------------------------
    connect(&dialogLoadGrib, SIGNAL(signalGribFileReceived(QString)),
            this,  SLOT(slotGribFileReceived(QString)));
    connect(loadVLM_grib, SIGNAL(signalGribFileReceived(QString)),
            this,  SLOT(slotGribFileReceived(QString)));
}

//----------------------------------------------------
void MainWindow::slotGribFileReceived(QString fileName)
{
    bool zoom =  (Util::getSetting("gribZoomOnLoad",0).toInt()==1);
    openGribFile(fileName, zoom);
}

//=============================================================
MainWindow::MainWindow(int w, int h, QWidget *parent)
    : QMainWindow(parent)
{
    float prcx,prcy,scale;
    setWindowTitle("QtVlm");
    selectedBoat = NULL;

    qWarning() <<  "Starting qtVlm";
    //--------------------------------------------------
    resize( Util::getSetting("mainWindowSize", QSize(w,h)).toSize() );
    move  ( Util::getSetting("mainWindowPos", QPoint()).toPoint() );

    int mapQuality = 0;
    gshhsReader = new GshhsReader("maps/gshhs", mapQuality);
    assert(gshhsReader);

    proj = new Projection (width(), height(), 0,0);
    prcx = (float) Util::getSetting("projectionCX", 0.0).toDouble();
    prcy = (float) Util::getSetting("projectionCY", 0.0).toDouble();
    scale = (float) Util::getSetting("projectionScale", 0.5).toDouble();
    proj->setCenterInMap(prcx,prcy);
    proj->setScale(scale);

    connect(proj,SIGNAL(newZoom(float)),this,SLOT(slotNewZoom(float)));

    //--------------------------------------------------
    terre = new Terrain(this, proj);
    assert(terre);
    terre->setGSHHS_map(gshhsReader);

    //--------------------------------------------------
    menuBar = new MenuBar(this);
    assert(menuBar);

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

    menuBar->cbGribStep->setCurrentIndex(Util::getSetting("gribDateStep", 2).toInt());

    toolBar->addSeparator();
    toolBar->addWidget(menuBar->boatList);
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
    setMenuBar(menuBar);
    setStatusBar(statusBar);
    setCentralWidget(terre);

    //--------------------------------------------------
    gribFilePath = Util::getSetting("gribFilePath", "grib").toString();
    QString fname = Util::getSetting("gribFileName", "").toString();
    if (fname != "" && QFile::exists(fname))
    {
        qWarning() << "Opening grib :" << fname;
        openGribFile(fname, false);
        qWarning() << "Grib opened";
    }

    compass = new mapCompass(proj,terre);
    if(Util::getSetting("showCompass",1).toInt()==1)
        compass->show();
    else
        compass->hide();

    //---------------------------------------------------------
    // Menu popup : bouton droit de la souris
    //---------------------------------------------------------
    menuPopupBtRight = menuBar->createPopupBtRight(this);

    //---------------------------------------------------------
    // VLM init
    //---------------------------------------------------------
    /* list of polar structure */
    polar_list = new polarList();

    /* read boat Data */
    race_vacLen=300;
    xmlData = new xml_boatData(proj,this,terre);
    xmlData->readBoatData(acc_list,race_list,"boatAcc.dat");

    boatAcc = new boatAccount_dialog(proj,this,terre);

    param = new paramVLM(terre);
    poi_input_dialog = new POI_input(terre);
    selPOI_instruction=NULL;
    isSelectingWP=false;

    terre->setBoatList(acc_list);
    menuBar->updateBoatList(acc_list);

    VLMBoard = new boardVLM(this);
    connect(this,SIGNAL(boatHasUpdated(boatAccount*)),
            VLMBoard,SLOT(boatUpdated(boatAccount*)));

    connect(param,SIGNAL(paramVLMChanged()),VLMBoard,SLOT(paramChanged()));

    connect(param,SIGNAL(paramVLMChanged()),this,SLOT(slotParamChanged()));
    connect(param, SIGNAL(inetUpdated()), this, SLOT(slotInetUpdated()));

    connect(poi_input_dialog,SIGNAL(addPOI(float,float,float,int,bool)),
            this,SLOT(slotAddPOI(float,float,float,int,bool)));

    poi_editor=new POI_Editor(this,terre);

    pilototo = new Pilototo(this,terre);
    connect(this,SIGNAL(editInstructions()),
            pilototo,SLOT(editInstructions()));
    connect(this,SIGNAL(editInstructionsPOI(Pilototo_instruction * ,POI * )),
            pilototo,SLOT(editInstructionsPOI(Pilototo_instruction * ,POI * )));
    connect(this,SIGNAL(boatHasUpdated(boatAccount*)),
            pilototo,SLOT(boatUpdated(boatAccount*)));

    raceParam = new race_dialog(this, terre);
    opponents = new opponentList(proj,this,terre);

    connect(this,SIGNAL(getTrace(QString,QList<position*> *)),
            opponents,SLOT(getTrace(QString,QList<position*> *)));
    terre->setOpponents(opponents);

    timer = new QTimer(this);
    timer->setSingleShot(false);
    nxtVac_cnt=0;
    connect(timer,SIGNAL(timeout()),this, SLOT(updateNxtVac()));

    loadVLM_grib = new DialogVLM_grib(this,terre);

    //---------------------------------------------------------
    // Active les actions
    //---------------------------------------------------------
    connectSignals();
    InitActionsStatus();

    // POI's
    while (!poi_list.isEmpty())
        delete poi_list.takeFirst();
    xmlPOI = new xml_POIData(proj,this,terre);
    xmlPOI->readData(poi_list,"poi.dat");
    //------------------------------------------------
    // sync all boat
    slotVLM_Sync();
}

//-----------------------------------------------
MainWindow::~MainWindow()
{
    //--------------------------------------------------
    // Save global settings
    //--------------------------------------------------
    Util::setSetting("mainWindowSize", size());
    Util::setSetting("mainWindowPos", pos());
    Util::setSetting("projectionCX", proj->getCX());
    Util::setSetting("projectionCY", proj->getCY());
    Util::setSetting("projectionScale",  proj->getScale());
    Util::setSetting("gribFileName",  gribFileName);
    Util::setSetting("gribFilePath",  gribFilePath);

    xmlPOI->writeData(poi_list,"poi.dat");

    if(selectedBoat) /* save the zoom factor */
        selectedBoat->setZoom(proj->getScale());

    slotWriteBoat();

    delete boatAcc;
}

void MainWindow::addPOI_list(POI * poi)
{
    poi_list.append(poi);
}

void MainWindow::delPOI_list(POI * poi)
{
    poi_list.removeAll(poi);
}

//-------------------------------------------------
void MainWindow::openGribFile(QString fileName, bool zoom)
{
    terre->loadGribFile(fileName, zoom);
    if (terre->getGrib()->isOk())
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
void MainWindow::slotCreatePOI()
{
    double lon, lat;
    proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
    emit newPOI((float)lon,(float)lat,proj);
}

//-------------------------------------------------
void MainWindow::slotOptions_Language()
{
    QString lang;
    MenuBar  *mb = menuBar;
    QAction *act = mb->acOptions_GroupLanguage->checkedAction();
    if (act == mb->acOptions_Lang_fr) {
        lang = "fr";
        Util::setSetting("appLanguage", lang);
        QMessageBox::information (this,
            QString("Changement de langue"),
            QString("Langue : Français\n\n")
              + QString("Les modifications seront prises en compte\n")
              + QString("au prochain lancement du programme.")
        );
    }
    else if (act == mb->acOptions_Lang_en) {
        lang = "en";
        Util::setSetting("appLanguage", lang);
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

    Util::setSetting("gshhsMapQuality", quality);
    emit signalMapQuality(quality);
}
//-------------------------------------------------
void MainWindow::slotMap_CitiesNames()
{
    MenuBar  *mb = menuBar;
    QAction *act = mb->acMap_GroupCitiesNames->checkedAction();
    if (act == mb->acMap_CitiesNames0)
        terre->setCitiesNamesLevel(0);
    else if (act == mb->acMap_CitiesNames1)
        terre->setCitiesNamesLevel(1);
    else if (act == mb->acMap_CitiesNames2)
        terre->setCitiesNamesLevel(2);
    else if (act == mb->acMap_CitiesNames3)
        terre->setCitiesNamesLevel(3);
    else if (act == mb->acMap_CitiesNames4)
        terre->setCitiesNamesLevel(4);
}


//-------------------------------------------------
void MainWindow::slotHelp_Help() {
    /*QMessageBox::question (this,
            tr("Aide"),
            tr("Des questions ?"));

    QString coderand = QDateTime::currentDateTime().toString("yyyymmsszzz");

    QMessageBox::information (this,
            tr("Aide"),
            tr("Félicitations, votre demande a bien été enregistrée.\n")
            +tr("Référence : ") + coderand
            +tr("\n\n")
            +tr("Nos services vous contacteront peut-être quand ils seront en mesure de vous répondre.")
            +tr("\n\n")
            +tr("En attendant, essayez donc d'appuyer au hasard sur des touches du clavier, ")
            +tr("ou bien de bouger la souris en appuyant de temps en temps ")
            +tr("sur l'un de ses boutons, ")
            +tr("ou bien n'importe quoi d'autre, ")
            +tr("et vous verrez bien s'il se passe quelque chose...")
            );*/
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
    QApplication::quit();
}
//-------------------------------------------------
void MainWindow::slotFile_Open()
{
    QString filter;
    filter =  tr("Fichiers GRIB (*.grb *.grib *.grb.bz2 *.grib.bz2 *.grb.gz *.grib.gz)")
            + tr(";;Autres fichiers (*)");

    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier GRIB"),
                         gribFilePath,
                         filter);

    if (fileName != "")
    {
        QFileInfo finfo(fileName);
        gribFilePath = finfo.absolutePath();
        bool zoom =  (Util::getSetting("gribZoomOnLoad",0).toInt()==1);
        openGribFile(fileName, zoom);
    }
}
//-------------------------------------------------
void MainWindow::slotFile_Close() {
    gribFileName = "";
    terre->loadGribFile("", false);

    menuBar->acDatesGrib_prev->setEnabled(false);
    menuBar->acDatesGrib_next->setEnabled(false);
    menuBar->cbGribStep->setEnabled(false);
    menuBar->datesGrib_sel->setEnabled(false);
    menuBar->datesGrib_now->setEnabled(false);

    setWindowTitle(tr("qtVlm"));

}
//========================================================================
void MainWindow::slotFile_Load_GRIB()
{
    double x0, y0, x1, y1;
    if (terre->getSelectedRectangle(&x0,&y0, &x1,&y1))
    {
        dialogLoadGrib.setZone(x0, y0, x1, y1);
        dialogLoadGrib.exec();
    }
    else {
        QMessageBox::warning (this,
            tr("Téléchargement d'un fichier GRIB"),
            tr("Vous devez sélectionner une zone de la carte."));
    }
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

//-----------------------------------------------
void MainWindow::slotFile_Info_GRIB()
{
    QString msg;
    if (! terre->getGrib()->isOk())
    {
        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            tr("Aucun fichir GRIB n'est chargé."));
    }
    else {
        Grib * grib = terre->getGrib();

        msg += tr("Fichier : %1\n") .arg(grib->getFileName().c_str());
        msg += tr("Taille : %1 octets\n") .arg(grib->getFileSize());
        msg += tr("\n");

        msg += tr("%1 enregistrements, ").arg(grib->getTotalNumberOfGribRecords());
        msg += tr("%1 dates :\n").arg(grib->getNumberOfDates());

        std::set<time_t> * sdates = grib->getListDates();
        msg += tr("    du %1\n").arg( Util::formatDateTimeLong(*(sdates->begin())) );
        msg += tr("    au %1\n").arg( Util::formatDateTimeLong(*(sdates->rbegin())) );

        msg += tr("\n");
        msg += tr("Données disponibles :\n");
        msg += tr("    Vent  : %1\n").arg(dataPresentInGrib(grib,GRB_WIND_VX));

        GribRecord * gr = grib->getFirstGribRecord();
        msg += tr("\n");
        msg += tr("Grille : %1 points (%2x%3)\n")
                        .arg(gr->getNi()*gr->getNj()).arg(gr->getNi()).arg(gr->getNj());
        msg += tr("\n");
        msg += tr("Etendue :\n");
        QString pos1, pos2;
        pos1 = Util::formatPosition( gr->getX(0), gr->getY(0) );
        pos2 = Util::formatPosition( gr->getX(gr->getNi()-1), gr->getY(gr->getNj()-1) );
        msg += tr("%1  ->  %2\n").arg( pos1, pos2);

        msg += tr("\n");
        msg += tr("Date de référence : %1\n")
                        .arg(Util::formatDateTimeLong(gr->getRecordRefDate()));

        QMessageBox::information (this,
            tr("Informations sur le fichier GRIB"),
            msg );
    }
}

//========================================================================
void MainWindow::slotDateStepChanged(int id)
{
    Util::setSetting("gribDateStep",id);
    updatePrevNext();
}

int MainWindow::getGribStep()
{
    int stepTable[7]={900,1800,3600,7200,10800,21600,43200};
    return stepTable[menuBar->cbGribStep->currentIndex()];
}

void MainWindow::updatePrevNext(void)
{
    if(terre->getGrib()->isOk())
    {
        time_t tps=terre->getCurrentDate();
        time_t min=terre->getGrib()->getMinDate();
        time_t max=terre->getGrib()->getMaxDate();
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
    time_t min=terre->getGrib()->getMinDate();
    time_t max=terre->getGrib()->getMaxDate();
    if(tps<min) tps=min;
    if(tps>max) tps=max;
    terre->setCurrentDate( tps );
    updatePrevNext();
}

void MainWindow::slotDateGribChanged_sel()
{
    terre->showGribDate_dialog();
    updatePrevNext();
}


//-------------------------------------------------
void MainWindow::slotDateGribChanged_next()
{

    if(terre->getGrib()->isOk())
    {
        time_t tps=terre->getCurrentDate();
        time_t max=terre->getGrib()->getMaxDate();
        int step=getGribStep();
        if((tps+step)<=max)
            terre->setCurrentDate(tps+step);
    }
    updatePrevNext();
}
//-------------------------------------------------
void MainWindow::slotDateGribChanged_prev()
{
    if(terre->getGrib()->isOk())
    {
        time_t tps=terre->getCurrentDate();
        time_t min=terre->getGrib()->getMinDate();
        int step=getGribStep();
        if((tps-step)>=min)
            terre->setCurrentDate(tps-step);
    }
    updatePrevNext();
}

void MainWindow::slotSetGribDate(int tps)
{
    if(terre->getGrib()->isOk())
    {
        time_t min=terre->getGrib()->getMinDate();
        time_t max=terre->getGrib()->getMaxDate();
        if(tps>=min && tps <=max)
            terre->setCurrentDate(tps);
    }
}
//-------------------------------------------------
void MainWindow::slotWindArrows(bool b)
{
    // pas de barbules sans flèches
    menuBar->acView_Barbules->setEnabled(b);
}

//-------------------------------------------------
//-------------------------------------------------
void MainWindow::slotWindColors(bool b)
{
    terre->setDrawWindColors(b);
}

//-------------------------------------------------
void MainWindow::statusBar_showSelectedZone()
{
    float x0,y0,  x1,y1;
    terre->getSelectedLine(&x0,&y0, &x1,&y1);

    QString message =
            tr("Sélection: ")
            + Util::formatPosition(x0,y0)
            + " -> "
            + Util::formatPosition(x1,y1);

    Orthodromie orth(x0,y0, x1,y1);

    message = message+ "   "
                + tr("(dist.orthodromique:")
                + Util::formatDistance(orth.getDistance())
                + tr("  init.dir: %1°").arg(qRound(orth.getAzimutDeg()))
                + ")";

    statusBar->showMessage(message);
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
        menuBar->acPilototo->setText(tr("Séléction de POI"));
        VLMBoard->btn_Pilototo->setText(tr("Séléction de POI"));
    }
}

void MainWindow::statusBar_showWindData(double x,double y)
{
    QString s, res;
    double a,b;

    statusBar->clearMessage();
    stBar_label_1->setText( Util::pos2String(TYPE_LAT,y) + ", " + Util::pos2String(TYPE_LON,x));

    if(terre->getGrib()->isOk() && terre->getGrib()->getInterpolatedValue_byDates(x,y,
                                          terre->getGrib()->getCurrentDate(),&a,&b))
    {
        res = "- " + tr("Vent") + ": ";
        s.sprintf("%.0f", radToDeg(b));
        res += s+tr("°")+", ";
        s.sprintf("%.1f",a);
        res += s+" kts";
    }
    stBar_label_2->setText(res);
}

void MainWindow::updateNxtVac(void)
{
    nxtVac_cnt--;
    if(nxtVac_cnt<0)
        nxtVac_cnt=(race_vacLen==0?300:60);
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

void  MainWindow::keyPressEvent (QKeyEvent *e)
{
    switch(e->key())
    {
        case Qt::Key_Escape:
            slotPOIselected(NULL);
            break;
    }
}

//--------------------------------------------------------------
void MainWindow::slotMouseClicked(QMouseEvent * e)
{
    statusBar_showSelectedZone();
    mouseClicX = e->x();
    mouseClicY = e->y();
    switch (e->button()) {
        case Qt::LeftButton : {
            break;
        }
        case Qt::MidButton :   // Centre la carte sur le point
            terre->setCentralPixel(e->x(), e->y());
            break;
        default :
            break;
    }
}
//--------------------------------------------------------------
void MainWindow::slotMouseMoved(QMouseEvent * e) {
    double xx, yy;
    proj->screen2map(e->x(), e->y(), &xx, &yy);
    if (terre->isSelectingZone())
    {
        statusBar_showSelectedZone();
    }
    else
    {
        statusBar_showWindData(xx, yy);
        drawVacInfo();
    }
}

void MainWindow::slotMouseDblClicked(QMouseEvent * e)
{
    mouseClicX = e->x();
    mouseClicY = e->y();
    double lon, lat;
    if(e->button()==Qt::LeftButton)
    {
        proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
        slotAddPOI((float)lat,(float)lon,-1,-1,false);
    }
}

void MainWindow::slotShowContextualMenu(QContextMenuEvent * e)
{
    double a,b,c,d;
    mouseClicX = e->x();
    mouseClicY = e->y();
    if(terre->getSelectedRectangle(&a,&b,&c,&d))
        menuBar->ac_delPOIs->setEnabled(true);
    else
        menuBar->ac_delPOIs->setEnabled(false);
    menuPopupBtRight->exec(QCursor::pos());
}

void MainWindow::slotVLM_ParamBoat(void) {

    boatAcc->initList(acc_list);
    boatAcc->exec();
}

void MainWindow::slotVLM_ParamRace(void)
{
    raceParam->initList(acc_list,race_list);
    raceParam->exec();

}

void MainWindow::slotVLM_Param(void)
{
    param->exec();
}

void MainWindow::slotVLM_Sync(void)
{
    bool hasFirstActivated = (selectedBoat!=NULL);
    QListIterator<boatAccount*> i (acc_list);

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
                selectedBoat=acc;
                acc->selectBoat();
            }
            else
            {
                acc->getData();
            }
        }
    }
}



/*****************************************
 signal send by each boat after it has update
*****************************************/
void MainWindow::slotBoatUpdated(boatAccount * boat,bool newRace)
{
    //qWarning() << "Boat updated " << boat->getLogin();
    if(boat == selectedBoat)
    {
        bool found=false;
        timer->stop();

        /* managing race data: opponnents position and trace*/
        if(newRace || opponents->getRaceId() != boat->getRaceId())
        { /* load a new race */
            for(int i=0;i<race_list.size();i++)
                if(race_list[i]->idrace == boat->getRaceId())
                {
                    race_vacLen=race_list[i]->vac_len;
                    if(!race_list[i]->oppList.isEmpty())
                    {
                        opponents->setBoatList(race_list[i]->oppList,race_list[i]->idrace,false);
                        found=true;
                    }
                    break;
                }
            if(!found)
            {
                opponents->clear();
                race_vacLen=0;
            }
        }
        else /* race has not changed, just refreshing position */
            opponents->refreshData();

        /* centering map on boat */
        terre->setCenterInMap(boat->getLon(),boat->getLat());

        /* updating Vac info */
        nxtVac_cnt=boat->getNextVac();
        drawVacInfo();
        timer->start(1000);

        /* Updating ETA */
        int nbS,j,h,m;
        QString txt;
        QString Eta = boat->getETA();
        QDateTime dtm =QDateTime::fromString(Eta,"yyyy-MM-dd HH:mm:ss");
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
        /* send to all POI the new WP, the corresponding WP if exists will draw in a different color*/
        emit WPChanged(boat->getWPLat(),boat->getWPLon());
    }
}

/*

  if(selectedBoat->getZoom() !=-1)
                    proj->setScale(selectedBoat->getZoom());
  if(acc==selectedBoat)
            {
                menuBar->setSelectedBoatIndex(cnt);
                menuBar->acPilototo->setEnabled(!acc->getLockStatus());
            }
  */

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
            newSelect->getData();
            if(newSelect->getZoom()!=-1)
                proj->setScale(newSelect->getZoom());
            menuBar->acPilototo->setEnabled(!newSelect->getLockStatus());
        }
        else
            menuBar->acPilototo->setEnabled(false);

        /* manage item of boat list */
        int cnt=0;
        for(int i=0;i<acc_list.count();i++)
        {
            if(acc_list[i] == newSelect)
            {
                menuBar->setSelectedBoatIndex(cnt);
                break;
            }
            if(acc_list[i]->getStatus())
                cnt++;
        }

    }
}

/***********************************
  Called when boat list is changed
  *********************************/

void MainWindow::slotChgBoat(int num)
{
    QListIterator<boatAccount*> i (acc_list);
    int cnt=0;
    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        if(acc->getStatus())
        {
            if(cnt==num)
            {
                acc->selectBoat();
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

void MainWindow::slotPOIselected(POI* poi)
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
    menuBar->updateBoatList(acc_list);
    slotVLM_Sync();
}

void MainWindow::slotAddPOI(float lat,float lon, float wph,int timestamp,bool useTimeStamp)
{
    POI * poi;

    poi = new POI(QString(tr("POI")),lon,lat, proj,
                  this, terre,POI_STD,wph,timestamp,useTimeStamp);

    addPOI_list(poi);
    poi->show();
}

void MainWindow::slotpastePOI()
{
    float lon, lat,wph;
    int tstamp;

    if(!Util::getWPClipboard(&lat,&lon,&wph,&tstamp))
        return;

    slotAddPOI(lat,lon,wph,tstamp,tstamp!=-1);
}

void MainWindow::slotChgWP(float lat,float lon, float wph)
{
    if(VLMBoard)
        VLMBoard->setWP(lat,lon,wph);
}

void MainWindow::slotPOIinput(void)
{
    poi_input_dialog->exec();
}

void MainWindow::slotDelPOIs(void)
{
    double lat0,lon0,lat1,lon1;
    double lat,lon;
    if(terre->getSelectedRectangle(&lon0,&lat0,&lon1,&lat1))
    {
        QListIterator<POI*> i (poi_list);
        int num=0,sup=0;

        int rep = QMessageBox::question (this,
            tr("Suppression de POI"),
             tr("La destruction d'un point d'intérêt est définitive.\n\nEtes-vous sûr ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (rep != QMessageBox::Yes)
            return;

        while(i.hasNext())
        {
            POI * poi = i.next();
            lat=poi->getLatitude();
            lon=poi->getLongitude();

            if(lat1<=lat && lat<=lat0 && lon0<=lon && lon<=lon1)
            {
                delPOI_list(poi);
                delete poi;
                sup++;
            }

            num++;
        }
        terre->clearSelection();
    }
}

void MainWindow::slotBoatSave(void)
{
    xmlPOI->writeData(poi_list,"poi.dat");
}

void MainWindow::slotPOIimport(void)
{
    xmlPOI->importZyGrib(poi_list);
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

void MainWindow::slotEditPOI(POI * poi)
{
    emit editPOI(poi);
}

void MainWindow::slotPilototo(void)
{
    if(!selectedBoat) return;

    if(selPOI_instruction)
    {
        slotPOIselected(NULL);
    }
    else
    {
        if(!getBoatLockStatus())
        {
            selectedBoat->getData();
            emit editInstructions();
        }
    }
}

bool MainWindow::isBoat(QString idu)
{
    for(int i=0;i<acc_list.count();i++)
        if(acc_list[i]->getBoatId() == idu)
        {
            if(acc_list[i]->getStatus())
                return true;
            else
                return false;
        }
    return false;
}

void MainWindow::slotReadBoat(void)
{
    qWarning() << "read boat data";
    xmlData->readBoatData(acc_list,race_list,QString("boatAcc.dat"));
    slotAccountListUpdated();
}

void MainWindow::slotWriteBoat(void)
{
    qWarning() << "write boat data";
    xmlData->writeBoatData(acc_list,race_list,QString("boatAcc.dat"));
}

void MainWindow::slotParamChanged(void)
{
    if(Util::getSetting("showCompass",1).toInt()==1)
        compass->show();
    else
        compass->hide();
    emit paramVLMChanged();
}

void MainWindow::getBoatWP(float * lat,float * lon)
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

void MainWindow::slotGetTrace(QString buff,QList<position*> * trace)
{
    emit getTrace(buff,trace);
}


void MainWindow::getPolar(QString fname,Polar ** ptr)
{
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
    qWarning() << "Testing";
}
