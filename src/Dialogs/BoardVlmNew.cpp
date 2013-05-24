#include "BoardVlmNew.h"
#include "ui_BoardVlmNew.h"
#include "boatVLM.h"
#include "POI.h"
#include "DialogWp.h"
#include "Polar.h"
#include <QBitmap>
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
    QPixmap del("img/delete.png");
    btn_clearWP->setMask(del.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
    btn_clearPilototo->setMask(del.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
    btn_clearPilototo->setIcon(QIcon(del));
    btn_clearWP->setIcon(QIcon(del));
    del.load("img/board_angleFlip.png");
    btn_angleFlip->setIcon(QIcon(del));
    btn_angleFlip->setMask(del.createMaskFromColor(Qt::transparent,Qt::MaskInColor));
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
    connect(this->btn_clearPilototo,SIGNAL(clicked()),this,SLOT(slot_clearPilototo()));
    connect(btn_pilototo,SIGNAL(clicked()),main,SLOT(slotPilototo()));
    connect(btn_clearWP,SIGNAL(clicked()),this,SLOT(slot_clearWP()));
    connect(main,SIGNAL(selectPOI(bool)),this,SLOT(slot_selectPOI(bool)));
    connect(main,SIGNAL(wpChanged()),this,SLOT(slot_updateBtnWP()));
    wpDialog = new DialogWp();
    connect(wpDialog,SIGNAL(selectPOI()),this,SLOT(slot_selectWP_POI()));
    connect(wpDialog,SIGNAL(selectPOI()),main,SLOT(slotSelectWP_POI()));
    connect(main,SIGNAL(editWP_POI(POI*)),this,SLOT(slot_selectPOI(POI*)));
    connect(main,SIGNAL(editWP_POI(POI*)),wpDialog,SLOT(show_WPdialog(POI *)));
    connect(btn_wp,SIGNAL(clicked()),this,SLOT(slot_editWP()));
    timer=new QTimer(this);
    timer->setInterval(600);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(slot_timerElapsed()));
    currentRB=NULL;
    myBoat=NULL;
    blocking=false;
    set_style(btn_sync);
    set_style(btn_pilototo);
    set_style(btn_wp);
    this->spin_HDG->installEventFilter(this);
    this->spin_TWA->installEventFilter(this);
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
    if(qAbs(spin_TWA->value())<myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()) ||
       qAbs(spin_TWA->value())>myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))
        spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
    else
        spin_TWA->setStyleSheet("color: black;");
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
    if(qAbs(spin_TWA->value())<myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()) ||
       qAbs(spin_TWA->value())>myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))
        spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
    else
        spin_TWA->setStyleSheet("color: black;");
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
    set_style(this->btn_sync,QColor(255,0,0));
    main->slotVLM_Sync();
}
void BoardVlmNew::slot_updateData()
{
    timerStop();
    myBoat=(boatVLM*)main->getSelectedBoat();
    if(!myBoat) return;
    this->blockSignals(true);
    set_style(this->btn_sync,QColor(255,255,127));
    updateLcds();
    this->spin_HDG->setValue(myBoat->getHeading());
    this->spin_TWA->setValue(computeAngle());
    if(qAbs(spin_TWA->value())<myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()) ||
       qAbs(spin_TWA->value())>myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))
        spin_TWA->setStyleSheet("QDoubleSpinBox {color: red;} QDoubleSpinBox QWidget {color:black;}");
    else
        spin_TWA->setStyleSheet("color: black;");
    QString tipTWA=tr("Meilleurs angles au pres/portant:")+" "+QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgUp(myBoat->getWindSpeed()))+tr("deg")+"/"
            +QString().sprintf("%.2f",myBoat->getPolarData()->getBvmgDown(myBoat->getWindSpeed()))+tr("deg");
    spin_TWA->setToolTip("<p style='white-space:pre'>"+tipTWA+"</p>");
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
void BoardVlmNew::slot_clearPilototo(void)
{
    //btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
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
                if(main->getPois()->at(n)->getWph()==WPHd)
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
    if(obj!=spin_HDG && obj!=spin_TWA) return false;
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
