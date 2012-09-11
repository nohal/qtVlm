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
#include <QTimer>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QClipboard>

#include "MainWindow.h"
#include "Util.h"
#include "Orthodromie.h"
#include "Version.h"
#include "Board.h"
#include "BoardVLM.h"
#include "BoardReal.h"
#include "settings.h"
#include "opponentBoat.h"
#include "mycentralwidget.h"
#include "MenuBar.h"
#include "GshhsReader.h"
#include "Polar.h"
#include "boatVLM.h"
#include "Grib.h"
#include "GribRecord.h"
#include "POI.h"
#include "Projection.h"
#include "Terrain.h"
#include "boatReal.h"
#include "route.h"

#include "DialogPoiDelete.h"
#include "DialogGribValidation.h"
#include "DialogPoiInput.h"
#include "DialogProxy.h"
#include "DialogVlmGrib.h"
#include "DialogParamVlm.h"
#include "DialogPilototo.h"
#include "dialogviewpolar.h"
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
    connect(mb->ac_delAllPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_delAllPOIs()));
    connect(mb->ac_delSelPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_delSelPOIs()));
    connect(mb->ac_notSimpSelPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_notSimpAllPOIs()));
    connect(mb->ac_simpSelPOIs, SIGNAL(triggered()), my_centralWidget, SLOT(slot_simpAllPOIs()));


    connect(mb->ac_moveBoat, SIGNAL(triggered()), this,SLOT(slot_moveBoat()));

    connect(mb->acFile_Open, SIGNAL(triggered()), this, SLOT(slotFile_Open()));
    connect(mb->acFile_Close, SIGNAL(triggered()), this, SLOT(slotFile_Close()));
    connect(mb->acFile_Open_Current, SIGNAL(triggered()), this, SLOT(slotFile_Open_Current()));
    connect(mb->acFile_Close_Current, SIGNAL(triggered()), this, SLOT(slotFile_Close_Current()));
    connect(mb->acFile_Load_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileLoad_GRIB()));
    connect(mb->acFile_Load_VLM_GRIB, SIGNAL(triggered()), this, SLOT(slotLoadVLMGrib()));
    connect(mb->acFile_Load_SAILSDOC_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slotLoadSailsDocGrib()));
    connect(mb->acFile_Info_GRIB, SIGNAL(triggered()), my_centralWidget, SLOT(slot_fileInfo_GRIB()));
    connect(mb->acFile_Quit, SIGNAL(triggered()), this, SLOT(slotFile_Quit()));
    connect(mb->acFile_QuitNoSave, SIGNAL(triggered()), this, SLOT(slotFile_QuitNoSave()));


    connect(mb->acFax_Open, SIGNAL(triggered()), my_centralWidget, SLOT(slotFax_open()));
    connect(mb->acFax_Close, SIGNAL(triggered()), my_centralWidget, SLOT(slotFax_close()));
    connect(mb->acImg_Open, SIGNAL(triggered()), my_centralWidget, SLOT(slotImg_open()));
    connect(mb->acImg_Close, SIGNAL(triggered()), my_centralWidget, SLOT(slotImg_close()));
    //-------------------------------------------------------
//    connect(mb->acMap_GroupQuality, SIGNAL(triggered(QAction *)),
//            this, SLOT(slotMap_Quality()));

    connect(mb->acMap_Zoom_In, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_In()));
    connect(mb->acMap_Zoom_Out, SIGNAL(triggered()),
            my_centralWidget,  SLOT(slot_Zoom_Out()));
//    menuBar->acMap_Zoom_In->installEventFilter(this);
//    menuBar->acMap_Zoom_Out->installEventFilter(this);
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
    connect(mb->acVLMParamPlayer, SIGNAL(triggered()), my_centralWidget, SLOT(slot_manageAccount()));
    connect(mb->acRace, SIGNAL(triggered()), my_centralWidget, SLOT(slot_raceDialog()));
    connect(mb->acVLMParam, SIGNAL(triggered()), this, SLOT(slotVLM_Param()));
    connect(mb->acVLMSync, SIGNAL(triggered()), this, SLOT(slotVLM_Sync()));

    connect(mb->acPOIAdd, SIGNAL(triggered()), this, SLOT(slot_newPOI()));
    connect(mb->ac_twaLine,SIGNAL(triggered()), my_centralWidget, SLOT(slot_twaLine()));
    connect(mb->ac_compassLine,SIGNAL(triggered()), this, SLOT(slotCompassLine()));
    connect(mb->ac_compassCenterBoat,SIGNAL(triggered()), this, SLOT(slotCompassCenterBoat()));
    connect(mb->ac_compassCenterWp,SIGNAL(triggered()), this, SLOT(slotCompassCenterWp()));
    connect(mb->ac_centerMap,SIGNAL(triggered()), this, SLOT(slot_centerMap()));

    connect(mb->ac_copyRoute,SIGNAL(triggered()), this, SLOT(slot_copyRoute()));
    connect(mb->ac_pasteRoute,SIGNAL(triggered()), this, SLOT(slot_pasteRoute()));
    connect(mb->acRoute_paste,SIGNAL(triggered()), this, SLOT(slot_pasteRoute()));
#ifdef __QTVLM_WITH_TEST
    if(mb->acVLMTest)
        connect(mb->acVLMTest, SIGNAL(triggered()), this, SLOT(slotVLM_Test()));
    if(mb->acGribInterpolation)
        connect(mb->acGribInterpolation, SIGNAL(triggered()), this, SLOT(slotGribInterpolation()));
#endif
    connect(mb->acPOIinput, SIGNAL(triggered()), poi_input_dialog, SLOT(slot_showPOI_input()));
    connect(mb->acPilototo, SIGNAL(triggered()), this, SLOT(slotPilototo()));
    connect(mb->acShowPolar, SIGNAL(triggered()),this,SLOT(slotShowPolar()));

    connect(mb->acPOIimport, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIimport()));
    connect(mb->acPOIgeoData, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIimportGeoData()));

    connect(dialogProxy, SIGNAL(proxyUpdated()), this, SLOT(slotInetUpdated()));

    connect(mb->acPOISave, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POISave()));
    connect(mb->acPOIRestore, SIGNAL(triggered()), my_centralWidget, SLOT(slot_POIRestore()));

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
//bool MainWindow::eventFilter(QObject * /*obj*/, QEvent * e)
//{
//    qWarning()<<"inside event filter";

//    if(e->type()==QEvent::MouseButtonPress)
//    {
//        QMouseEvent *mouseEvent=static_cast<QMouseEvent *>(e);
//        if(mouseEvent->modifiers()==Qt::ShiftModifier)
//        {
//            qWarning()<<"zoom with shift pressed";
//            // soon here: zoom but keep boat position on screen
//        }
//        else
//            qWarning()<<"zoom without shift pressed";
//    }
//    return false;
//}

//----------------------------------------------------
void MainWindow::slot_gribFileReceived(QString fileName)
{
    bool zoom =  (Settings::getSetting("gribZoomOnLoad",0).toInt()==1);
    openGribFile(fileName, zoom);
    updateTitle();
}

//=============================================================
MainWindow::MainWindow(int w, int h, QWidget *parent)
    : QMainWindow(parent)
{
    //debugPOI=NULL;
    restartNeeded=false;
    setWindowIcon (QIcon (appFolder.value("icon")+"qtVlm_48x48.png"));
    noSave=false;
    isStartingUp=true;
    finishStart=true;
    nBoat=0;
    double prcx,prcy,scale;

    updateTitle();
    selectedBoat = NULL;
    showingSelectionMessage=false;
    INTERPOLATION_DEFAULT=Settings::getSetting("defaultInterpolation",INTERPOLATION_HYBRID).toInt();

    //settings = new Settings();

    qWarning() <<  "Starting qtVlm - " << Version::getCompleteName();
    progress=new QProgressDialog(this,Qt::SplashScreen);
    progress->setLabelText("Starting qtVLM");
    progress->setMaximum(100);
    progress->setMinimum(0);
    progress->setCancelButton(NULL);
    progress->setMinimumDuration (0);
    progress->setValue(0);
    progress->show();
    timerprogress=NULL;

    /* timer de gestion des VAC */
    timer = new QTimer(this);
    timer->setSingleShot(false);
    nxtVac_cnt=0;
    connect(timer,SIGNAL(timeout()),this, SLOT(updateNxtVac()));

    prcx = Settings::getSetting("projectionCX", 0.0).toDouble();
    prcy = Settings::getSetting("projectionCY", 0.0).toDouble();
    proj = new Projection (width(), height(),prcx,prcy);
    connect(proj,SIGNAL(newZoom(double)),this,SLOT(slotNewZoom(double)));

    scale = Settings::getSetting("projectionScale", 0.5).toDouble();
    proj->setScale(scale);


    dialogProxy = new DialogProxy();

    //--------------------------------------------------

    progress->setLabelText("initializing menus and toolbars");
    menuBar = new MenuBar(this);
    setMenuBar(menuBar);

    my_centralWidget = new myCentralWidget(proj,this,menuBar);
    menuBar->setMCW(this->my_centralWidget);
    this->setCentralWidget(my_centralWidget);
    connect(this,SIGNAL(addPOI_list(POI*)),my_centralWidget,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(addPOI(QString,int,double,double,double,int,bool,boat*)),
            my_centralWidget,SLOT(slot_addPOI(QString,int,double,double,double,int,bool,boat*)));
    connect(my_centralWidget,SIGNAL(POI_selectAborted(POI*)),this,SLOT(slot_POIselected(POI*)));
    connect(this,SIGNAL(moveBoat(double,double)),my_centralWidget,SLOT(slot_moveBoat(double,double)));
   // connect(this,SIGNAL(updateRoute()),my_centralWidget,SLOT(slot_updateRoute()));

    //--------------------------------------------------

    statusBar =new QStatusBar(this);
    QFontInfo finfo = statusBar->fontInfo();
    QFont font("", finfo.pointSize(), QFont::Normal, false);
    font.setStyleHint(QFont::TypeWriter);
    //font.setStretch(QFont::SemiCondensed);
    font.setFamily("Courier");
    font.setFixedPitch(true);
    statusBar->setFont(font);
    statusBar->setStyleSheet("QStatusBar::item {border: 0px;}");

    stBar_label_1 = new QLabel("Welcome in QtVlm", statusBar);
    stBar_label_1->setFont(font);
    stBar_label_1->setStyleSheet("color: rgb(0, 0, 255);");
    statusBar->addWidget(stBar_label_1);
    font.setBold(true);
    stBar_label_2 = new QLabel("", statusBar);
    stBar_label_2->setFont(font);
    stBar_label_2->setStyleSheet("color: rgb(255, 0, 0);");
    statusBar->addWidget(stBar_label_2);

    font.setBold(false);
    stBar_label_3 = new QLabel("", statusBar);
    stBar_label_3->setFont(font);
    //stBar_label_3->setStyleSheet("color: rgb(255, 0, 0);");
    statusBar->addWidget(stBar_label_3);
    font.setFixedPitch(false);
    //--------------------------------------------------
    toolBar = addToolBar(tr("Outils"));
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->addAction(menuBar->acFile_Quit);
    toolBar->addSeparator();
    toolBar->addAction(menuBar->acFile_Open);
    toolBar->addAction(menuBar->acFile_Load_GRIB);    
    toolBar->addAction(menuBar->acFile_Load_VLM_GRIB);
    toolBar->addAction(menuBar->acFile_Load_SAILSDOC_GRIB);
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

    menuBar->menuFile->addAction(toolBar->toggleViewAction());

    toolBar->addSeparator();
    toolBar->addWidget(menuBar->boatList);
    toolBar->addSeparator();
    tool_ESTIME = new QLabel(tr(" Estime "), toolBar);
    tool_ESTIME->setFont(font);
    tool_ESTIME->setStyleSheet("color: rgb(0, 0, 255);");
    toolBar->addWidget(tool_ESTIME);
    toolBar->addWidget(menuBar->estime);
    tool_ESTIMEUNIT = new QLabel("", toolBar);
    tool_ESTIMEUNIT->setFont(font);
    tool_ESTIMEUNIT->setStyleSheet("color: rgb(0, 0, 255);");
    startEstime=new QCheckBox("", toolBar);
    connect (startEstime,SIGNAL(toggled(bool)),this,SLOT(slotParamChanged()));
    startEstime->setToolTip(tr("Si cette option est cochee<br>l'estime calcule la vitesse du bateau<br>a la prochaine vac.<br>Sinon elle utilise la vitesse du bateau<br>telle que donnee par VLM"));
    startEstime->setChecked(Settings::getSetting("startSpeedEstime", 1).toInt()==1);
    toolBar->addWidget(tool_ESTIMEUNIT);
    toolBar->addWidget(startEstime);
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

    Util::setFontDialog(tool_ETA);
    Util::setFontDialog(tool_ESTIME);
    Util::setFontDialog(tool_ESTIMEUNIT);
    //Util::setFontDialog(statusBar);
    Util::setFontDialog(menuBar);
    //--------------------------------------------------

    setStatusBar(statusBar);
    progress->setValue(10);

    //--------------------------------------------------
    progress->setLabelText("opening grib");
    gribFilePath = Settings::getSetting("gribFilePath", appFolder.value("grib")).toString();
    if(gribFilePath.isEmpty())
        gribFilePath = appFolder.value("grib");
    QString fname = Settings::getSetting("gribFileName", "").toString();
    if (fname != "" && QFile::exists(fname))
    {
   //     qWarning() << "Opening grib :" << fname;
        openGribFile(fname, false);
        gribFileName=fname;
   //     qWarning() << "Grib opened";
    }
    fname = Settings::getSetting("gribFileNameCurrent", "").toString();
    if (fname != "" && QFile::exists(fname))
    {
   //     qWarning() << "Opening grib :" << fname;
        openGribFile(fname, false,true);
        gribFileNameCurrent=fname;
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
    polar_list = new polarList(my_centralWidget->getInet(),this);
//    progress->setValue(30);
    progress->setLabelText("reading boats data");
    my_centralWidget->loadBoat();
    progress->setValue(60);

    progress->setLabelText("Drawing some");

    poi_input_dialog = new DialogPoiInput(my_centralWidget);

    selPOI_instruction=NULL;
    isSelectingWP=false;

    //menuBar->updateBoatList(my_centralWidget->getBoats());

    myBoard = new board(this,my_centralWidget->getInet(),statusBar);
    connect(menuBar->acOptions_SH_ComBandeau,SIGNAL(triggered()),myBoard,SLOT(slot_hideShowCompass()));

    param = new DialogParamVlm(this,my_centralWidget);
    connect(param,SIGNAL(paramVLMChanged()),myBoard,SLOT(paramChanged()));
    connect(this,SIGNAL(wpChanged()),myBoard->VLMBoard(),SLOT(update_btnWP()));
    connect(param,SIGNAL(paramVLMChanged()),this,SLOT(slot_ParamVLMchanged()));

    progress->setLabelText("Preparing coffee");

    pilototo = new DialogPilototo(this,my_centralWidget,my_centralWidget->getInet());

    loadVLM_grib = new DialogVlmGrib(this,my_centralWidget,my_centralWidget->getInet());

    //---------------------------------------------------------
    // Active les actions
    //---------------------------------------------------------
    connectSignals();

    /* initialisation du niveau de qualité */

    //int quality = Settings::getSetting("gshhsMapQuality", 1).toInt();
    int quality=4;
//    for (int qual=4; qual>=0; qual--)
//    {
//        if (! my_centralWidget->get_gshhsReader()->gshhsFilesExists(qual))
//        {
//            switch (qual) {
//                case 0: menuBar->acMap_Quality1->setEnabled(false); break;
//                case 1: menuBar->acMap_Quality2->setEnabled(false); break;
//                case 2: menuBar->acMap_Quality3->setEnabled(false); break;
//                case 3: menuBar->acMap_Quality4->setEnabled(false); break;
//                case 4: menuBar->acMap_Quality5->setEnabled(false); break;
//            }
//            if (quality >= qual)
//            {
//                quality = qual-1;
//            }
//        }
//    }

    /*if (quality < 0)
    {
        QMessageBox::information (this,
            QString(tr("Erreur")),
            QString(tr("Cartes non trouvees.\n\n")
                    +tr("Verifiez l'installation du programme."))
        );
        quality = 0;
    }*/
    //menuBar->setQuality(quality);
    emit signalMapQuality(quality);

    //------------------------------------------------
    // sync all boat


    progress->setLabelText("Drawing all");
    progress->setValue(90);

     //--------------------------------------------------
    // get screen geometry
    QDesktopWidget * desktopWidget = QApplication::desktop ();

    //qWarning() << "Display info:";
    //qWarning() << "Number of screen: " << desktopWidget->screenCount();
//    for(int i=0;i < desktopWidget->screenCount(); i++)
//    {
//        qWarning() << i << ": " << desktopWidget->screenGeometry(i) << (i==desktopWidget->primaryScreen()?" (qtVlm screen)":" (Other screen)");
//    }

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
        myBoard->floatingBoard(false);

    if(Settings::getSetting("saveMainWindowGeometry","1").toInt())
    {
        QSize savedSize = Settings::getSetting("mainWindowSize", QSize(w,h)).toSize();

        //qWarning() << "Have saved size: " << savedSize;

        if(savedSize.height()>screenRect.height() || savedSize.width() > screenRect.width())
        {
            move(QPoint(0,0));
            showMaximized();
        }
        else
        {
            //qWarning() << "Resizing to saved size";
            resize( Settings::getSetting("mainWindowSize", QSize(w,h)).toSize() );
            move  ( Settings::getSetting("mainWindowPos", QPoint()).toPoint() );
            if(Settings::getSetting("mainWindowMaximized","0").toInt()==1)
                showMaximized();
        }
    }
    else
        showMaximized ();



    /* init du dialog de validation de grib (present uniquement en mode debug)*/
#ifdef __QTVLM_WITH_TEST
    gribValidation_dialog = new DialogGribValidation(my_centralWidget,this);
#endif
    //********************************************

    //slot_deleteProgress();

    QList<Player*> players=my_centralWidget->getPlayers();

#ifdef __REAL_BOAT_ONLY
    if(players.count()==0) {
        Player * newPlayer = new Player("MyBoat","",BOAT_REAL,0,"MyBoat",proj,this,my_centralWidget,my_centralWidget->getInet());
        my_centralWidget->slot_addPlayer_list(newPlayer);
        players=my_centralWidget->getPlayers();
    }

#endif


    if(players.count()==1)
    {
        myBoard->playerChanged(players.at(0));
        if(players.at(0)->getType()==BOAT_VLM)
        {
            progress->setLabelText("Updating player");
            progress->setValue(91);
            connect(players.at(0),SIGNAL(playerUpdated(bool,Player*)),this,SLOT(slot_updPlayerFinished(bool,Player*)));
            players.at(0)->updateData();
        }
        else
        {
            my_centralWidget->slot_playerSelected(players.at(0));
            my_centralWidget->loadPOI();
            isStartingUp=false;
            updateTitle();
            slot_deleteProgress();            
            my_centralWidget->emitUpdateRoute(NULL);
        }
        //Util::setFontDialog(statusBar);
        Util::setFontDialog(menuBar);
        return;
    }



    bool res;
    progress->setLabelText("Calling player dialog");
    progress->setValue(92);

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
                    progress->setLabelText("Updating boats (1)");
                    progress->setValue(95);
                    VLM_Sync_sync();
                    timerprogress=new QTimer();
                    timerprogress->setSingleShot(true);
                    timerprogress->setInterval(5000);
                    connect(timerprogress,SIGNAL(timeout()),this, SLOT(slot_deleteProgress()));
                    timerprogress->start();
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
            updateTitle();
            my_centralWidget->emitUpdateRoute(NULL);
        }
    }

    slot_deleteProgress();
    //Util::setFontDialog(statusBar);
    Util::setFontDialog(menuBar);
}

//-----------------------------------------------
void MainWindow::listAllChildren(QObject * ptr,int depth=0)
{
    if(!ptr) ptr=this;
    QObjectList childList=ptr->children();
    if(childList.count()!=0)
    {
        for(int i=0;i<childList.count();i++)
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
bool MainWindow::getStartEstimeSpeedFromGrib()
{
    return this->startEstime->isChecked();
}

//-----------------------------------------------
MainWindow::~MainWindow()
{
    //--------------------------------------------------
    // Save global settings
    //--------------------------------------------------
    my_centralWidget->setAboutToQuit();
    if(noSave) return;
    if(Settings::getSetting("saveMainWindowGeometry","1").toInt())
    {
        //qWarning() << "Saving window geometry: " << size() << " " << pos();
        Settings::setSetting("mainWindowSize", size());
        Settings::setSetting("mainWindowPos", pos());
        Settings::setSetting("mainWindowMaximized",this->isMaximized()?"1":"0");
    }
    Settings::setSetting("projectionCX", proj->getCX());
    Settings::setSetting("projectionCY", proj->getCY());
    Settings::setSetting("projectionScale",  proj->getScale());
    Settings::setSetting("gribFileName",  gribFileName);
    Settings::setSetting("gribFileNameCurrent",  gribFileNameCurrent);
    Settings::setSetting("gribFilePath",  gribFilePath);
    Settings::setSetting("startSpeedEstime",startEstime->isChecked()?1:0);
    /*freeze all routes*/
    //my_centralWidget->freezeRoutes(true);
    if(selectedBoat) /* save the zoom factor */
        selectedBoat->setZoom(proj->getScale());
//    if(my_centralWidget)
//        delete my_centralWidget;

}

void MainWindow::keyPressEvent ( QKeyEvent  * /* event */ )
{
    //qWarning() << "Key pressed in main: " << event->key();
}
void MainWindow::slot_deleteProgress (void)
{
    //qWarning() << "Removing progress";
    progress->close();
    delete progress;
    if(timerprogress)
        delete timerprogress;
    if(restartNeeded)
        this->my_centralWidget->setAboutToQuit();
    else if(selectedBoat && selectedBoat->getType()==BOAT_REAL)
    {
        proj->setScaleAndCenterInMap(selectedBoat->getZoom(),selectedBoat->getLon(),selectedBoat->getLat());
        if(Settings::getSetting("polarEfficiency",100).toInt()!=100)
        {
            selectedBoat->reloadPolar(true);
            emit boatHasUpdated(selectedBoat);
        }
    }

}

//-------------------------------------------------
void MainWindow::openGribFile(QString fileName, bool zoom, bool current)
{
    Grib * myGrib;
    bool badCurrent=false;
    if(current)
    {
        my_centralWidget->loadGribFileCurrent(fileName, zoom);
        myGrib=my_centralWidget->getGribCurrent();
        if(myGrib && myGrib->getNumberOfGribRecords(GRB_CURRENT_VX,LV_MSL,0)==0)
        {
            slotFile_Close_Current();
            badCurrent=true;
            myGrib=NULL;
        }
    }
    else
    {
        my_centralWidget->loadGribFile(fileName, zoom);
        myGrib=my_centralWidget->getGrib();
    }
    if (myGrib && !badCurrent)
    {
        slotDateGribChanged_now();
        if(!current)
        {
            gribFileName = fileName;
            Settings::setSetting("gribFileName",  gribFileName);
            Settings::setSetting("gribFilePath",  gribFilePath);
        }
        else
        {
            gribFileNameCurrent = fileName;
            Settings::setSetting("gribFileNameCurrent",  gribFileNameCurrent);
            Settings::setSetting("gribFilePath",  gribFilePath);
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
    if(!my_centralWidget->getGrib() && !my_centralWidget->getGribCurrent())
    {
        menuBar->cbGribStep->setEnabled(false);
        menuBar->acDatesGrib_prev->setEnabled(false);
        menuBar->acDatesGrib_next->setEnabled(false);
        menuBar->datesGrib_sel->setEnabled(false);
        menuBar->datesGrib_now->setEnabled(false);
    }
    else
    {
        menuBar->cbGribStep->setEnabled(true);
        menuBar->acDatesGrib_prev->setEnabled(true);
        menuBar->acDatesGrib_next->setEnabled(true);
        menuBar->datesGrib_sel->setEnabled(true);
        menuBar->datesGrib_now->setEnabled(true);
    }
    updateTitle();
}
void MainWindow::updateTitle()
{
    QString ver="qtVlm "+QString().setNum(sizeof(int*)*8)+" bits "+Version::getVersion();

    if(isStartingUp) {
        setWindowTitle(ver);
        return;
    }

    Grib * g=my_centralWidget->getGrib();
    Grib * gc=my_centralWidget->getGribCurrent();

    QString g1,g2;
    if(g && g->isOk())
    {
        QFileInfo i(gribFileName);
        QDateTime startGribDate=QDateTime().fromTime_t(g->getMinDate()).toUTC();
        startGribDate.setTimeSpec(Qt::UTC);
        QDateTime endGribDate=QDateTime().fromTime_t(g->getMaxDate()).toUTC();
        endGribDate.setTimeSpec(Qt::UTC);
        g1 = " grib: "+ i.fileName() +tr(" (du ")+
                   startGribDate.toString(tr("dd/MM/yyyy hh:mm"))+tr(" au ")+
                   endGribDate.toString(tr("dd/MM/yyyy hh:mm"))+")";
    }
    if(gc && gc->isOk())
    {
        QFileInfo i(gribFileNameCurrent);
        QDateTime startGribDate=QDateTime().fromTime_t(gc->getMinDate()).toUTC();
        startGribDate.setTimeSpec(Qt::UTC);
        QDateTime endGribDate=QDateTime().fromTime_t(gc->getMaxDate()).toUTC();
        endGribDate.setTimeSpec(Qt::UTC);
        g2 = tr(" courant: ")+ i.fileName() +tr(" (du ")+
                   startGribDate.toString(tr("dd/MM/yyyy hh:mm"))+tr(" au ")+
                   endGribDate.toString(tr("dd/MM/yyyy hh:mm"))+")";
    }
    setWindowTitle(ver+g1+g2);
}

//-------------------------------------------------
// SLOTS
//-------------------------------------------------

void MainWindow::slotUpdateOpponent(void)
{
    bool found=false;
    if(!selectedBoat || selectedBoat->getType()!=BOAT_VLM)
    {
        my_centralWidget->getOppList()->clear();
        return;
    }

    for(int i=0;i<my_centralWidget->getRaces().size();i++)
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
    double lon, lat;
    proj->screen2map(mouseClicX,mouseClicY, &lon, &lat);
    emit newPOI(lon,lat,proj,selectedBoat);
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
        Settings::setSetting("appLanguage", lang);
        QMessageBox::information (this,
            QString("Changement de langue"),
            QString("Langue : FranÃ§ais\n\n")
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
    else if (act == mb->acOptions_Lang_cz) {
        lang = "cz";
        Settings::setSetting("appLanguage", lang);
        QMessageBox::information (this,
            QString("Application Language"),
            QString("Language : English\n\n")
              + QString("Please reload application to activate language.\n")
        );
    }
}
//-------------------------------------------------
//void MainWindow::slotMap_Quality()
//{
//    int quality = 0;
//    MenuBar  *mb = menuBar;
//    QAction *act = mb->acMap_GroupQuality->checkedAction();
//    if (act == mb->acMap_Quality1)
//        quality = 0;
//    else if (act == mb->acMap_Quality2)
//        quality = 1;
//    else if (act == mb->acMap_Quality3)
//        quality = 2;
//    else if (act == mb->acMap_Quality4)
//        quality = 3;
//    else if (act == mb->acMap_Quality5)
//        quality = 4;

//    Settings::setSetting("gshhsMapQuality", quality);
//    emit signalMapQuality(quality);
//}
//-------------------------------------------------



//-------------------------------------------------
void MainWindow::slotHelp_Help() {
    QDesktopServices::openUrl(QUrl("http://wiki.virtual-loup-de-mer.org/index.php/QtVlm#L.27interface_de_qtVlm"));
}

//-------------------------------------------------
void MainWindow::slotHelp_APropos() {
    QMessageBox::about (this,
            tr("A propos"),
            tr("qtVlm : GUI pour Virtual loup de mer")
            +"\nhttp://www.virtual-loup-de-mer.org\n"+

            tr("Version : ")+Version::getVersion()
                    +"      "+Version::getDate()
            +"\n"+ tr("Licence : GNU GPL v3")
            +"\n"+ "http://qtvlm.sf.net"
            +"\n"+ "http://virtual-winds.com/forum"
            +"\n"+ tr("Grib part is originaly from zygrib project")
            +"\n"+ "http://www.zygrib.org"
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
void MainWindow::slotFile_QuitNoSave() {
    my_centralWidget->setAboutToQuit();
    noSave=true;
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
        gribFilePath=appFolder.value("grib");
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
    updateTitle();
}
void MainWindow::slotFile_Open_Current()
{
    QString filter;
    filter =  tr("Fichiers GRIB (*.grb *.grib *.grb.bz2 *.grib.bz2 *.grb.gz *.grib.gz)")
            + tr(";;Autres fichiers (*)");
    QDir dirGrib(gribFilePath);
    if(!dirGrib.exists())
    {
        gribFilePath=appFolder.value("grib");
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
        openGribFile(fileName, zoom, true);
    }
    updateTitle();
}
//-------------------------------------------------
void MainWindow::slotFile_Close_Current() {
    gribFileNameCurrent = "";
    my_centralWidget->loadGribFileCurrent("", false);
    if(!(my_centralWidget->getGrib() && my_centralWidget->getGrib()->isOk()))
    {
        menuBar->acDatesGrib_prev->setEnabled(false);
        menuBar->acDatesGrib_next->setEnabled(false);
        menuBar->cbGribStep->setEnabled(false);
        menuBar->datesGrib_sel->setEnabled(false);
        menuBar->datesGrib_now->setEnabled(false);

    }
    updateTitle();
    my_centralWidget->emitUpdateRoute(NULL);
}
//-------------------------------------------------
void MainWindow::slotFile_Close()
{
    gribFileName = "";
    my_centralWidget->loadGribFile("", false);
    if(!(my_centralWidget->getGribCurrent() && my_centralWidget->getGribCurrent()->isOk()))
    {
        menuBar->acDatesGrib_prev->setEnabled(false);
        menuBar->acDatesGrib_next->setEnabled(false);
        menuBar->cbGribStep->setEnabled(false);
        menuBar->datesGrib_sel->setEnabled(false);
        menuBar->datesGrib_now->setEnabled(false);

    }
    updateTitle();
    my_centralWidget->emitUpdateRoute(NULL);
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
void MainWindow::slotDateGribChanged_now(bool b)
{
    time_t tps=QDateTime::currentDateTime().toUTC().toTime_t();
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t min=grib->getMinDate();
        time_t max=grib->getMaxDate();
        if(tps<min) tps=min;
        if(tps>max) tps=max;
        my_centralWidget->setCurrentDate( tps, b );
        updatePrevNext();
    }
}

void MainWindow::slotDateGribChanged_sel()
{
    my_centralWidget->showGribDate_dialog();
    updatePrevNext();
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
}

void MainWindow::slotSetGribDate(time_t tps)
{
    Grib * grib = my_centralWidget->getGrib();
    if(grib)
    {
        time_t min=grib->getMinDate();
        time_t max=grib->getMaxDate();
        if(tps>=min && tps <=max)
            my_centralWidget->setCurrentDate(tps);
    }
}
//-------------------------------------------------
Grib * MainWindow::getGrib()
{
    return my_centralWidget->getGrib();
}

void MainWindow::slotWindArrows(bool b)
{
    // pas de barbules sans fleches
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
    if(selectedBoat)
       return selectedBoat->getVacLen();
    else
        return 1;
}
void MainWindow::updatePilototo_Btn(boatVLM * boat)
{
    if(!selPOI_instruction)
    {
        /* compute nb Pilototo instructions */
        QStringList * lst = boat->getPilototo();
        QString pilototo_txt=tr("Pilototo");
        QString pilototo_toolTip="";
        myBoard->VLMBoard()->set_style(myBoard->VLMBoard()->btn_Pilototo,QColor(255, 255, 127));
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
            {
                pilototo_txt=pilototo_txt+" ("+QString().setNum(nbPending)+"/"+QString().setNum(nb)+")";
                if(nbPending!=0)
                {
                    myBoard->VLMBoard()->set_style(myBoard->VLMBoard()->btn_Pilototo,QColor(14,184,63),QColor(255, 255, 127));
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
        myBoard->VLMBoard()->btn_Pilototo->setText(pilototo_txt);
        myBoard->VLMBoard()->btn_Pilototo->setToolTip(pilototo_toolTip);
    }
    else
    {
        menuBar->acPilototo->setText(tr("Selection d'une marque"));
        myBoard->VLMBoard()->btn_Pilototo->setText(tr("Selection d'une marque"));
    }
}

void MainWindow::statusBar_showWindData(double x,double y)
{
#if 0 /*unflag to visualize closest point to next gate from mouse position*/
    if(!selectedBoat) return;
    QList<vlmLine*> gates=((boatVLM*)this->selectedBoat)->getGates();
    int nWP=this->selectedBoat->getNWP();

    if(gates.isEmpty() || nWP<=0 || nWP>gates.count())
    {
    }
    else
    {
        vlmLine *porte=NULL;
        for (int i=nWP-1;i<gates.count();++i)
        {
            porte=gates.at(i);
            if (!porte->isIceGate()) break;
        }
        double X,Y;
        proj->map2screenDouble(x,y,&X,&Y);
        double cx=X;
        double cy=Y;
        proj->map2screenDouble(porte->getPoints()->first().lon,porte->getPoints()->first().lat,&X,&Y);
        double ax=X;
        double ay=Y;
        proj->map2screenDouble(porte->getPoints()->last().lon,porte->getPoints()->last().lat,&X,&Y);
        double bx=X;
        double by=Y;
    #if 1 /*remove 1 pixel at each end to make sure we cross*/
        QLineF porteLine(ax,ay,bx,by);
        QLineF p1(porteLine.pointAt(0.5),porteLine.p1());
        p1.setLength(p1.length()-1);
        QLineF p2(porteLine.pointAt(0.5),porteLine.p2());
        p2.setLength(p2.length()-1);
        ax=p1.p2().x();
        ay=p1.p2().y();
        bx=p2.p2().x();
        by=p2.p2().y();
    #endif
        double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
        double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
        double r = r_numerator / r_denomenator;
    //
        double px = ax + r*(bx-ax);
        double py = ay + r*(by-ay);
    //


    //
    // (xx,yy) is the point on the lineSegment closest to (cx,cy)
    //
        double xx = px;
        double yy = py;
        if ( (r >= 0) && (r <= 1) )
        {
        }
        else
        {
            double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
            double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
            if (dist1 < dist2)
            {
                    xx = ax;
                    yy = ay;
            }
            else
            {
                    xx = bx;
                    yy = by;
            }

        }
        double a,b;
        proj->screen2mapDouble(xx,yy,&a,&b);
//        closest=vlmPoint(a,b);
//        Orthodromie oo(lon,lat,a,b);
//        closest.distArrival=oo.getDistance();
//        closest.capArrival=oo.getAzimutDeg();
        if(debugPOI==NULL)
            debugPOI = this->my_centralWidget->slot_addPOI("debug",0,b,a,-1,false,false,this->selectedBoat);
        else
        {
            debugPOI->setLatitude(b);
            debugPOI->setLongitude(a);
            debugPOI->slot_updateProjection();
            QApplication::processEvents();
        }

    }
#endif
    QString s, res;
    double a,b;

    if(showingSelectionMessage)
    {
        showingSelectionMessage=false;
        statusBar->clearMessage();
    }

    if(!statusBar->currentMessage().isEmpty())
        return;

    QString label1= Util::pos2String(TYPE_LAT,y) + ", " + Util::pos2String(TYPE_LON,x);
    if(this->getSelectedBoat())
    {
        Orthodromie oo(this->getSelectedBoat()->getLon(),this->getSelectedBoat()->getLat(),x,y);
        label1=label1+QString().sprintf(" - %6.2f",oo.getAzimutDeg())+tr("deg")+
                QString().sprintf("/%7.2fNM",oo.getDistance());
    }
    stBar_label_1->setText(label1);

    Grib * grib = my_centralWidget->getGrib();
    bool bo=false;
    res.clear();
    bo=(grib && grib->getInterpolatedValue_byDates(x,y,grib->getCurrentDate(),&a,&b));
    if(bo)
    {
        res = "- " + tr(" Vent") + ": ";
        s.sprintf("%6.2f", radToDeg(b));
        res += s+tr("deg")+", ";
        s.sprintf("%6.2f",a);
        res += s+tr(" kts");
    }
    bo=(grib && grib->getInterpolatedValueCurrent_byDates(x,y,grib->getCurrentDate(),&a,&b));
    if(bo)
    {
        res += " - " + tr(" Courant") + ": ";
        s.sprintf("%6.2f", Util::A360(radToDeg(b)+180.0));
        res += s+tr("deg")+", ";
        s.sprintf("%6.2f",a);
        res += s+tr(" kts");
    }
    else
    {
        grib=my_centralWidget->getGribCurrent();
        bo=(grib && grib->getInterpolatedValueCurrent_byDates(x,y,grib->getCurrentDate(),&a,&b));
        if(bo)
        {
            res += " - " + tr(" Courant") + ": ";
            s.sprintf("%6.2f", Util::A360(radToDeg(b)+180.0));
            res += s+tr("deg")+", ";
            s.sprintf("%6.2f",a);
            res += s+tr(" kts");
        }
    }
    stBar_label_2->setText(res);
}

void MainWindow::statusBar_showSelectedZone(double x0, double y0, double x1, double y1)
{
    QString message =
            tr("Selection: ")
            + Util::formatPosition(x0,y0)
            + " -> "
            + Util::formatPosition(x1,y1);

    Orthodromie orth(x0,y0, x1,y1);
    QString s;
    message = message+ "   "
                + tr("(dist.orthodromique:")
                + Util::formatDistance(orth.getDistance())
//                + tr("  init.dir: %1deg").arg(qRound(orth.getAzimutDeg()))
                + tr("  init.dir: %1deg").arg(s.sprintf("%.1f",orth.getAzimutDeg()))
                + ")";

    showingSelectionMessage=true;
    statusBar->showMessage(message);
}

void MainWindow::updateNxtVac(void)
{
    if(!selectedBoat || selectedBoat->getType()!=BOAT_VLM)
    {
        nxtVac_cnt=0;
    }
    else
    {
        nxtVac_cnt--;
        if(nxtVac_cnt<0) {
            nxtVac_cnt=selectedBoat->getVacLen();
            myBoard->outdatedVLM();
        }
    }
    drawVacInfo();
}
QList<POI*> * MainWindow::getPois()
{
    return my_centralWidget->getPoisList();
}

void MainWindow::drawVacInfo(void)
{
    if(selectedBoat && selectedBoat->getType()==BOAT_VLM && statusBar->currentMessage().isEmpty())
    {
        QDateTime lastVac_date;
        lastVac_date.setTimeSpec(Qt::UTC);
        lastVac_date.setTime_t(((boatVLM*)selectedBoat)->getPrevVac());
        stBar_label_3->setText("- "+ tr("Derniere synchro") + ": " + lastVac_date.toString(tr("dd-MM-yyyy, HH:mm:ss")) + " - "+
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
        menuBar->ac_simpSelPOIs->setEnabled(true);
        menuBar->ac_notSimpSelPOIs->setEnabled(true);
    }
    else
    {
        menuBar->ac_delAllPOIs->setEnabled(false);
        menuBar->ac_delSelPOIs->setEnabled(false);
        menuBar->ac_simpSelPOIs->setEnabled(false);
        menuBar->ac_notSimpSelPOIs->setEnabled(false);
    }

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
        menuBar->ac_copyRoute->setEnabled(true);
        menuBar->ac_copyRoute->setData(my_centralWidget->getRouteToClipboard()->getName());
    }
    else
    {
        menuBar->ac_copyRoute->setEnabled(false);
        menuBar->ac_copyRoute->setData(QString());
    }
    QString clipboard=QApplication::clipboard()->text();
    if(clipboard.isEmpty() || !clipboard.contains("<kml") || !clipboard.contains("Placemark"))
        menuBar->ac_pasteRoute->setEnabled(false);
    else
        menuBar->ac_pasteRoute->setEnabled(true);

    menuPopupBtRight->exec(QCursor::pos());
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
void MainWindow::slot_pasteRoute()
{
    my_centralWidget->importRouteFromMenuKML("",true);
}
void MainWindow::slotInetUpdated(void)
{
    //qWarning() << "Inet Updated";
    emit updateInet();
    slotVLM_Sync();
}

void MainWindow::slot_centerMap()
{
    proj->setCentralPixel(this->mouseClicX,this->mouseClicY);
}

void MainWindow::slotCompassLine(void)
{
//    QPoint cursor_pos=my_centralWidget->mapFromGlobal(cursor().pos());
    emit showCompassLine((double)mouseClicX,(double)mouseClicY);
}
void MainWindow::slotCompassLineForced(double a, double b)
{
//    QPoint cursor_pos=my_centralWidget->mapFromGlobal(cursor().pos());
    emit showCompassLine(a,b);
}
void MainWindow::slotCompassCenterBoat(void)
{
    Settings::setSetting("compassCenterBoat", menuBar->ac_compassCenterBoat->isChecked()?"1":"0");
    emit showCompassCenterBoat();
}
void MainWindow::slotCompassCenterWp(void)
{
    menuBar->ac_compassCenterBoat->setChecked(false);
    Settings::setSetting("compassCenterBoat", "0");
    emit showCompassCenterWp();
}

void MainWindow::slotVLM_Param(void)
{
    param->changeParam();
    param->exec();
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
    for(int n=0;n<listBoats.count();n++)
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
        return ;
    }

    QList<boatVLM*> listBoats = *my_centralWidget->getBoats();
    if(listBoats.count()==0) return;
    menuBar->boatList->setEnabled(false);
    nBoat--;
    if(nBoat>=0)
    {
        acc = listBoats.at(nBoat);
        if(acc->getStatus() || !acc->isInitialized())
        {
            //qWarning() << "Doing a synch_synch with VLM: " << acc->getName();
            connect(acc,SIGNAL(hasFinishedUpdating(void)),this,SLOT(slot_boatHasUpdated()));
            //toBeCentered=nBoat;
//            if(selectedBoat==NULL)
//                acc->slot_selectBoat();
//            else
                acc->slot_getData(true);
        }
        else
        {
            //qWarning() << "Calling again SyncSync with next boat";
            VLM_Sync_sync();
        }
    }
    else
    {
        int lastBoatSelected=Settings::getSetting("LastBoatSelected","-10").toInt();
        if(lastBoatSelected!=-10)
        {
            bool found=false;
            for(nBoat=0;nBoat<listBoats.count();nBoat++)
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
                Settings::setSetting("LastBoatSelected","-10");
            }
        }
        if(lastBoatSelected==-10)
        {
            for(nBoat=0;nBoat<listBoats.count();nBoat++)
            {
                if(listBoats.at(nBoat)->getStatus())
                {
                    listBoats.at(nBoat)->slot_selectBoat();
                    Settings::setSetting("LastBoatSelected",listBoats.at(nBoat)->getId());
                    break;
                }
            }
        }
        menuBar->boatList->setEnabled(true);
        slotDateGribChanged_now(false);
        isStartingUp=false;
        if(Settings::getSetting("centerOnBoatChange","1").toInt()==1)
            this->slot_centerBoat();
        my_centralWidget->emitUpdateRoute(NULL);
        updateTitle();
    }
}
void MainWindow::slot_boatHasUpdated()
{
     disconnect(acc,SIGNAL(hasFinishedUpdating(void)),this,SLOT(slot_boatHasUpdated()));
     //qWarning() << "slot_boatHasUpdated => syncSync";
     VLM_Sync_sync();
}

/*****************************************
 signal send by each boat after it has update
*****************************************/
void MainWindow::slotBoatUpdated(boat * upBoat,bool newRace,bool doingSync)
{
    //qWarning() << "Boat updated " << boat->getBoatPseudo();

    if(upBoat->getType()==BOAT_VLM)
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
            bool found=false;
            //qWarning() << "selected boat update: " << boat->getName();
            timer->stop();
            /* managing race data: opponnents position and trace*/
            int i=0;
            for(i=0;i<my_centralWidget->getRaces().size();i++)
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
                    if(Settings::getSetting("centerOnSynch",0).toInt()==1)
                        proj->setCenterInMap(boat->getLon(),boat->getLat());
                }
                else
                {
                    if(Settings::getSetting("centerOnBoatChange",1).toInt()==1)
                        proj->setScaleAndCenterInMap(boat->getZoom(),boat->getLon(),boat->getLat());
                }
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
            txt.replace("j",tr("j"));
            txt.replace("h",tr("h"));
            txt.replace("m",tr("m"));
            txt.replace("s",tr("s"));
            tool_ETA->setText(tr(" Arrivee WP")+": " +dtm.toString(tr("dd-MM-yyyy, HH:mm:ss"))+ " " +txt);

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
//        qWarning() << "Real boat has updated";
        /*if(my_centralWidget->getOppList())
        {
            my_centralWidget->getOppList()->clear();
            my_centralWidget->getOppList()->refreshData();
        }*/
        /* Updating ETA */
        boatReal *boat=(boatReal *) upBoat;
        if((boat->getWPLat()==0 && boat->getWPLon()==0) ||boat->getEta()==-1)
            tool_ETA->clear();
        else
        {
            int nbS,j,h,m;
            QString txt;
            QDateTime dtm =QDateTime::fromTime_t(boat->getEta()).toUTC();
            if(!dtm.isValid())
                tool_ETA->clear();
            else
            {
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
                txt.replace("j",tr("j"));
                txt.replace("h",tr("h"));
                txt.replace("m",tr("m"));
                txt.replace("s",tr("s"));
                tool_ETA->setText(tr(" Arrivee WP")+": " +dtm.toString(tr("dd-MM-yyyy, HH:mm:ss"))+ " " +txt);
            }
        }
        emit boatHasUpdated(upBoat);
        //emit WPChanged(upBoat->getWPLat(),upBoat->getWPLon());
    }

}

void MainWindow::slotSelectBoat(boat* newSelect)
{
    if(!newSelect->getStatus()) return;
    if(!my_centralWidget->getBoats() && newSelect->getType() == BOAT_VLM)
    {
        qWarning() << "CRITICAL: slotSelectBoat - empty boatList";
        return ;
    }
    if(newSelect->getType()!=BOAT_VLM)
    {
        selectedBoat=newSelect;
        selectedBoat->slot_selectBoat();
        selectedBoat->setZoom(proj->getScale());
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
        selectedBoat->slot_selectBoat();

        /* change the board ? */
        //emit boatChanged(selectedBoat);

        if(newSelect->getStatus()) /* is selected boat activated ?*/
        {            
            if(newSelect->getType()==BOAT_VLM)
            {
                //qWarning() << "getData from slot_selectBoat";
                ((boatVLM*)newSelect)->slot_getData(false);
                menuBar->acPilototo->setEnabled(!newSelect->getLockStatus());
            }

            slotDateGribChanged_now(false);
            //proj->setScaleAndCenterInMap(newSelect->getZoom(),newSelect->getLon(),newSelect->getLat());
        }
        else
        {
            if(newSelect->getType()==BOAT_VLM)
                menuBar->acPilototo->setEnabled(false);
        }

        /* manage item of boat list */
        if(newSelect->getType()==BOAT_VLM)
        {
            int cnt=0;
            for(int i=0;i<my_centralWidget->getBoats()->count();i++)
            {
                if(my_centralWidget->getBoats()->at(i) == newSelect)
                {
                    menuBar->setSelectedBoatIndex(cnt);
                    break;
                }
                if(my_centralWidget->getBoats()->at(i)->getStatus())
                    cnt++;
            }
        }

    }
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
                Settings::setSetting("LastBoatSelected", acc->getId());
                acc->slot_selectBoat();
                /* sync lunched, update grib date */
                //slotDateGribChanged_now();
                emit selectedBoatChanged();
                for(int i=0;i<my_centralWidget->getRaces().size();i++)
                {
                    if(my_centralWidget->getRaces()[i]->idrace == acc->getRaceId())
                    {
                        my_centralWidget->drawNSZ(i);
                        break;
                    }
                }
                break;
            }
            cnt++;
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
        slot_deleteProgress();
        my_centralWidget->emitUpdateRoute(NULL);
        return;
    }

    my_centralWidget->updatePlayer(player);

    my_centralWidget->slot_playerSelected(player);
    my_centralWidget->loadPOI();
    nBoat=my_centralWidget->getBoats()->size();
    toBeCentered=-1;
    if(nBoat>0)
    {
        progress->setLabelText("Updating boats (2)");
        progress->setValue(95);
        VLM_Sync_sync();
        timerprogress=new QTimer();
        timerprogress->setSingleShot(true);
        timerprogress->setInterval(5000);
        connect(timerprogress,SIGNAL(timeout()),this, SLOT(slot_deleteProgress()));
        timerprogress->start();
        return;
    }
    else
    {
        isStartingUp=false;
        updateTitle();
    }
    my_centralWidget->emitUpdateRoute(NULL);
    slot_deleteProgress();
}

void MainWindow::slotSelectPOI(DialogPilototoInstruction * instruction)
{
    selPOI_instruction=instruction;
    menuBar->boatList->setEnabled(false);
    updatePilototo_Btn((boatVLM*)selectedBoat);
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
        DialogPilototoInstruction * tmp=selPOI_instruction;
        selPOI_instruction=NULL;
        updatePilototo_Btn((boatVLM*)selectedBoat);
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

void MainWindow::slotAccountListUpdated(void)
{

    selectedBoat=NULL;

    if(!my_centralWidget->getBoats())
    {
        qWarning() << "CRITICAL: slotAccountListUpdated - empty boatList";
        return ;
    }

    //qWarning() << "Boat list updated: " << my_centralWidget->getBoats()->count() << " boats";

    menuBar->updateBoatList(*my_centralWidget->getBoats());

    slotVLM_Sync();
}

void MainWindow::slotChgWP(double lat,double lon, double wph)
{
    if(this->selectedBoat->getType()==BOAT_VLM)
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

void MainWindow::slotpastePOI()
{
    double lon, lat,wph;
    int tstamp;
    QString name;

    if(!Util::getWPClipboard(&name,&lat,&lon,&wph,&tstamp))
        return;

    emit addPOI(name,POI_TYPE_POI,lat,lon,wph,tstamp,tstamp!=-1, selectedBoat);
}

void MainWindow::slotBoatLockStatusChanged(boat* boat,bool status)
{
    if(boat==selectedBoat)
    {
        if(selPOI_instruction)
        {
            emit setChangeStatus(true);
            menuBar->acPilototo->setEnabled(true);
            menuBar->acVLMSync->setEnabled(false);
            myBoard->VLMBoard()->btn_Pilototo->setEnabled(true);
        }
        else if(isSelectingWP)
        {
            emit setChangeStatus(true);
            menuBar->acPilototo->setEnabled(false);
            menuBar->acVLMSync->setEnabled(false);
            myBoard->VLMBoard()->btn_Pilototo->setEnabled(false);
        }
        else
        {
            emit setChangeStatus(status);
            menuBar->acPilototo->setEnabled(!status);
            menuBar->acVLMSync->setEnabled(true);
            myBoard->VLMBoard()->btn_Pilototo->setEnabled(!status);
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
    DialogViewPolar *dvp=new DialogViewPolar(this);
    dvp->setBoat(selectedBoat);
    dvp->exec();
    if(dvp)
        delete dvp;
}

void MainWindow::slotPilototo(void)
{
    if(!selectedBoat || selectedBoat->getType()!=BOAT_VLM) return;

    if(selPOI_instruction)
    {
        slot_POIselected(NULL);
    }
    else
    {
        if(!getBoatLockStatus())
        {
            emit editInstructions();
            ((boatVLM*)selectedBoat)->slot_getData(true);
        }
    }
}
void MainWindow::setPilototoFromRoute(ROUTE *route)
{
    if(!route->getBoat() || route->getBoat()->getType()!=BOAT_VLM)
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
    if(!route->getBoat() || route->getBoat()->getType()!=BOAT_VLM)
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
void MainWindow::clearPilototo()
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

    for(int i=0;i<my_centralWidget->getBoats()->count();i++)
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

void MainWindow::getBoatWP(double * lat,double * lon)
{
   if(!lat || !lon)
       return;
   if(!selectedBoat || selectedBoat->getType()!=BOAT_VLM)
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
    loadVLM_grib->showDialog();
}

/*************************************/
#ifdef __QTVLM_WITH_TEST

#include "miniunz.h"

void MainWindow::slotVLM_Test(void)
{    
    qWarning() << "unzip reports: " << miniunzip(UZ_EXTRACT_WOPATH | UZ_OVERWRITE,"test2.zip","./",NULL,NULL);


}
#else
void MainWindow::slotVLM_Test(void)
{
}
#endif
void MainWindow::slotGribInterpolation(void)
{
#ifdef __QTVLM_WITH_TEST
    if(my_centralWidget->getGrib())
    {
        gribValidation_dialog->setMode(this->my_centralWidget->getGrib()->getInterpolationMode());
        gribValidation_dialog->show();
    }
#endif
}

void MainWindow::slotEstime(int valeur)
{
    switch(Settings::getSetting("estimeType","0").toInt())
    {
        case 0:
            Settings::setSetting("estimeTime", valeur);
            param->estimeVal_time->setValue(valeur);
            break;
        case 1:
            Settings::setSetting("estimeVac", valeur);
            param->estimeVal_vac->setValue(valeur);
            break;
        case 2:
            Settings::setSetting("estimeLen", valeur);
            param->estimeVal_dist->setValue(valeur);
    }
    emit paramVLMChanged();
}
void MainWindow::slot_ParamVLMchanged()
{
    switch(Settings::getSetting("estimeType","0").toInt())
    {
        case 0:
            tool_ESTIMEUNIT->setText(tr(" mins "));
            menuBar->estime->setValue(Settings::getSetting("estimeTime","60").toInt());
            break;
        case 1:
            tool_ESTIMEUNIT->setText(tr(" vacs "));
            menuBar->estime->setValue(Settings::getSetting("estimeVac","12").toInt());
            break;
        case 2:
            tool_ESTIMEUNIT->setText(tr(" NM "));
            menuBar->estime->setValue(Settings::getSetting("estimeLen","50").toInt());
            break;
        default:
            tool_ESTIMEUNIT->setText(tr(" mins "));
            menuBar->estime->setValue(0);
    }

}
