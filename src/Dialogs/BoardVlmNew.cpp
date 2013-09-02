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

#include <QBitmap>
#include <QMessageBox>
#include <QStyleFactory>
#include <QGraphicsDropShadowEffect>

#include "BoardVlmNew.h"
#include "ui_BoardVlmNew.h"
#include "POI.h"
#include "DialogWp.h"
#include "Polar.h"
#include "settings.h"
#include "vlmLine.h"
#include "boatVLM.h"
#include "MapDataDrawer.h"

BoardVlmNew::BoardVlmNew(MainWindow *main)
    : QDialog(main)

{
    this->setParent(main);
    this->setupUi(this);
    tryMoving=false;
    this->setFontDialog(this);
    this->main=main;
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    this->setModal(false);
    this->move(1,qRound((main->height()-this->height())/2.0));
    this->show();
    QPixmap del("img/delete.png");
    btn_clearPilototo->setIcon(QIcon(del));
    btn_clearPilototo->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    btn_clearWP->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    btn_clearWP->setIcon(QIcon(del));
    btn_clearWP->setMask(del.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
    btn_clearPilototo->setMask(del.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
    QPixmap angleFlip("img/board_angleFlip.png");
    btn_angleFlip->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    btn_angleFlip->setIcon(QIcon(angleFlip));
    btn_angleFlip->setMask(angleFlip.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
    defaultStyleSheet=this->lab_TWA->styleSheet();
    connect(this->btn_sync,SIGNAL(clicked()),this,SLOT(slot_vlmSync()));
    connect(main,SIGNAL(boatHasUpdated(boat*)),this,SLOT(slot_updateData()));
    connect(main,SIGNAL(boatChanged(boat*)),this,SLOT(slot_updateData()));
    connect(main,SIGNAL(WPChanged(double,double)),this,SLOT(slot_wpChanged()));
    connect(main,SIGNAL(outDatedVlmData()),this,SLOT(slot_outDatedVlmData()));
    connect(this->rd_HDG,SIGNAL(clicked()),this,SLOT(slot_sendOrder()));
    connect(this->rd_TWA,SIGNAL(clicked()),this,SLOT(slot_sendOrder()));
    connect(this->rd_ORTHO,SIGNAL(clicked()),this,SLOT(slot_sendOrder()));
    connect(this->rd_VMG,SIGNAL(clicked()),this,SLOT(slot_sendOrder()));
    connect(this->rd_VBVMG,SIGNAL(clicked()),this,SLOT(slot_sendOrder()));
    connect(this->dial,SIGNAL(valueChanged(int)),this,SLOT(slot_dialChanged(int)));
    connect(this->btn_angleFlip,SIGNAL(clicked()),this,SLOT(slot_flipAngle()));
    connect(this->spin_TWA,SIGNAL(valueChanged(double)),this,SLOT(slot_TWAChanged()));
    connect(this->spin_HDG,SIGNAL(valueChanged(double)),this,SLOT(slot_HDGChanged()));
    connect(this->btn_clearPilototo,SIGNAL(clicked()),this,SLOT(slot_clearPilototo()));
    connect(btn_pilototo,SIGNAL(clicked()),main,SLOT(slotPilototo()));
    connect(btn_clearWP,SIGNAL(clicked()),this,SLOT(slot_clearWP()));
    connect(main,SIGNAL(selectPOI(bool)),this,SLOT(slot_selectPOI(bool)));
    connect(main,SIGNAL(wpChanged()),this,SLOT(slot_updateBtnWP()));
    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_reloadSkin()));
    connect(main,SIGNAL(updateLockIcon(QIcon)),this,SLOT(slot_lock()));
    wpDialog = new DialogWp();
    connect(wpDialog,SIGNAL(selectPOI()),this,SLOT(slot_selectWP_POI()));
    connect(wpDialog,SIGNAL(selectPOI()),main,SLOT(slotSelectWP_POI()));
    connect(main,SIGNAL(editWP_POI(POI*)),this,SLOT(slot_selectPOI(POI*)));
    connect(main,SIGNAL(editWP_POI(POI*)),wpDialog,SLOT(show_WPdialog(POI *)));
    connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(slot_tabChanged(int)));
    connect(btn_wp,SIGNAL(clicked()),this,SLOT(slot_editWP()));
    timer=new QTimer(this);
    timer->setInterval(600);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(slot_timerElapsed()));
    timerDial=new QTimer(this);
    timerDial->setInterval(500);
    timerDial->setSingleShot(true);
    connect(timerDial,SIGNAL(timeout()),this,SLOT(slot_timerDialElapsed()));
    currentRB=NULL;
    myBoat=NULL;
    blocking=false;
    set_style(btn_sync);
    set_style(btn_pilototo);
    set_style(btn_wp);
    this->spin_HDG->installEventFilter(this);
    this->spin_TWA->installEventFilter(this);
    this->spin_PolarTWS->installEventFilter(this);
    vibration=new QTimer(this);
    vibration->setSingleShot(false);
    nbVib=-1;
    vibration->setInterval(100);
    connect(vibration,SIGNAL(timeout()),this,SLOT(slot_vibrate()));
    slot_reloadSkin();
    polarImg=QPixmap(this->lab_polar->size());
    polarImg.fill(Qt::transparent);
    this->lab_polar->setPixmap(polarImg);
    connect(this->spin_PolarTWS,SIGNAL(valueChanged(double)),this,SLOT(slot_drawPolar()));
    lab_polar->installEventFilter(this);
    lab_polarData->clear();
    QString tabStyle;
    tabStyle+="QTabWidget::pane { border: 0; } ";
    tabStyle+="QTabWidget { background: transparent; } ";
    tabStyle+="QTabBar::tab { border: 0px solid #000000;border-bottom-right-radius: 8px;border-top-right-radius: 8px;padding: 2px;margin-left: 4px;margin-right: 4px;margin-bottom:3px;}";
    tabStyle+="QTabBar::tab:selected { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #CC9900, stop: 0.8 #FFE085);margin-left:0;}";
    tabStyle+="QTabBar::tab:!selected { background: #C2C7CB;margin-right: 4px;}";
    tabWidget->setStyleSheet(tabStyle);
    this->lab_backTab1->installEventFilter(this);
    this->lab_backTab2->installEventFilter(this);
    this->lab_backTab3->installEventFilter(this);
    this->lab_back->installEventFilter(this);
    QGraphicsDropShadowEffect *shadow=new QGraphicsDropShadowEffect(this);
    this->setGraphicsEffect(shadow);
    vibStates.append(10);
    vibStates.append(-10);
    vibStates.append(9);
    vibStates.append(-9);
    vibStates.append(8);
    vibStates.append(-8);
    vibStates.append(7);
    vibStates.append(-7);
    vibStates.append(6);
    vibStates.append(-6);
    vibStates.append(5);
    vibStates.append(-5);
    vibStates.append(4);
    vibStates.append(-4);
    vibStates.append(3);
    vibStates.append(-3);
    vibStates.append(2);
    vibStates.append(-2);
    vibStates.append(1);
    vibStates.append(-1);
    vibStates.append(0);
    flipBS=false;
}
BoardVlmNew::~BoardVlmNew()
{
    delete wpDialog;
}
void BoardVlmNew::setFontDialog(QObject * o)
{

    QFont myFont(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    if(o->isWidgetType())
    {
        QWidget * widget=qobject_cast<QWidget*> (o);
        if(widget==lab_VBVMG || widget==lab_VMG || widget==lab_HDG || widget==lab_TWA || widget==lab_ORTHO)
            myFont.setPointSizeF(8.0);
        else
            myFont.setPointSizeF(8.5);
        myFont.setStyle(widget->font().style());
        myFont.setBold(widget->font().bold());
        myFont.setItalic(widget->font().italic());
        widget->setFont(myFont);
        widget->setLocale(QLocale::system());
    }
    foreach(QObject * object,o->children())
    {
        this->setFontDialog(object); /*recursion*/
    }
}

void BoardVlmNew::slot_tabChanged(int tabNb)
{
    switch(tabNb)
    {
    case 0:
        lab_back->setPixmap(imgBack0);
        break;
    case 1:
        lab_back->setPixmap(imgBack1);
        break;
    case 2:
        lab_back->setPixmap(imgBack2);
        break;
    }
}

void BoardVlmNew::slot_reloadSkin()
{
    QPixmap skin;
    QString skinName=Settings::getSetting("defaultSkin",QFileInfo("img/skin_compas.png").absoluteFilePath()).toString();
    if(!QFile(skinName).exists())
        skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    if(myBoat && myBoat->get_useSkin())
    {
        QString specificSkin=myBoat->get_boardSkin();
        if(QFile(specificSkin).exists())
            skinName=specificSkin;
    }
    skin.load(skinName);
    imgBack0=QPixmap (270,510);
    imgBack0.fill(Qt::transparent);
    QPainter pnt(&imgBack0);
    pnt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pnt.setRenderHint(QPainter::Antialiasing,true);
    pnt.drawPixmap(0,0,skin,0,510,270,510);
    this->lab_backTab1->setPixmap(imgBack0);
    pnt.end();
    imgBack1=QPixmap (270,510);
    imgBack1.fill(Qt::transparent);
    pnt.begin(&imgBack1);
    pnt.drawPixmap(0,0,skin,300,510,270,510);
    this->lab_backTab2->setPixmap(imgBack1);
    pnt.end();
    imgBack2=QPixmap (270,510);
    imgBack2.fill(Qt::transparent);
    pnt.begin(&imgBack2);
    pnt.drawPixmap(0,0,skin,600,510,270,510);
    this->lab_backTab3->setPixmap(imgBack2);
    pnt.end();
    slot_tabChanged(tabWidget->currentIndex());
    this->windAngle->loadSkin();
    this->slot_updateData();
}
void BoardVlmNew::slot_flipAngle()
{
    this->spin_TWA->setValue(-spin_TWA->value());
}
void BoardVlmNew::slot_TWAChanged()
{
    if(!myBoat) return;
    if(blocking) return;
    blocking=true;
    currentRB=this->lab_TWA;
    if(myBoat->getPolarData())
    {
        double twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
        else if(twa==qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: green;} QDoubleSpinBox QWidget {color:black;}");
        else
            spin_TWA->setStyleSheet(spin_HDG->styleSheet());
    }
    double heading = myBoat->getWindDir() + spin_TWA->value();
    if(heading<0) heading+=360;
    else if(heading>360) heading-=360;
    this->spin_HDG->setValue(heading);
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading);
    if(!timer->isActive())
        timer->start();
    /* update estime */
    double newSpeed=myBoat->getSpeed();
    if(myBoat->getPolarData())
        newSpeed=myBoat->getPolarData()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
    myBoat->drawEstime(spin_HDG->value(),newSpeed);
    blocking=false;
}
void BoardVlmNew::slot_HDGChanged()
{
    if(!myBoat) return;
    if(blocking) return;
    blocking=true;
    currentRB=this->lab_HDG;
    spin_HDG->blockSignals(true);
    spin_TWA->blockSignals(true);
    double heading=spin_HDG->value();
    double angle=Util::A180(heading-myBoat->getWindDir());
    this->spin_TWA->setValue(angle);
    if(myBoat->getPolarData())
    {
        double twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
        else if(twa==qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: green;} QDoubleSpinBox QWidget {color:black;}");
        else
            spin_TWA->setStyleSheet(spin_HDG->styleSheet());
    }
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading);
    spin_HDG->blockSignals(false);
    spin_TWA->blockSignals(false);
    if(!timer->isActive())
        timer->start();
    /* update estime */
    double newSpeed=myBoat->getSpeed();
    if(myBoat->getPolarData())
        newSpeed=myBoat->getPolarData()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
    myBoat->drawEstime(spin_HDG->value(),newSpeed);
    blocking=false;
}
void BoardVlmNew::slot_lock()
{
    bool lock=true;
    if(myBoat)
        lock=myBoat->getLockStatus();
    this->spin_HDG->setDisabled(lock);
    this->spin_TWA->setDisabled(lock);
    this->rd_HDG->setDisabled(lock);
    this->rd_ORTHO->setDisabled(lock);
    this->rd_TWA->setDisabled(lock);
    this->rd_VBVMG->setDisabled(lock);
    this->rd_VMG->setDisabled(lock);
    this->dial->setDisabled(lock);
    this->btn_angleFlip->setDisabled(lock);
    wpDialog->setLocked(!lock);
    this->btn_clearPilototo->setDisabled(lock);
    this->btn_clearWP->setDisabled(lock);
}

void BoardVlmNew::slot_wpChanged()
{
    timerStop();
    if(!myBoat) return;
    this->slot_updateBtnWP();
}

void BoardVlmNew::slot_sendOrder()
{
    timerStop();
    if(!myBoat || myBoat->getLockStatus()) return;
    this->blockSignals(true);
    dial->blockSignals(true);
    set_style(this->btn_sync,QColor(255,0,0));
    if(rd_HDG->isChecked())
    {
        myBoat->set_pilotHeading(this->spin_HDG->value());
        dial->setValue(1);
    }
    else if(rd_TWA->isChecked())
    {
        myBoat->set_pilotAngle(this->spin_TWA->value());
        dial->setValue(2);
    }
    else if(rd_ORTHO->isChecked())
    {
        myBoat->set_pilotOrtho();
        dial->setValue(5);
    }
    else if(rd_VMG->isChecked())
    {
        myBoat->set_pilotVmg();
        dial->setValue(4);
    }
    else
    {
        myBoat->set_pilotVbvmg();
        dial->setValue(3);
    }
    dial->blockSignals(false);
    this->blockSignals(false);
}
void BoardVlmNew::slot_dialChanged(int)
{
    if(blocking) return;
    timerStop();
    timerDial->start();
}
void BoardVlmNew::slot_timerDialElapsed()
{
    switch(dial->value())
    {
    case 1:
        rd_HDG->setChecked(true);
        break;
    case 2:
        rd_TWA->setChecked(true);
        break;
    case 3:
        rd_VBVMG->setChecked(true);
        break;
    case 4:
        rd_VMG->setChecked(true);
        break;
    case 5:
        rd_ORTHO->setChecked(true);
        break;
    }
    slot_sendOrder();
}
void BoardVlmNew::slot_vlmSync()
{
    timerStop();
    btn_sync->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    set_style(this->btn_sync,QColor(255,0,0));
    main->slotVLM_Sync();
}
void BoardVlmNew::slot_updateData()
{
    timerStop();
    vibration->stop();
    windAngle->setRotation(0.0);
    nbVib=0;
    if(!main->getSelectedBoat() || main->getSelectedBoat()->get_boatType()!=BOAT_VLM)
    {
        myBoat=NULL;
        return;
    }
    myBoat=(boatVLM*)main->getSelectedBoat();
    if(!myBoat) return;
    slot_lock();
    this->blockSignals(true);
    this->blocking=true;
    updateLcds();
    QPointF position=myBoat->getPosition();
    det_POS->setText(Util::formatLongitude(position.x())+"-"+Util::formatLatitude(position.y()));
    if(qRound(myBoat->getDnm())<100)
        det_DNM->setText(QString().sprintf("%.2f",myBoat->getDnm())+tr("nm"));
    else
        det_DNM->setText(QString().sprintf("%d",qRound(myBoat->getDnm()))+tr("nm"));
    det_ORT->setText(QString().sprintf("%.2f",myBoat->getOrtho())+tr("deg"));
    det_VMG->setText(QString().sprintf("%.2f",myBoat->getVmg())+tr("kts"));
    det_ANGLE->setText(QString().sprintf("%.2f",myBoat->getWPangle())+tr("deg"));
    det_TWS->setText(QString().sprintf("%.2f",myBoat->getWindSpeed())+tr("kts"));
    det_TWD->setText(QString().sprintf("%.2f",myBoat->getWindDir())+tr("deg"));
    if(myBoat->getPolarData())
    {
        det_UPWind->setText(QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()))+tr("deg"));
        det_DwWind->setText(QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))+tr("deg"));
    }
    if(qRound(myBoat->getLoch())<100)
        det_LOCH->setText(QString().sprintf("%.2f",myBoat->getLoch())+tr("nm"));
    else
        det_LOCH->setText(QString().sprintf("%d",qRound(myBoat->getLoch()))+tr("nm"));
    this->det_BS->setText(QString().sprintf("%.2f",myBoat->getSpeed())+tr("kts"));
    this->det_HDG->setText(QString().sprintf("%.2f",myBoat->getHeading())+tr("deg"));
    this->det_AVG->setText(QString().sprintf("%.2f",myBoat->getAvg())+tr("kts"));
    this->lab_RANK->setText(myBoat->getName()+" "+myBoat->getScore()+" ("+QString().sprintf("%d",myBoat->getRank())+")");
    this->det_boatBox->setTitle(lab_RANK->text());
    this->det_raceBox->setTitle(myBoat->getRaceName());
    this->det_GATE_ORT->setText(QString().sprintf("%.2f",myBoat->getClosest().capArrival)+tr("deg"));
    if(qRound(myBoat->getClosest().distArrival)<100)
        this->det_GATE_DIST->setText(QString().sprintf("%.2f",myBoat->getClosest().distArrival)+tr("nm"));
    else
        this->det_GATE_DIST->setText(QString().sprintf("%d",qRound(myBoat->getClosest().distArrival))+tr("nm"));
    if(!myBoat->getGates().isEmpty())
        this->det_GATE->setText(myBoat->getGates().at(myBoat->getNWP()-1)->getDesc());
    this->spin_HDG->setValue(myBoat->getHeading());
    this->spin_TWA->setValue(computeAngle());
    if(myBoat->getPolarData())
    {
        double twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
        else if(twa==qRound(myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("QDoubleSpinBox {color: green;} QDoubleSpinBox QWidget {color:black;}");
        else
            spin_TWA->setStyleSheet(spin_HDG->styleSheet());
        QString tipTWA=tr("Meilleurs angles au pres/portant:")+" "+QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()))+tr("deg")+"/"
                +QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))+tr("deg");
        spin_TWA->setToolTip("<p style='white-space:pre'>"+tipTWA+"</p>");
    }
    this->lab_TWA->setStyleSheet(defaultStyleSheet.toUtf8());
    this->lab_HDG->setStyleSheet(defaultStyleSheet.toUtf8());
    this->lab_ORTHO->setStyleSheet(defaultStyleSheet.toUtf8());
    this->lab_VMG->setStyleSheet(defaultStyleSheet.toUtf8());
    this->lab_VBVMG->setStyleSheet(defaultStyleSheet.toUtf8());
    switch(myBoat->getPilotType())
    {
    case 1:
        this->rd_HDG->setChecked(true);
        this->dial->setValue(1);
        this->lab_HDG->setStyleSheet(QString::fromUtf8("background-color: rgb(28, 205, 28);"));
        break;
    case 2:
        this->rd_TWA->setChecked(true);
        this->dial->setValue(2);
        this->lab_TWA->setStyleSheet(QString::fromUtf8("background-color: rgb(28, 205, 28);"));
        break;
    case 3:
        this->rd_ORTHO->setChecked(true);
        this->dial->setValue(5);
        this->lab_ORTHO->setStyleSheet(QString::fromUtf8("background-color: rgb(28, 205, 28);"));
        break;
    case 4:
        this->rd_VMG->setChecked(true);
        this->dial->setValue(4);
        this->lab_VMG->setStyleSheet(QString::fromUtf8("background-color: rgb(28, 205, 28);"));
        break;
    case 5:
        this->rd_VBVMG->setChecked(true);
        this->dial->setValue(3);
        this->lab_VBVMG->setStyleSheet(QString::fromUtf8("background-color: rgb(28, 205, 28);"));
        break;
    }
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,-1);
    update_btnPilototo();
    slot_updateBtnWP();
    this->blockSignals(false);
    spin_PolarTWS->setValue(myBoat->getWindSpeed());
    this->blocking=false;
    set_style(this->btn_sync,QColor(255,255,127));
    if(!vibration->isActive())
        vibration->start(80);
}
void BoardVlmNew::slot_vibrate()
{
    ++nbVib;
    if(nbVib>=vibStates.size())
    {
        vibration->stop();
        windAngle->setRotation(0);
        nbVib=-1;
        return;
    }
    windAngle->setRotation(vibStates.at(nbVib));
}
void BoardVlmNew::slot_drawPolar()
{
    lab_polarData->clear();
    polarImg.fill(Qt::transparent);
    Polar * polar=myBoat->getPolarData();
    if(!polar)
    {
        lab_polarName->setText(tr("pas de polaire chargee"));
        lab_polar->setPixmap(polarImg);
        return;
    }
    lab_polarName->setText(polar->getName());
    polarPnt.begin(&polarImg);
    QFont myFont(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    myFont.setPointSizeF(8.0);
    polarPnt.setFont(myFont);
    polarPnt.setRenderHint(QPainter::Antialiasing);
    double maxSpeed=-1;
//    if(this->allSpeed->isChecked())
//        maxSpeed=polar->getMaxSpeed();
    QPen pen;
    pen.setBrush(Qt::red);
    double maxSize=(polarImg.height()/2)*0.85;
    double ws=spin_PolarTWS->value();
    double wsMin=ws;
    double wsMax=ws;
//    if(this->allSpeed->isChecked())
//    {
//        wsMin=5;
//        wsMax=50;
//    }
    QString s;
//    if(this->allSpeed->isChecked())
//    {
//        this->BVMG_down->clear();
//        this->BVMG_up->clear();
//    }
    QPointF center(0,polarImg.height()/2.0);
    int step=5;
    int nn=0;
    QPolygonF polarGreen;
    for(ws=wsMin;ws<=wsMax;ws+=step)
    {
        ++nn;
        polarValues.clear();
        polarLine.clear();
        polarGreen.clear();
        double bvmgUp=polar->getBvmgUp(ws,false);
        double bvmgDown=polar->getBvmgDown(ws,false);
        pen.setColor(Qt::red);
        pen.setWidth(2);
        polarPnt.setPen(pen);
        s="("+QString().sprintf("%.1f",bvmgUp)+tr("deg")+"/"+QString().sprintf("%.1f",bvmgDown)+tr("deg")+")";
        this->lab_polarUpDw->setText(s);
        for(int angle=0;angle<=180;++angle)
        {
            double speed=polar->getSpeed(ws,angle,false);
            polarValues.append(speed);
            if(speed>maxSpeed /*&& !this->allSpeed->isChecked()*/) maxSpeed=speed;
        }
        maxSpeed=ceil(maxSpeed);
        for (int angle=0;angle<=180;++angle)
        {
            QLineF line(center,QPointF(polarImg.width(),polarImg.height()/2.0));
            line.setAngle(90-angle);
            line.setLength(polarValues.at(angle)*maxSize/maxSpeed);
            polarLine.append(line.p2());
            if(angle>bvmgUp && angle < bvmgDown)
                polarGreen.append(line.p2());
        }
        polarPnt.drawPolyline(polarLine.toPolygon());
        pen.setColor(Qt::green);
        polarPnt.setPen(pen);
        polarPnt.drawPolyline(polarGreen.toPolygon());
//        if(this->allSpeed->isChecked())
//        {
//            pen.setColor(Qt::blue);
//            pen.setWidthF(0.5);
//            pnt.setPen(pen);
//            s=s.sprintf("tws %.0f",ws);
//            pnt.drawText(polarLine.at(qMin(180,nn*15+40)),s);
//        }
    }
    pen.setColor(Qt::black);
    pen.setWidthF(0.5);
    polarPnt.setPen(pen);
    QPolygonF lines;
    bool flip=true;
    for (int bs=0;bs<=maxSpeed+1;bs+=2)
    {
        lines.clear();
        for (int angle=0;angle<=180;++angle)
        {
            QLineF line(center,QPointF(polarImg.width(),polarImg.height()/2.0));
            line.setAngle(90-angle);
            line.setLength(bs*maxSize/maxSpeed);
            lines.append(line.p2());
        }
        polarPnt.drawPolyline(lines.toPolygon());
        if(flip)
        {
            s=s.sprintf("%d",bs);
            polarPnt.drawText(lines.first(),s);
        }
        flip=!flip;
    }
    for (int angle=0;angle<=180;angle+=15)
    {
        QLineF line(center,QPointF(polarImg.width(),polarImg.height()/2.0));
        line.setAngle(90-angle);
        line.setLength((maxSpeed+1)*maxSize/maxSpeed);
        polarPnt.drawLine(line);
        if(angle==0) continue;
        line.setLength(line.length()*1.03);
        s=s.sprintf("%d",angle);
        polarPnt.drawText(line.p2(),s);
    }
    pen.setColor(MapDataDrawer::getWindColorStatic(myBoat->getWindSpeed(),Settings::getSetting("colorMapSmooth", true).toBool()));
    pen.setWidthF(2.0);
    polarPnt.setPen(pen);
    QLineF line(center,QPointF(polarImg.width(),polarImg.height()/2.0));
    line.setAngle(90-(qAbs(spin_TWA->value())));
    line.setLength((maxSpeed+1)*maxSize/maxSpeed);
    polarPnt.drawLine(line);
    polarPnt.end();
    this->lab_polar->setPixmap(polarImg);
}
void BoardVlmNew::slot_outDatedVlmData()
{
    set_style(this->btn_sync,QColor(255,191,21));
}
void BoardVlmNew::updateLcds()
{
    QString s;
    s.sprintf("%.2f",(double)qRound(myBoat->getSpeed()*100.0)/100.0);
    lcd_BS->setDigitCount(s.count());
    this->lcd_BS->display(s);
    s.sprintf("%.2f",(double)qRound(myBoat->getWindSpeed()*100.0)/100.0);
    lcd_TWS->setDigitCount(s.count());
    this->lcd_TWS->display(s);
    s.sprintf("%.2f",(double)qRound(myBoat->getWindDir()*100.0)/100.0);
    lcd_TWD->setDigitCount(s.count());
    this->lcd_TWD->display(s);
    QColor color=MapDataDrawer::getWindColorStatic(myBoat->getWindSpeed(),Settings::getSetting("colorMapSmooth", true).toBool());
    this->lcd_TWS->setStyleSheet((QString().sprintf("background-color: rgb(%d, %d, %d);",color.red(),color.green(),color.blue())));
    color=Qt::white;
    this->lcd_TWD->setStyleSheet((QString().sprintf("background-color: rgba(%d, %d, %d,%d);",color.red(),color.green(),color.blue(),180)));
    this->lcd_BS->setStyleSheet((QString().sprintf("background-color: rgba(%d, %d, %d, %d);",color.red(),color.green(),color.blue(),180)));
}
void BoardVlmNew::slot_timerElapsed()
{
    if(currentRB==lab_TWA)
        lab_TWS->show();
    else
        lab_TWA->show();
    currentRB->setVisible(!currentRB->isVisible());
    flipBS=!flipBS;
    double speed=myBoat->getSpeed();
    QColor color=Qt::white;
    if(myBoat && flipBS && myBoat->getPolarData())
    {
        speed=myBoat->getPolarData()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
        color=Qt::green;
        color=color.lighter();
    }
    QString s;
    s.sprintf("%.2f",(double)qRound(speed*100.0)/100.0);
    lcd_BS->setDigitCount(s.count());
    this->lcd_BS->display(s);
    this->lcd_BS->setStyleSheet((QString().sprintf("background-color: rgba(%d, %d, %d, %d);",color.red(),color.green(),color.blue(),180)));
}
void BoardVlmNew::timerStop()
{
    timer->stop();
    currentRB=NULL;
    lab_TWA->show();
    lab_HDG->show();
}
double BoardVlmNew::computeAngle(void) { /* we assume a boat exists => should be tested by caller */
    double angle_val;
    double val=myBoat->getHeading()-myBoat->getWindDir();
    if(myBoat->getPilotType()==2)
        angle_val = myBoat->getPilotString().toDouble();
    else
    {
        angle_val = myBoat->getTWA();
        calcAngleSign(val,angle_val)
    }
    return angle_val;
}
void BoardVlmNew::slot_clearPilototo(void)
{
    if(!myBoat) return;
    main->slot_clearPilototo();
}
void BoardVlmNew::update_btnPilototo()
{
    if(!myBoat) return;
    /* Pilototo btn */
    QStringList lst = myBoat->getPilototo();
    QString pilototo_txt=tr("Pilototo");
    QString pilototo_toolTip="";

    int nbPending=0;
    int nb=0;
    for(int i=0;i<lst.count();i++)
        if(lst.at(i)!="none" && !lst.at(i).isEmpty()) {
            QStringList instr_buf = lst.at(i).split(",");
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
    if(nbPending!=0)
        set_style(this->btn_pilototo,QColor(14,184,63));
    else
        set_style(this->btn_pilototo,QColor(255,255,127));

    btn_pilototo->setText(pilototo_txt);
    btn_pilototo->setToolTip(pilototo_toolTip);

//    /* clear piltoto btn */
//    if(nb!=0)
//        btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(14,184,63);"));
//    else
//        btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(230,230,230);"));
}
void BoardVlmNew::set_style(QPushButton * button, QColor color, QColor color2)
{
    QString borderString, bgString, hoverString;
    if(button==this->btn_sync)
        borderString="border: 1px solid #555;border-radius: 11px;padding: 4px;";
    else if(button==this->btn_clearPilototo)
        borderString="border: 1px solid #555;border-radius: 7px;padding: 1px;";
    else if(button==this->btn_clearWP)
        borderString="border: 1px solid #555;border-radius: 7px;padding: 1px;";
    else if(button==this->btn_angleFlip)
        borderString="border: 1px solid #555;border-radius: 11px;padding: 3px;";
    else if(button==this->btn_wp)
        borderString="border: 1px solid #555;border-radius: 7px;padding: 1px;";
    else if(button==this->btn_pilototo)
        borderString="border: 1px solid #555;border-radius: 7px;padding: 1px;";
    borderString="QPushButton {"+borderString+"border-style: outset;";
    if(color2==Qt::white)
    {
        color2.setHsv(color.hue(),255,220);
    }
    bgString="background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 "+color2.name()+", stop: 1 "+color.name()+");}";
    hoverString="QPushButton:hover {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 "+color.name()+", stop: 1 "+color2.name()+");border-style:inset;}";
    button->setStyleSheet(borderString+bgString+hoverString);
}
void BoardVlmNew::slot_editWP()
{
    if(main->get_selPOI_instruction())
        main->slot_POIselected(NULL);
    else
        wpDialog->show_WPdialog(myBoat);
}

void BoardVlmNew::slot_updateBtnWP(void)
{
    if(!myBoat) return;

    double WPLat = myBoat->getWPLat();
    double WPLon = myBoat->getWPLon();
    double WPHd = myBoat->getWPHd();
    QString tip;

    if(WPLat==0 && WPLon==0)
    {
        btn_wp->setText(tr("Pas de WP"));
        set_style(btn_wp,Qt::lightGray);
        tip=tr("Pas de WP actif");
    }
    else
    {
        QString wpPos;
        if(WPLat==0)
            wpPos+="0 N";
        else
            wpPos+=Util::pos2String(TYPE_LAT,WPLat);

        wpPos+=", ";

        if(WPLon==0)
            wpPos+="0 E";
        else
            wpPos+=Util::pos2String(TYPE_LON,WPLon);

        if(WPHd!=-1)
        {
            wpPos+=" @";
            wpPos+=QString().sprintf("%.1f",WPHd);
            wpPos+=tr("deg");
        }
        QString str;
        bool foundWP=false;
        bool correctWPH=false;
        QString wpName;
        for(int n=0;n<main->getPois()->count();++n)
        {
            if(main->getPois()->at(n)->getIsWp())
            {
                foundWP=true;
                wpName=main->getPois()->at(n)->getName();
                if(qRound(main->getPois()->at(n)->getWph()*100.0)==qRound(WPHd*100.0))
                    correctWPH=true;
                break;
            }
        }
        if(!foundWP)
        {
            set_style(btn_wp,QColor(255, 255, 127));/*yellow*/
            tip=tr("WP defini dans VLM (pas de POI correspondant)");
            tip+="<br>"+wpPos;
            str=tr("WP VLM");
        }
        else
        {
            str=tr("WP: ")+wpName;
            if(correctWPH)
            {
                set_style(btn_wp,Qt::green);/*green*/
                tip=tr("WP defini dans VLM (")+wpName+tr(" dans qtVlm)");
                tip+="<br>"+wpPos;
            }
            else
            {
                set_style(btn_wp,QColor(151,179,210));/*blue*/
                tip=tr("WP defini dans VLM (")+wpName+tr(" dans qtVlm)");
                tip+="<br>"+wpPos;
                tip+="<br>"+tr("Le cap a suivre n'est pas le meme");
            }
        }
        if(myBoat->getPilotType()<3)
        {
            set_style(btn_wp,QColor(255, 191, 21));/*orange*/
            tip=tr("WP defini dans VLM mais le mode de navigation n'est pas coherent");
            tip+="<br>"+wpPos;
        }
        tip="<p style='white-space:pre'>"+tip+"</p>";
        btn_wp->setToolTip(tip);
        btn_wp->setText(str);
//        if(WPLat==0)
//            str+="0 N";
//        else
//            str+=Util::pos2String(TYPE_LAT,WPLat);

//        str+=", ";

//        if(WPLon==0)
//            str+="0 E";
//        else
//            str+=Util::pos2String(TYPE_LON,WPLon);

//        if(WPHd!=-1)
//        {
//            str+=" @";
//            str+=QString().sprintf("%.1f",WPHd);
//            str+=tr("deg");
//        }

    }
}
void BoardVlmNew::slot_clearWP()
{
    if(!myBoat) return;
    QPointF pos(0,0);
    myBoat->setWP(pos,-1.0);
}
void BoardVlmNew::slot_selectPOI(bool doSelect)
{
    if(doSelect)
    {
        btn_pilototo->setText(tr("Annuler"));
        set_style(btn_pilototo,QColor(151,179,210));/*blue*/
        this->set_enabled(false);
        btn_pilototo->setEnabled(true);
    }
    else
    {
        update_btnPilototo();
        this->set_enabled(true);
    }
}
void BoardVlmNew::slot_selectPOI(POI *)
{
    this->set_enabled(true);
    slot_updateBtnWP();
}
void BoardVlmNew::slot_selectWP_POI()
{
    btn_wp->setText(tr("Annuler"));
    set_style(btn_wp,QColor(151,179,210));/*blue*/
    this->set_enabled(false);
    btn_wp->setEnabled(true);
}
void BoardVlmNew::set_enabled(const bool &b)
{
    this->btn_angleFlip->setEnabled(b);
    this->btn_clearPilototo->setEnabled(b);
    this->btn_clearWP->setEnabled(b);
    this->btn_sync->setEnabled(b);
    this->btn_wp->setEnabled(b);
    this->btn_pilototo->setEnabled(b);
    this->dial->setEnabled(b);
    this->rd_HDG->setEnabled(b);
    this->rd_ORTHO->setEnabled(b);
    this->rd_TWA->setEnabled(b);
    this->rd_VBVMG->setEnabled(b);
    this->rd_VMG->setEnabled(b);
    this->spin_HDG->setEnabled(b);
    this->spin_TWA->setEnabled(b);
}
bool BoardVlmNew::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==lab_polar)
    {
        if(event->type()==QEvent::MouseButtonRelease)
        {
            lab_polar->setPixmap(polarImg);
            lab_polarData->clear();
            return true;
        }
        if(event->type()!=QEvent::MouseMove || polarLine.isEmpty())
            return false;
        QMouseEvent * mouseEvent=static_cast<QMouseEvent*>(event);
        QPixmap i2=polarImg;
        QPen pe(Qt::blue);
        pe.setWidthF(3);
        QPainter pp(&i2);
        pp.setRenderHint(QPainter::Antialiasing);
        pp.setPen(pe);
        QPointF center(0,polarImg.height()/2.0);
        QLineF line(center,mouseEvent->pos());
        int angle=qRound(line.angle());
        if(angle>180)
            angle=angle-360;
        angle=90-angle;
        if(angle<0 || angle>180)
            return true;
        pp.drawLine(center,polarLine.at(angle));
        pe.setWidthF(1);
        pp.setPen(pe);
        QString s;
        double Y=90-angle;
        double a=this->spin_PolarTWS->value()*cos(degToRad(Y));
        double b=this->spin_PolarTWS->value()*sin(degToRad(Y));
        double bb=b+polarValues.at(angle);
        double aws=sqrt(a*a+bb*bb);
        double awa=90-radToDeg(atan(bb/a));
        double vmg=polarValues.at(angle)*cos(degToRad(angle));
        s=tr("TWA")+"<br>";
        s+=QString().sprintf("%d",angle)+tr("deg")+"<br>";
        s+=tr("BS")+"<br>";
        s+=QString().sprintf("%.2f",polarValues.at(angle))+tr("kts")+"<br>";
        s+=tr("AWA")+"<br>";
        s+=QString().sprintf("%.2f",awa)+tr("deg")+"<br>";
        s+=tr("AWS")+"<br>";
        s+=QString().sprintf("%.2f",aws)+tr("kts")+"<br>";
        s+=tr("VMG")+"<br>";
        s+=QString().sprintf("%.2f",vmg)+tr("kts");
        lab_polar->setPixmap(i2);
        lab_polarData->setText(s);
        return true;
    }
    if(obj==lab_backTab1 || obj==lab_backTab2 || obj==lab_backTab3 || obj==lab_back)
    {
        if(event->type()==QEvent::MouseButtonPress)
        {
            tryMoving=true;
            QMouseEvent *m=static_cast<QMouseEvent *>(event);
            startMove=mapToGlobal(m->pos());
            initialPos=this->pos();
        }
        else if(event->type()==QEvent::MouseButtonRelease)
        {
            tryMoving=false;
            nbVib=5;
            windAngle->setRotation(0);
            vibration->start(50);
        }
        else if(tryMoving && event->type()==QEvent::MouseMove)
        {
            QMouseEvent *m=static_cast<QMouseEvent *>(event);
            QPoint mousePos=mapToGlobal(m->pos());
            int x=qBound(qRound(-this->width()/2.0),initialPos.x()+mousePos.x()-startMove.x(),qRound(main->width()-this->width()/2.0));
            int y=qBound(qRound(-this->height()/2.0),initialPos.y()+mousePos.y()-startMove.y(),qRound(main->height()-this->height()/2.0));
            QLineF L=QLine(x,y,this->x(),this->y());
            int i=qRound(2000.0/(double)main->width())*L.length();
            if (this->x()>x)
                i=-i;
            this->move(x,y);
            windAngle->setRotation(windAngle->getRotation()+i);
        }
        return false;
    }
    if(obj!=spin_HDG && obj!=spin_TWA && obj!=spin_PolarTWS) return false;
    QDoubleSpinBox * spinBox=static_cast<QDoubleSpinBox *>(obj);
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key()==Qt::Key_Shift)
            spinBox->setSingleStep(0.1);
        else if(keyEvent->key()==Qt::Key_Control)
            spinBox->setSingleStep(10.0);
        else if(keyEvent->key()==Qt::Key_Alt)
            spinBox->setSingleStep(0.01);
    }
    else if (event->type()==QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key()==Qt::Key_Shift || keyEvent->key()==Qt::Key_Control || keyEvent->key()==Qt::Key_Alt)
            spinBox->setSingleStep(1.0);
    }
    if(event->type()==QEvent::Wheel)
    {
        /*by default wheeling with ctrl already multiply singleStep by 10
          so to get 10 you need to put 1...*/
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if(wheelEvent->modifiers()==Qt::ControlModifier)
            spinBox->setSingleStep(1);
    }
    return false;
}
bool BoardVlmNew::confirmChange()
{
    if(Settings::getSetting("askConfirmation","0").toInt()==0)
        return true;

    return QMessageBox::question(0,tr("Confirmation a chaque ordre vers VLM"),
                                 tr("Confirmez-vous cet ordre?"),QMessageBox::Yes|QMessageBox::No,
                             QMessageBox::Yes)==QMessageBox::Yes;
}
/*********************/
/* VLM20 windAngle   */
/*********************/

VlmCompass::VlmCompass(QWidget * parent):QWidget(parent)
{
    setFixedSize(200,200);
    loadSkin();
    WPdir = -1;
    newHeading=-1;
    rotation=0;
}
void VlmCompass::loadSkin()
{
    QPixmap skin;
    QString skinName=Settings::getSetting("defaultSkin",QFileInfo("img/skin_compas.png").absoluteFilePath()).toString();
    if(!QFile(skinName).exists())
        skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    skin.load(skinName);
    img_fond=QPixmap(200,200);
    img_fond.fill(Qt::transparent);
    QPainter pnt(&img_fond);
    pnt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pnt.setRenderHint(QPainter::Antialiasing,true);
    pnt.drawPixmap(0,0,skin,0,0,200,200);
    pnt.drawPixmap(0,0,skin,300,0,200,200);
    pnt.end();
    img_boat=QPixmap(200,200);
    img_boat.fill(Qt::transparent);
    pnt.begin(&img_boat);
    pnt.drawPixmap(0,0,skin,0,300,200,200);
    pnt.end();
    heading =windDir=windSpeed=0;
    img_arrow_wp=QPixmap(200,200);
    img_arrow_wp.fill(Qt::transparent);
    pnt.begin(&img_arrow_wp);
    pnt.drawPixmap(0,0,skin,300,300,200,200);
    pnt.end();
    img_arrow_gate=QPixmap(200,200);
    img_arrow_gate.fill(Qt::transparent);
    pnt.begin(&img_arrow_gate);
    pnt.drawPixmap(0,0,skin,600,300,200,200);
    pnt.end();
    img_arrow_wind=QPixmap(200,200);
    img_arrow_wind.fill(Qt::transparent);
    pnt.begin(&img_arrow_wind);
    pnt.drawPixmap(0,0,skin,600,0,200,200);
    pnt.end();
}

void VlmCompass::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setViewport(0,0,200,200);

    draw(&painter);
}
void VlmCompass::setRotation(const double r)
{
    rotation=r;
    update();
}

void VlmCompass::draw(QPainter * painter)
{
    painter->drawPixmap(0,0,img_fond);
    painter->save();
    painter->translate(100,100);
    painter->rotate(heading+rotation);
    painter->drawPixmap(-100,-100,img_boat);
    painter->restore();
    if(newHeading!=heading && newHeading!=-1)
    {
        QPixmap tempBoat=img_boat;
        QPainter pnt(&tempBoat);
        pnt.setRenderHint(QPainter::Antialiasing,true);
        pnt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pnt.fillRect(0,0,200,200,QBrush(QColor(0,0,0,100)));
        pnt.end();
        painter->save();
        painter->translate(100,100);
        painter->rotate(newHeading+rotation);
        painter->drawPixmap(-100,-100,tempBoat);
        painter->restore();
    }
    if(WPdir!=-1)
    {
        painter->save();
        painter->translate(100,100);
        painter->rotate(WPdir);
        painter->drawPixmap(-100,-100,img_arrow_wp);
        painter->restore();
    }
    painter->save();
    painter->translate(100,100);
    painter->rotate(gateDir);
    painter->drawPixmap(-100,-100,img_arrow_gate);
    painter->restore();
    QPixmap tempWind=img_arrow_wind;
    QPainter pnt(&tempWind);
    pnt.setRenderHint(QPainter::Antialiasing,true);
    pnt.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    pnt.fillRect(0,0,200,200,QBrush(windSpeed_toColor()));
    pnt.end();
    painter->save();
    painter->translate(100,100);
    painter->rotate(windDir);
    painter->drawPixmap(-100,-100,tempWind);
    painter->restore();
}

QColor VlmCompass::windSpeed_toColor()
{
    return MapDataDrawer::getWindColorStatic(windSpeed,Settings::getSetting("colorMapSmooth", true).toBool());
}

void VlmCompass::setValues(const double &heading, const double &windDir, const double &windSpeed, const double &WPdir, const double &gateDir, const double &newHeading)
{
    //qWarning() << "windAngle set: heading=" << heading << " windDir=" << windDir << " windSpeed=" << windSpeed << " WPdir=" << WPdir << " " << newHeading;
    this->heading=heading;
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->WPdir=WPdir;
    this->newHeading=newHeading;
    this->gateDir=gateDir;
    update();
}
