#include "BoardVlmNew.h"
#include "ui_BoardVlmNew.h"
#include "boatVLM.h"
BoardVlmNew::BoardVlmNew(MainWindow *main)
    : QDialog(main)

{
    this->setParent(main);
    this->setupUi(this);
    Util::setFontDialog(this);
    this->main=main;
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    this->setModal(false);
    this->move(1,qRound((main->height()-this->height())/2.0));
    this->show();
    btn_angleFlip->setIcon(QIcon("img/board_angleFlip.png"));
    btn_clearPilototo->setIcon(QIcon("img/del.png"));
    btn_clearWP->setIcon(QIcon("img/del.png"));
    defaultStyleSheet=this->lab_TWA->styleSheet();
    connect(this->btn_sync,SIGNAL(clicked()),this,SLOT(slot_vlmSync()));
    connect(main,SIGNAL(boatHasUpdated(boat*)),this,SLOT(slot_updateData()));
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
    timer=new QTimer(this);
    timer->setInterval(600);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(slot_timerElapsed()));
    currentRB=NULL;
    myBoat=NULL;
    blocking=false;
}

BoardVlmNew::~BoardVlmNew()
{
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
    double heading = myBoat->getWindDir() + spin_TWA->value();
    if(heading<0) heading+=360;
    else if(heading>360) heading-=360;
    this->spin_HDG->setValue(heading);
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading);
    if(!timer->isActive())
        timer->start();
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
    this->windAngle->setValues(myBoat->getHeading(),myBoat->getWindDir(),myBoat->getWindSpeed(),myBoat->getWPdir(),myBoat->getClosest().capArrival,heading);
    spin_HDG->blockSignals(false);
    spin_TWA->blockSignals(false);
    if(!timer->isActive())
        timer->start();
    blocking=false;
}

void BoardVlmNew::slot_wpChanged()
{
    timerStop();
    if(!myBoat) return;
    //btn_sync->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
}

void BoardVlmNew::slot_sendOrder()
{
    timerStop();
    if(!myBoat) return;
    this->blockSignals(true);
    btn_sync->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
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
    this->blockSignals(false);
}
void BoardVlmNew::slot_dialChanged(int value)
{
    timerStop();
    switch(value)
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
    main->slotVLM_Sync();
}
void BoardVlmNew::slot_updateData()
{
    timerStop();
    myBoat=(boatVLM*)main->getSelectedBoat();
    if(!myBoat) return;
    this->blockSignals(true);
    btn_sync->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));
    updateLcds();
    this->spin_HDG->setValue(myBoat->getHeading());
    this->spin_TWA->setValue(computeAngle());
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
    this->blockSignals(false);

}
void BoardVlmNew::slot_outDatedVlmData()
{
    btn_sync->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 191, 21);"));
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
}
void BoardVlmNew::slot_timerElapsed()
{
    if(currentRB==lab_TWA)
        lab_TWS->show();
    else
        lab_TWA->show();
    currentRB->setVisible(!currentRB->isVisible());
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
