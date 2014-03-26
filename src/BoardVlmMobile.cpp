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
#include <QFile>
#include <QFileInfo>

#include "BoardVlmMobile.h"
#include "ui_BoardVlmMobile.h"
#include "POI.h"
#include "PolarInterface.h"
#include "vlmLine.h"
#include "BoatInterface.h"
#include "MapDataDrawer.h"
#include "settings_def.h"
#include "AngleUtil.h"
#include <QTranslator>
#include <QDebug>
#include <QStyleFactory>
#include <QLCDNumber>
#include <QToolBar>

BoardVlmMobile::BoardVlmMobile (QWidget* parent): BoardInterface (parent)
{
    sc=1.0;
}

void BoardVlmMobile::initBoard(MainWindowInterface *main)

{
    this->setAttribute(Qt::WA_TranslucentBackground);
    qWarning()<<"start of init board for"<<this->getName();
    mutex=new QMutex();
    sc=1.0;
    fontSizeRatio=qApp->font().pointSize()/9.0;
    this->main=main;
    this->setParent(main);

    qWarning()<<"before setup ui";
    this->setupUi(this);
    qWarning()<<"after setup ui";
    //backLabel->lower();
    QString myStyleSheet=""
            "QTabWidget::pane { border: 0; } "
            "QTabWidget::tab-bar {alignment: left;}"
            "QTabWidget { background-color: transparent; } "
            "QTabBar::tab { color: #000000;border: 0px solid #000000;border-bottom-right-radius: 8px;border-top-right-radius: 8px;padding: 2px;padding-top: 2px;padding-bottom: 2px;margin-left: 4px;margin-right: 4px;margin-bottom:3px;}"
            "QTabBar::tab:selected { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #CC9900, stop: 0.8 #FFE085);margin-left:0;;padding: 2px;padding-top: 2px;padding-bottom:2px;}"
            "QTabBar::tab:!selected { background-color: #C2C7CB;margin-right: 4px;;padding: 2px;padding-top: 2px;padding-bottom:2px;margin-top:0px;}"
            "QPushButton {color:black;}"
            "";
    this->setStyleSheet(myStyleSheet);

    this->windAngle->setMain(main);
    tryMoving=false;
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    this->setModal(false);
    this->show();
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
    connect(this->btn_angleFlip,SIGNAL(clicked()),this,SLOT(slot_flipAngle()));
    connect(this->spin_TWA,SIGNAL(valueChanged(double)),this,SLOT(slot_TWAChanged()));
    connect(this->spin_HDG,SIGNAL(valueChanged(double)),this,SLOT(slot_HDGChanged()));
    connect(this->btn_clearPilototo,SIGNAL(clicked()),this,SLOT(slot_clearPilototo()));
    connect(btn_pilototo,SIGNAL(clicked()),main,SLOT(slotPilototo()));
    connect(btn_clearWP,SIGNAL(clicked()),this,SLOT(slot_clearWP()));
    connect(main,SIGNAL(wpChanged()),this,SLOT(slot_updateBtnWP()));
    connect(main,SIGNAL(paramVLMChanged()),this,SLOT(slot_reloadSkin()));
    connect(main,SIGNAL(updateLockIcon(QString)),this,SLOT(slot_lock()));
    connect(main,SIGNAL(editWP_POI(POI*)),this,SLOT(slot_selectPOI(POI*)));
    connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(slot_tabChanged(int)));
    connect(btn_wp,SIGNAL(clicked()),this,SLOT(slot_editWP()));
    currentRB=NULL;
    myBoat=NULL;
    blocking=false;
    set_style(btn_sync);
    set_style(btn_pilototo);
    set_style(btn_wp);
    this->spin_HDG->installEventFilter(this);
    this->spin_TWA->installEventFilter(this);
    this->spin_PolarTWS->installEventFilter(this);
    connect(this->spin_PolarTWS,SIGNAL(valueChanged(double)),this,SLOT(slot_drawPolar()));
    lab_polar->installEventFilter(this);
    lab_polarData->clear();
//    this->lab_backTab1->installEventFilter(this);
//    this->lab_backTab2->installEventFilter(this);
//    this->lab_backTab3->installEventFilter(this);
//    this->lab_back->installEventFilter(this);
    if(main->getSettingApp(newBoard_Shadow).toInt()==1)
    {
        QGraphicsDropShadowEffect *shadow=new QGraphicsDropShadowEffect(this);
        this->setGraphicsEffect(shadow);
    }
    flipBS=false;
    myResize();
    connect(main,SIGNAL(geometryChanged()),this,SLOT(myReposition()));
    qWarning()<<"end of init board for"<<this->getName();
}
void BoardVlmMobile::myResize()
{
    mutex->lock();
    this->hide();
    //disconnect(main,SIGNAL(geometryChanged()),this,SLOT(myReposition()));
    disconnect(main,SIGNAL(drawVacInfo()),this,SLOT(drawVacInfo()));
    qWarning()<<"start myresize";
    double maxH=qMin(main->centralWidget()->height(),main->centralWidget()->width());
    this->sc=(maxH*9.0/10.0)/320.0;
    this->setFixedSize(320*sc,320*sc);
    QApplication::processEvents();
    windAngle->setScale(sc);
    fontSizeRatio=qApp->font().pointSizeF()/8.0;
    setFontDialog(this);
    this->slot_reloadSkin();
    polarImg=QPixmap(this->lab_polar->size());
    polarImg.fill(Qt::transparent);
    this->lab_polar->setPixmap(polarImg);
    lab_polarUpDw->setText("X");
    QSize sizeIcon=QToolBar().iconSize();
    lab_polarUpDw->setText("");
    btn_angleFlip->setIconSize(sizeIcon);
    btn_clearPilototo->setIconSize(sizeIcon);
    btn_clearWP->setIconSize(sizeIcon);
    QPixmap del("img/delete.png");
    del=del.scaled(sizeIcon,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    btn_clearPilototo->setIcon(QIcon(del));
    btn_clearWP->setIcon(QIcon(del));
    btn_clearPilototo->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    btn_clearWP->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    QPixmap angleFlip("img/board_angleFlip.png");
    angleFlip=angleFlip.scaled(sizeIcon,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    btn_angleFlip->setStyleSheet("QPushButton {border: none; margin: 0px;padding: 0px;}");
    btn_angleFlip->setIcon(QIcon(angleFlip));
    //drawBackground();
    myReposition();
    if(main->getSettingApp(show_DashBoard).toInt()==1)
        this->show();
    else
        hide();
    connect(main,SIGNAL(drawVacInfo()),this,SLOT(drawVacInfo()));
    connect(main,SIGNAL(geometryChanged()),this,SLOT(myReposition()));
    mutex->unlock();
    qWarning()<<"end myresize";
}
void BoardVlmMobile::myReposition()
{
    this->move(1,qRound(main->height()/2.0-this->height()/2.0));
}

void BoardVlmMobile::drawBackground()
{
//    qWarning()<<"start drawing background";
//    QApplication::processEvents();
//    QPixmap bg(backLabel->size());
//    bg.fill(this->palette().color(QPalette::Window));
//    QPainter p(&bg);
//    p.setPen(Qt::NoPen);
//    p.setBrush(QBrush(Qt::transparent));
//    p.setCompositionMode(QPainter::CompositionMode_Source);
//    QRectF r=QRect(windAngle->pos(),windAngle->size());
//    QPointF center=r.center();
//    r.setSize(QSizeF(r.size().width()*.9,r.size().height()*.9));
//    r.moveCenter(center);
//    p.drawEllipse(r);
//    p.end();
//    backLabel->setPixmap(bg);
//    qWarning()<<"end drawing background";
}

QString BoardVlmMobile::getName()
{
    return "Nouveau tableau de bord VLM par Maitai";
}
BoardVlmMobile::~BoardVlmMobile()
{
    delete mutex;
}
void BoardVlmMobile::setFontDialog(QObject * o)
{
   if(o->isWidgetType())
    {
        QWidget * widget=qobject_cast<QWidget*> (o);
        widget->setLocale(QLocale::system());
        mySetFont(widget);
    }
    foreach(QObject * object,o->children())
        this->setFontDialog(object); /*recursion*/
}
void BoardVlmMobile::mySetFont(QWidget *widget)
{
    QFont myFont=widget->font();
    myFont.setPointSizeF(qApp->font().pointSizeF());
    widget->setFont(myFont);
}

void BoardVlmMobile::slot_tabChanged(int tabNb)
{
    if(tabNb==3)
        slot_drawPolar();
}

void BoardVlmMobile::slot_reloadSkin()
{
    if(main->getSettingApp(newBoard_Shadow).toInt()==1)
    {
        QGraphicsDropShadowEffect *shadow=new QGraphicsDropShadowEffect(this);
        this->setGraphicsEffect(shadow);
    }
    else
        this->setGraphicsEffect(NULL);
#if 0
    QPixmap skin;
    QString skinName=main->getSettingApp(defaultSkin).toString();
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
    pnt.end();
    imgBack1=QPixmap (270,510);
    imgBack1.fill(Qt::transparent);
    pnt.begin(&imgBack1);
    pnt.drawPixmap(0,0,skin,300,510,270,510);
    pnt.end();
    imgBack2=QPixmap (270,510);
    imgBack2.fill(Qt::transparent);
    pnt.begin(&imgBack2);
    pnt.drawPixmap(0,0,skin,600,510,270,510);
    pnt.end();
    if(sc!=1.0)
    {
        QSize s=imgBack0.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        imgBack0=imgBack0.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=imgBack1.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        imgBack1=imgBack1.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=imgBack2.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        imgBack2=imgBack2.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
//    this->lab_backTab1->setPixmap(imgBack0);
//    this->lab_backTab2->setPixmap(imgBack1);
//    this->lab_backTab3->setPixmap(imgBack2);
//    slot_tabChanged(tabWidget->currentIndex());
#endif
    QString skinName=main->getSettingApp(defaultSkin).toString();
    if(!QFile(skinName).exists())
        skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    if(myBoat && myBoat->get_useSkin())
    {
        QString specificSkin=myBoat->get_boardSkin();
        if(QFile(specificSkin).exists())
            skinName=specificSkin;
    }
    this->windAngle->loadSkin(skinName);
    this->slot_updateData();
}
void BoardVlmMobile::slot_flipAngle()
{
    this->spin_TWA->setValue(-spin_TWA->value());
}
void BoardVlmMobile::slot_TWAChanged()
{
    if(!myBoat) return;
    double twa=spin_TWA->value();
    spin_TWA->blockSignals(true);
    if(twa>180.0)
        spin_TWA->setValue(twa-360.0);
    else if(twa<-180.0)
        spin_TWA->setValue(twa+360.0);
    spin_TWA->blockSignals(false);
    if(blocking) return;
    blocking=true;
    currentRB=this->rd_TWA;
    {
        twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: red;");
        else if(twa==qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: green;");
        else
            spin_TWA->setStyleSheet(spin_HDG->styleSheet());
    }
    double heading = myBoat->getWindDir() + spin_TWA->value();
    if(heading<0) heading+=360;
    else if(heading>360) heading-=360;
    this->spin_HDG->setValue(heading);
    double vmg=-1;
    if(myBoat->getPolarDataInterface())
    {
        myBoat->getPolarDataInterface()->bvmgWind((myBoat->getClosest().capArrival-myBoat->getWindDir()),myBoat->getWindSpeed(),&vmg);
        vmg+=myBoat->getWindDir();
        while (vmg>=360.0) vmg-=360.0;
        while (vmg<0.0) vmg+=360.0;
    }
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading,vmg);
    /* update estime */
    double newSpeed=myBoat->getSpeed();
    if(myBoat->getPolarDataInterface())
        newSpeed=myBoat->getPolarDataInterface()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
    myBoat->drawEstime(spin_HDG->value(),newSpeed);
    blocking=false;
}
void BoardVlmMobile::slot_HDGChanged()
{
    if(!myBoat) return;
    double heading=spin_HDG->value();
    spin_HDG->blockSignals(true);
    if(heading>360)
        spin_HDG->setValue(heading-360);
    spin_HDG->blockSignals(false);
    if(blocking) return;
    blocking=true;
    currentRB=this->rd_HDG;
    spin_HDG->blockSignals(true);
    spin_TWA->blockSignals(true);
    heading=spin_HDG->value();
    double angle=AngleUtil::A180(heading-myBoat->getWindDir());
    this->spin_TWA->setValue(angle);
    if(myBoat->getPolarDataInterface())
    {
        double twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: red;");
        else if(twa==qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: green;");
        else
            spin_TWA->setStyleSheet("color: black;");
    }
    double vmg=-1;
    if(myBoat->getPolarDataInterface())
    {
        myBoat->getPolarDataInterface()->bvmgWind((myBoat->getClosest().capArrival-myBoat->getWindDir()),myBoat->getWindSpeed(),&vmg);
        vmg+=myBoat->getWindDir();
        while (vmg>=360.0) vmg-=360.0;
        while (vmg<0.0) vmg+=360.0;
    }
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading,vmg);
    spin_HDG->blockSignals(false);
    spin_TWA->blockSignals(false);
    /* update estime */
    double newSpeed=myBoat->getSpeed();
    if(myBoat->getPolarDataInterface())
        newSpeed=myBoat->getPolarDataInterface()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
    myBoat->drawEstime(spin_HDG->value(),newSpeed);
    blocking=false;
}
void BoardVlmMobile::slot_lock()
{
    bool lock=false;
    if(myBoat)
        lock=myBoat->getLockStatus();
    this->spin_HDG->setDisabled(lock);
    this->spin_TWA->setDisabled(lock);
    this->rd_HDG->setDisabled(lock);
    this->rd_ORTHO->setDisabled(lock);
    this->rd_TWA->setDisabled(lock);
    this->rd_VBVMG->setDisabled(lock);
    this->rd_VMG->setDisabled(lock);
    this->btn_angleFlip->setDisabled(lock);
    this->btn_clearPilototo->setDisabled(lock);
    this->btn_clearWP->setDisabled(lock);
}

void BoardVlmMobile::slot_wpChanged()
{
    if(!myBoat) return;
    this->slot_updateBtnWP();
}

void BoardVlmMobile::slot_sendOrder()
{
    qWarning()<<"inside slot sendOrder";
    if(!myBoat || myBoat->getLockStatus()) return;
    this->blockSignals(true);
    set_style(this->btn_sync,QColor(255,0,0));
    if(rd_HDG->isChecked())
    {
        myBoat->set_pilotHeading(this->spin_HDG->value());
    }
    else if(rd_TWA->isChecked())
    {
        myBoat->set_pilotAngle(this->spin_TWA->value());
    }
    else if(rd_ORTHO->isChecked())
    {
        myBoat->set_pilotOrtho();
    }
    else if(rd_VMG->isChecked())
    {
        myBoat->set_pilotVmg();
    }
    else
    {
        myBoat->set_pilotVbvmg();
    }
    this->blockSignals(false);
}
void BoardVlmMobile::slot_vlmSync()
{
    btn_sync->setStyleSheet("background-color: rgb(255, 0, 0);");
    set_style(this->btn_sync,QColor(255,0,0));
    main->slotVLM_Sync();
}
void BoardVlmMobile::slot_updateData()
{
    windAngle->setRotation(0.0);
    if(!main->get_selectedBoatInterface() || main->get_selectedBoatInterface()->get_boatType()!=BOAT_VLM)
    {
        myBoat=NULL;
        return;
    }
    myBoat=main->get_selectedBoatInterface();
    if(!myBoat) return;
    slot_lock();
    this->blockSignals(true);
    this->blocking=true;
    updateLcds();
    QPointF position=myBoat->getPosition();
    det_POS->setText(main->formatLatitude(position.y())+"-"+main->formatLongitude(position.x()));
    if(qRound(myBoat->getDnm())<100)
        det_DNM->setText(QString().sprintf("%.2f",myBoat->getDnm())+tr("nm"));
    else
        det_DNM->setText(QString().sprintf("%d",qRound(myBoat->getDnm()))+tr("nm"));
    det_ORT->setText(QString().sprintf("%.2f",myBoat->getOrtho())+tr("deg"));
    det_VMG->setText(QString().sprintf("%.2f",myBoat->getVmg())+tr("kts"));
    double WPAngle=myBoat->getWPangle();
    if(WPAngle>180.0) WPAngle-=360.0;
    det_ANGLE->setText(QString().sprintf("%.2f",WPAngle)+tr("deg"));
    det_TWS->setText(QString().sprintf("%.2f",myBoat->getWindSpeed())+tr("kts"));
    det_TWD->setText(QString().sprintf("%.2f",myBoat->getWindDir())+tr("deg"));
    if(myBoat->getPolarDataInterface())
    {
        det_UPWind->setText(QString().sprintf("%.2f",myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed()))+tr("deg"));
        det_DwWind->setText(QString().sprintf("%.2f",myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed()))+tr("deg"));
    }
    if(qRound(myBoat->getLoch())<100)
        det_LOCH->setText(QString().sprintf("%.2f",myBoat->getLoch())+tr("nm"));
    else
        det_LOCH->setText(QString().sprintf("%d",qRound(myBoat->getLoch()))+tr("nm"));
    this->det_BS->setText(QString().sprintf("%.2f",myBoat->getSpeed())+tr("kts"));
    this->det_HDG->setText(QString().sprintf("%.2f",myBoat->getHeading())+tr("deg"));
    this->det_AVG->setText(QString().sprintf("%.2f",myBoat->getAvg())+tr("kts"));
    this->lab_RANK->setText(myBoat->getBoatName()+" "+myBoat->getScore()+" ("+QString().sprintf("%d",myBoat->getRank())+")");
    this->det_boatBox->setTitle(lab_RANK->text());
    this->det_raceBox->setTitle(myBoat->getRaceName());
    this->det_GATE_ORT->setText(QString().sprintf("%.2f",myBoat->getClosest().capArrival)+tr("deg"));
    double vmg=0;
    if(myBoat->getPolarDataInterface())
    {
        myBoat->getPolarDataInterface()->bvmgWind((myBoat->getClosest().capArrival-myBoat->getWindDir()),myBoat->getWindSpeed(),&vmg);
        vmg+=myBoat->getWindDir();
        while (vmg>=360.0) vmg-=360.0;
        while (vmg<0.0) vmg+=360.0;
    }
    this->det_GATE_VMG->setText(QString().sprintf("%.2f",vmg)+tr("deg"));
    if(qRound(myBoat->getClosest().distArrival)<100)
        this->det_GATE_DIST->setText(QString().sprintf("%.2f",myBoat->getClosest().distArrival)+tr("nm"));
    else
        this->det_GATE_DIST->setText(QString().sprintf("%d",qRound(myBoat->getClosest().distArrival))+tr("nm"));
    if(!myBoat->getGates().isEmpty())
        this->det_GATE->setText("->"+myBoat->getGates().at(myBoat->getNWP()-1)->getDesc());
    this->spin_HDG->setValue(myBoat->getHeading());
    this->spin_TWA->setValue(computeAngle());
    if(myBoat->getPolarDataInterface())
    {
        double twa=qAbs(qRound(spin_TWA->value()*10.0));
        if(twa<qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa>qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: red;");
        else if(twa==qRound(myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed())*10.0) ||
           twa==qRound(myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed())*10.0))
            spin_TWA->setStyleSheet("color: green;");
        else
            spin_TWA->setStyleSheet("");
        QString tipTWA=tr("Meilleurs angles au pres/portant:")+" "+QString().sprintf("%.2f",myBoat->getPolarDataInterface()->getBvmgUp(myBoat->getWindSpeed()))+tr("deg")+"/"
                +QString().sprintf("%.2f",myBoat->getPolarDataInterface()->getBvmgDown(myBoat->getWindSpeed()))+tr("deg");
        spin_TWA->setToolTip("<p style='white-space:pre'>"+tipTWA+"</p>");
    }
    switch(myBoat->getPilotType())
    {
    case 1:
        this->rd_HDG->setChecked(true);
        break;
    case 2:
        this->rd_TWA->setChecked(true);
        break;
    case 3:
        this->rd_ORTHO->setChecked(true);
        break;
    case 4:
        this->rd_VMG->setChecked(true);
        break;
    case 5:
        this->rd_VBVMG->setChecked(true);
        break;
    }
    vmg=-1;
    if(myBoat->getPolarDataInterface())
    {
        myBoat->getPolarDataInterface()->bvmgWind((myBoat->getClosest().capArrival-myBoat->getWindDir()),myBoat->getWindSpeed(),&vmg);
        vmg+=myBoat->getWindDir();
        while (vmg>=360.0) vmg-=360.0;
        while (vmg<0.0) vmg+=360.0;
    }
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,-1,vmg);
    update_btnPilototo();
    slot_updateBtnWP();
    spin_PolarTWS->setValue(myBoat->getWindSpeed());
    this->slot_drawPolar();
    this->blockSignals(false);
    this->blocking=false;
    set_style(this->btn_sync,QColor(20,255,20));
}
void BoardVlmMobile::slot_drawPolar()
{
    if(!myBoat) return;
    lab_polarData->clear();
    polarImg=QPixmap(this->lab_polar->size());
    polarImg.fill(Qt::transparent);
    PolarInterface * polar=myBoat->getPolarDataInterface();
    if(!polar)
    {
        lab_polarName->setText(tr("pas de polaire chargee"));
        lab_polar->setPixmap(polarImg);
        return;
    }
    lab_polarName->setText(polar->getName());
    polarPnt.begin(&polarImg);
    QFont ff=qApp->font();
    ff.setPointSizeF(ff.pointSizeF()*.8);
    polarPnt.setFont(ff);
    polarPnt.setRenderHint(QPainter::Antialiasing);
    double maxSpeed=-1;
    double maxSpeedKts=maxSpeed;
    double maxSpeedTwa=0;
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
        s="("+QString().sprintf("%.1f",bvmgUp)+tr("deg")+"/"+QString().sprintf("%.1f",bvmgDown)+tr("deg")+")";
        this->lab_polarUpDw->setText(s);
        for(int angle=0;angle<=180;++angle)
        {
            double speed=polar->getSpeed(ws,angle,false);
            polarValues.append(speed);
            if(speed>maxSpeed )
            {
                maxSpeed=speed;
                maxSpeedTwa=angle;
            }
        }
        maxSpeedKts=maxSpeed;
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
        pen.setColor(Qt::red);
        pen.setWidth(2*sc);
        polarPnt.setPen(pen);
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
    pen.setColor(palette().color(QPalette::WindowText));
    pen.setWidthF(0.5*sc);
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
    pen.setColor(main->getWindColorStatic(myBoat->getWindSpeed(),main->getSettingApp(colorMapSmoothSet).toBool()));
    pen.setWidthF(2.0*sc);
    polarPnt.setPen(pen);
    QLineF line(center,QPointF(polarImg.width(),polarImg.height()/2.0));
    line.setAngle(90-(qAbs(spin_TWA->value())));
    line.setLength((maxSpeed+1)*maxSize/maxSpeed);
    polarPnt.drawLine(line);
    polarPnt.end();
    this->lab_polar->setPixmap(polarImg);
    double Pbs,Ptwa,Ptws;
    polar->getMaxSpeedData(&Pbs,&Ptws,&Ptwa);
    stringMaxSpeed=tr("Max Speed:")+QString().sprintf("<br>%.2f ",maxSpeedKts)+tr("kts")+
                      " "+tr("at")+"<br>"+QString().sprintf("%.2f",maxSpeedTwa)+tr("deg")+"<br><br>"+
                   tr("Absolute<br>max speed:")+"<br>"+
                   tr("TWS:")+"<br>"+QString().sprintf("%.2f ",Ptws)+tr("kts")+"<br>"+
                   tr("TWA:")+"<br>"+QString().sprintf("%.2f",Ptwa)+tr("deg")+"<br>"+
                   tr("BS:")+"<br>"+QString().sprintf("%.2f ",Pbs)+tr("kts");
    lab_polarData->setText(stringMaxSpeed);
}
void BoardVlmMobile::slot_outDatedVlmData()
{
    if (!(btn_sync->styleSheet()).contains(QColor(255, 0, 0).name())) //if red stays red
        set_style(this->btn_sync,QColor(255,191,21));
}
void BoardVlmMobile::updateLcds()
{
    if(!myBoat) return;
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
    s.sprintf("%.2f",(double)qRound(myBoat->getTWA()*100.0)/100.0);
    lcd_TWA->setDigitCount(s.count());
    this->lcd_TWA->display(s);
    QColor color=main->getWindColorStatic(myBoat->getWindSpeed(),main->getSettingApp(colorMapSmoothSet).toBool());
    this->lcd_TWS->setStyleSheet((QString().sprintf("color: black;background-color: rgb(%d, %d, %d);",color.red(),color.green(),color.blue())));
    color=Qt::white;
    this->lcd_TWD->setStyleSheet((QString().sprintf("color:black;background-color: rgba(%d, %d, %d,%d);",color.red(),color.green(),color.blue(),180)));
    this->lcd_BS->setStyleSheet((QString().sprintf("color:black;background-color: rgba(%d, %d, %d, %d);",color.red(),color.green(),color.blue(),180)));
    this->lcd_TWA->setStyleSheet((QString().sprintf("color:black;background-color: rgba(%d, %d, %d, %d);",color.red(),color.green(),color.blue(),180)));
}
void BoardVlmMobile::slot_timerElapsed()
{
    if(!myBoat) return;
    if(currentRB==rd_TWA)
        rd_HDG->setStyleSheet("");
    else
        rd_TWA->setStyleSheet("");
    if(flipBS)
        currentRB->setStyleSheet("color: cyan");
    else
        currentRB->setStyleSheet("");
    flipBS=!flipBS;
    double speed=myBoat->getSpeed();
    QColor color=Qt::white;
    if(myBoat && flipBS && myBoat->getPolarDataInterface())
    {
        speed=myBoat->getPolarDataInterface()->getSpeed(myBoat->getWindSpeed(),qAbs(spin_TWA->value()));
        color=Qt::green;
        color=color.lighter();
    }
    QString s;
    s.sprintf("%.2f",(double)qRound(speed*100.0)/100.0);
    lcd_BS->setDigitCount(s.count());
    this->lcd_BS->display(s);
    this->lcd_BS->setStyleSheet((QString().sprintf("color:black;background-color: rgba(%d, %d, %d, %d);",color.red(),color.green(),color.blue(),180)));
}
double BoardVlmMobile::computeAngle(void) { /* we assume a boat exists => should be tested by caller */
    if(!myBoat) return 0;
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
void BoardVlmMobile::slot_clearPilototo(void)
{
    if(!myBoat) return;
    main->slot_clearPilototo();
}
void BoardVlmMobile::update_btnPilototo()
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

}
void BoardVlmMobile::set_style(QPushButton * button, QColor color, QColor color2)
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
void BoardVlmMobile::slot_editWP()
{
    if(!myBoat) return;
    if(main->get_selPOI_instruction())
        main->slot_POIselected(NULL);
    else
        main->manageWPDialog(myBoat,this);
}

void BoardVlmMobile::slot_updateBtnWP(void)
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
            wpPos+=main->pos2String(TYPE_LAT,WPLat);

        wpPos+=", ";

        if(WPLon==0)
            wpPos+="0 E";
        else
            wpPos+=main->pos2String(TYPE_LON,WPLon);

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
//            str+=main->pos2String(TYPE_LAT,WPLat);

//        str+=", ";

//        if(WPLon==0)
//            str+="0 E";
//        else
//            str+=main->pos2String(TYPE_LON,WPLon);

//        if(WPHd!=-1)
//        {
//            str+=" @";
//            str+=QString().sprintf("%.1f",WPHd);
//            str+=tr("deg");
//        }

    }
}
void BoardVlmMobile::slot_clearWP()
{
    if(!myBoat) return;
    QPointF pos(0,0);
    myBoat->setWP(pos,-1.0);
}
void BoardVlmMobile::slot_selectPOI(bool doSelect)
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
void BoardVlmMobile::slot_selectPOI(POI *)
{
    this->set_enabled(true);
    slot_updateBtnWP();
}
void BoardVlmMobile::slot_selectWP_POI()
{
    btn_wp->setText(tr("Annuler"));
    set_style(btn_wp,QColor(151,179,210));/*blue*/
    this->set_enabled(false);
    btn_wp->setEnabled(true);
    main->slotSelectWP_POI();
}
void BoardVlmMobile::set_enabled(const bool &b)
{
    this->btn_angleFlip->setEnabled(b);
    this->btn_clearPilototo->setEnabled(b);
    this->btn_clearWP->setEnabled(b);
    this->btn_sync->setEnabled(b);
    this->btn_wp->setEnabled(b);
    this->btn_pilototo->setEnabled(b);
    this->rd_HDG->setEnabled(b);
    this->rd_ORTHO->setEnabled(b);
    this->rd_TWA->setEnabled(b);
    this->rd_VBVMG->setEnabled(b);
    this->rd_VMG->setEnabled(b);
    this->spin_HDG->setEnabled(b);
    this->spin_TWA->setEnabled(b);
}
bool BoardVlmMobile::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==lab_polar)
    {
        if(event->type()==QEvent::MouseButtonRelease)
        {
            lab_polar->setPixmap(polarImg);
            lab_polarData->setText(stringMaxSpeed);
            return true;
        }
        if(event->type()!=QEvent::MouseMove || polarLine.isEmpty())
            return false;
        QMouseEvent * mouseEvent=static_cast<QMouseEvent*>(event);
        QPixmap i2=polarImg;
        QPen pe(Qt::blue);
        pe.setWidthF(3*sc);
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
        pe.setWidthF(1*sc);
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
//    if(obj==lab_backTab1 || obj==lab_backTab2 || obj==lab_backTab3 || obj==lab_back)
//    {
//        if(event->type()==QEvent::MouseButtonPress)
//        {
//            tryMoving=true;
//            QMouseEvent *m=static_cast<QMouseEvent *>(event);
//            startMove=mapToGlobal(m->pos());
//            initialPos=this->pos();
//        }
//        else if(event->type()==QEvent::MouseButtonRelease)
//        {
//            tryMoving=false;
//            nbVib=5;
//            windAngle->setRotation(0);
//            //vibration->start(50);
//        }
//        else if(tryMoving && event->type()==QEvent::MouseMove)
//        {
//            QMouseEvent *m=static_cast<QMouseEvent *>(event);
//            QPoint mousePos=mapToGlobal(m->pos());
//            int x=qBound(qRound(-this->width()/2.0),initialPos.x()+mousePos.x()-startMove.x(),qRound(main->width()-this->width()/2.0));
//            int y=qBound(qRound(-this->height()/2.0),initialPos.y()+mousePos.y()-startMove.y(),qRound(main->height()-this->height()/2.0));
//            QLineF L=QLine(x,y,this->x(),this->y());
//            int i=qRound(2000.0/(double)main->width())*L.length();
//            if (this->x()>x)
//                i=-i;
//            this->move(x,y);
//            windAngle->setRotation(windAngle->getRotation()+i);
//        }
//        return false;
//    }
    if(obj!=spin_HDG && obj!=spin_TWA && obj!=spin_PolarTWS) return false;
    QDoubleSpinBox * spinBox=static_cast<QDoubleSpinBox *>(obj);
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
#ifdef __QTVLM_SHIFT_INC_MOD
        if(keyEvent->key()==Qt::Key_Shift)
            spinBox->setSingleStep(0.1);
        else if(keyEvent->key()==Qt::Key_Control)
            spinBox->setSingleStep(10.0);
        else if(keyEvent->key()==Qt::Key_Alt)
            spinBox->setSingleStep(0.01);
#else
        if(keyEvent->key()==Qt::Key_Control)
            spinBox->setSingleStep(0.1);
        else if(keyEvent->key()==Qt::Key_Alt)
            spinBox->setSingleStep(0.01);
#endif
    }
    else if (event->type()==QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(
#ifdef __QTVLM_SHIFT_INC_MOD
                keyEvent->key()==Qt::Key_Shift ||
#endif
            keyEvent->key()==Qt::Key_Control || keyEvent->key()==Qt::Key_Alt)
            spinBox->setSingleStep(1.0);
    }
    if(event->type()==QEvent::Wheel)
    {
        /*by default wheeling with ctrl already multiply singleStep by 10
          so to get 10 you need to put 1...*/
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        if(wheelEvent->modifiers()==Qt::ControlModifier)
            spinBox->setSingleStep(0.01);
    }
    return false;
}
bool BoardVlmMobile::confirmChange()
{
    if(main->getSettingApp(askConfirmation).toInt()==0)
        return true;

    return QMessageBox::question(0,tr("Confirmation a chaque ordre vers VLM"),
                                 tr("Confirmez-vous cet ordre?"),QMessageBox::Yes|QMessageBox::No,
                             QMessageBox::Yes)==QMessageBox::Yes;
}
void BoardVlmMobile::drawVacInfo(void)
{
    if(!myBoat) return;
    QDateTime lastVac_date;
    lastVac_date.setTimeSpec(Qt::UTC);
    lastVac_date.setTime_t(myBoat->getPrevVac());
    btn_sync->setToolTip(tr("Derniere synchro") + ": " + lastVac_date.toString(tr("dd-MM-yyyy, HH:mm:ss")));
    btn_sync->setText("Sync: "+QString().setNum(main->get_nxtVac_cnt()) + "s");
}
/*********************/
/* VLM20 windAngle   */
/*********************/

VlmCompass::VlmCompass(QWidget * parent):QWidget(parent)
{
    sc=1.0;
    setFixedSize(200,200);
    WPdir = -1;
    newHeading=-1;
    rotation=0;
    main=NULL;
    this->setAttribute(Qt::WA_TranslucentBackground);
}
void VlmCompass::loadSkin(const QString &SkinName)
{
    if(!main) return;
    QPixmap skin;
    QString skinName=SkinName;
    if(skinName.isEmpty())
    {
        skinName=main->getSettingApp(defaultSkin).toString();
        if(!QFile(skinName).exists())
            skinName=QFileInfo("img/skin_compas.png").absoluteFilePath();
    }
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
    img_arrow_gateVmg=QPixmap(200,200);
    img_arrow_gateVmg.fill(Qt::transparent);
    pnt.begin(&img_arrow_gateVmg);
    pnt.drawPixmap(0,0,skin,800,300,200,200);
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
    if(sc!=1.0)
    {
        QSize s=img_fond.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_fond=img_fond.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=img_boat.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_boat=img_boat.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=img_arrow_wp.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_arrow_wp=img_arrow_wp.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=img_arrow_gateVmg.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_arrow_gateVmg=img_arrow_gateVmg.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=img_arrow_gate.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_arrow_gate=img_arrow_gate.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        s=img_arrow_wind.size();
        s.setHeight(s.height()*sc);
        s.setWidth(s.width()*sc);
        img_arrow_wind=img_arrow_wind.scaled(s,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    }
}

void VlmCompass::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setViewport(0,0,200*sc,200*sc);

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
    painter->translate(100*sc,100*sc);
    painter->rotate(heading+rotation);
    painter->drawPixmap(-100*sc,-100*sc,img_boat);
    painter->restore();
    if(newHeading!=heading && newHeading!=-1)
    {
        QPixmap tempBoat=img_boat;
        QPainter pnt(&tempBoat);
        pnt.setRenderHint(QPainter::Antialiasing,true);
        pnt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pnt.fillRect(0,0,200*sc,200*sc,QBrush(QColor(0,0,0,100*sc)));
        pnt.end();
        painter->save();
        painter->translate(100*sc,100*sc);
        painter->rotate(newHeading+rotation);
        painter->drawPixmap(-100*sc,-100*sc,tempBoat);
        painter->restore();
    }
    if(WPdir!=-1)
    {
        painter->save();
        painter->translate(100*sc,100*sc);
        painter->rotate(WPdir);
        painter->drawPixmap(-100*sc,-100*sc,img_arrow_wp);
        painter->restore();
    }
    painter->save();
    painter->translate(100*sc,100*sc);
    painter->rotate(gateDir);
    painter->drawPixmap(-100*sc,-100*sc,img_arrow_gate);
    painter->restore();
    if(gateVmg!=-1)
    {
        painter->save();
        painter->translate(100*sc,100*sc);
        painter->rotate(gateVmg);
        painter->drawPixmap(-100*sc,-100*sc,img_arrow_gateVmg);
        painter->restore();
    }
    QPixmap tempWind=img_arrow_wind;
    QPainter pnt(&tempWind);
    pnt.setRenderHint(QPainter::Antialiasing,true);
    pnt.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    pnt.fillRect(0,0,200*sc,200*sc,QBrush(windSpeed_toColor()));
    pnt.end();
    painter->save();
    painter->translate(100*sc,100*sc);
    painter->rotate(windDir);
    painter->drawPixmap(-100*sc,-100*sc,tempWind);
    painter->restore();
}

QColor VlmCompass::windSpeed_toColor()
{
    if(!main) return QColor(Qt::black);
    return main->getWindColorStatic(windSpeed,main->getSettingApp(colorMapSmoothSet).toBool());
}

void VlmCompass::setValues(const double &heading, const double &windDir, const double &windSpeed, const double &WPdir, const double &gateDir, const double &newHeading, const double &newGateVmg)
{
    //qWarning() << "windAngle set: heading=" << heading << " windDir=" << windDir << " windSpeed=" << windSpeed << " WPdir=" << WPdir << " " << newHeading;
    this->heading=heading;
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->WPdir=WPdir;
    this->newHeading=newHeading;
    this->gateDir=gateDir;
    this->gateVmg=newGateVmg;
    update();
}
void VlmCompass::setScale(const double &s)
{
    this->sc=s;
    setFixedSize(qRound(200*sc),qRound(200*sc));
//    this->move(20*sc,90*sc);
}
