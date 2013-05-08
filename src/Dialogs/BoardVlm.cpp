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

#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>

#include "BoardVlm.h"
#include "Board.h"
#include "BoardComponent.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "boatVLM.h"
#include "settings.h"
#include "Util.h"
#include "DialogWp.h"
#include "Polar.h"


#define POLAR_SPEED_MODE   1
#define NOPOLAR_SPEED_MODE 2
#define VLM_SPEED_MODE     3

#define SPEED_COLOR_UPDATE    "color: rgb(100, 200, 0);"
#define SPEED_COLOR_VLM       "color: rgb(255, 0, 0);"
#define SPEED_COLOR_NO_POLAR  "color: rgb(255, 170, 127);"

#define ACTIVE_BOAT_TYPE BOAT_VLM
#define ACTIVE_BOAT_CAST boatVLM

/************************************************************/
/*   BoardVlmUi                                      */
/************************************************************/
BoardVlmUi::BoardVlmUi(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);

    init_dock(tr("Vlm"),BOARD_TYPE_VLM);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));

    connect(mainWindow,SIGNAL(accountListUpdated(Player*)),this,SLOT(slot_initBoatList(Player*)));
    connect(board,SIGNAL(sig_setCurrentBoat(boat*)),this,SLOT(slot_setCurrentBoat(boat*)));
    connect(btn_races,SIGNAL(clicked()),mainWindow->getMy_centralWidget(),SLOT(slot_raceDialog()));
    connect(this,SIGNAL(VLM_Sync()),mainWindow,SLOT(slotVLM_Sync()));
    connect(this,SIGNAL(selectBoat(boat*)),mainWindow,SLOT(slotSelectBoat(boat*)));
    connect(mainWindow,SIGNAL(updateLockIcon(QIcon)),this,SLOT(slot_updateLockIcon(QIcon)));
    connect(btnLock, SIGNAL(clicked()), mainWindow, SLOT(slotFile_Lock()));
    connect(mainWindow,SIGNAL(setChangeStatus(bool,bool,bool)),this,SLOT(slot_setChangeStatus(bool,bool,bool)));
    connect(mainWindow,SIGNAL(outDatedVlmData()),this,SLOT(slot_outDatedVlmData()));
}

BoardVlmUi::~BoardVlmUi() {

}

void BoardVlmUi::slot_initBoatList(Player * vlmPlayer) {
    boatList->setEnabled(false);
    boatList->clear();

    QListIterator<boatVLM*> i (*vlmPlayer->getBoats());
    while(i.hasNext()) {
        boatVLM * boatVlm = i.next();
        QVariant var=VPtr<boatVLM>::asQVariant(boatVlm);
        if(boatVlm->getStatus()) {
            if(boatVlm->getAliasState())
                boatList->addItem(boatVlm->getAlias() + "(" + boatVlm->getBoatPseudo() + ")",var);
            else
                boatList->addItem(boatVlm->getBoatPseudo(),var);
        }
    }

    boatList->setEnabled(true);
}

void BoardVlmUi::slot_setCurrentBoat(boat *myBoat) {
    if(!myBoat)
        return;

    if(myBoat->get_boatType()!=BOAT_VLM)
        return;

    boatVLM * boatVlm=(boatVLM*)myBoat;

    /* update list selected item to point to correct boat */
    this->blockSignals(true);
    for(int i=0;i<boatList->count();i++) {
        QVariant var=boatList->itemData(i);
        boatVLM * ptr = VPtr<boatVLM>::asPtr(var);
        if(ptr==boatVlm) {
            boatList->setCurrentIndex(i);
            break;
        }
    }
    this->blockSignals(false);
}

void BoardVlmUi::slot_boatSelected(int index) {
    QVariant var=boatList->itemData(index);
    boatVLM * boatVlm = VPtr<boatVLM>::asPtr(var);
    emit selectBoat(boatVlm);
}

void BoardVlmUi::slot_updateData(void) {
    INIT_BOAT;

    boatScore->setText(boat->getScore()+" ("+QString().sprintf("%d",boat->getRank())+")");
    /*put back synch button to wait color*/
    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 127);"));

}

void BoardVlmUi::slot_showBoatInfo(void) {
    INIT_BOAT;

    QString polar_str=boat->getPolarName();

    if(boat->getPolarData())
    {
        if(boat->getPolarState())
            polar_str+= " ("+tr("forcee")+")";
        if (boat->getPolarData()->getIsCsv())
            polar_str += " - format csv";
        else
            polar_str += " - format pol";
    }
    else
        polar_str+= " ("+tr("erreur chargement")+")";

    QString boatID=boat->getDispName();
    QMessageBox::information(this,tr("Information sur")+" " + boatID,
                             (boat->getAliasState()?tr("Alias:")+ " " + boat->getAlias() + "\n":"") +
                             tr("ID:") + " " + boat->getBoatId() + "\n" +
                             tr("Pseudo:") + " " + boat->getBoatPseudo() + "\n" +
                             tr("Nom:") + " " + boat->getBoatName() + "\n" +
                             tr("Email:") + " " + boat->getEmail() + "\n" +
                             tr("Course:") + " (" + boat->getRaceId() + ") " + boat->getRaceName() + "\n" +
                             tr("Score:") + " " + boat->getScore() + "\n" +
                             tr("Polaire:") + " " + polar_str
                             );
}

void BoardVlmUi::slot_vlmSynch(void) {
    INIT_BOAT;

    //set_style(btn_Synch,QColor(255,0,0));
    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));

    emit VLM_Sync();
}

void BoardVlmUi::slot_toggleGps(bool /*toggleState*/) {

}

void BoardVlmUi::slot_updateLockIcon(QIcon ic) {
    btnLock->setIcon(ic);
}

void BoardVlmUi::slot_setChangeStatus(bool ,bool ,bool btnSync) {
    btn_Synch->setEnabled(btnSync);
}

void BoardVlmUi::slot_outDatedVlmData(void) {
    btn_Synch->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 191, 21);"));
}

/************************************************************/
/*   BoardPilotVLMBoat                                      */
/************************************************************/
BoardPilotVLMBoat::BoardPilotVLMBoat(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);

    init_dock(tr("Boat control"),BOARD_TYPE_PILOT);

    btn_angleFlip->setIcon(QIcon("img/board_angleFlip.png"));
    btn_setAngle->setIcon(QIcon("img/apply.png"));
    btn_setHeading->setIcon(QIcon("img/apply.png"));
    blocking=false;

    /* init list of radio buttons */
    rdList.clear();
    rdList.append(this->rd_heading);
    rdList.append(this->rd_angle);    
    rdList.append(this->rd_ortho);
    rdList.append(this->rd_VMG);
    rdList.append(this->rd_vbvmg);

    /* init default bg of radio btn */
    rdStyleSheet=this->rd_heading->styleSheet();

    /* validation button */
    btn_setHeading->setEnabled(false);
    btn_setAngle->setEnabled(false);

    /* make some connection */
    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
    connect(editHeading,SIGNAL(hasEvent()),this,SLOT(slot_hasEvent()));
    connect(editAngle,SIGNAL(hasEvent()),this,SLOT(slot_hasEvent()));
    connect(btn_Pilototo,SIGNAL(clicked()),mainWindow,SLOT(slotPilototo()));
    connect(btn_clearPilototo,SIGNAL(clicked()),this,SLOT(slot_clearPilototo()));
    connect(mainWindow,SIGNAL(selectPOI(bool)),this,SLOT(slot_selectPOI(bool)));
    connect(mainWindow,SIGNAL(setChangeStatus(bool,bool,bool)),this,SLOT(slot_setChangeStatus(bool,bool,bool)));
}

BoardPilotVLMBoat::~BoardPilotVLMBoat() {
    //disconnect(boardVlm,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

void BoardPilotVLMBoat::slot_selectPOI(bool doSelect) {
    if(doSelect)
        btn_Pilototo->setText(tr("Select WP - click to cancel"));
    else
        updatePilototBtn();
}

void BoardPilotVLMBoat::slot_setChangeStatus(bool status,bool pilototo,bool ) {
    slot_updateData(); // make sure to put data back to boat values
    bool st=!status;
    for(int i=0;i<rdList.count();++i)
        rdList.at(i)->setEnabled(st);

    btn_angleFlip->setEnabled(st);
    btn_clearPilototo->setEnabled(st);
    editAngle->setEnabled(st);
    editHeading->setEnabled(st);
    btn_Pilototo->setEnabled(pilototo);
}

void BoardPilotVLMBoat::slot_hasEvent()
{
    QObject *s = sender();
    if(s==NULL) return;

    if(s==editHeading)
        slot_setHeading();
    else if(s==editAngle)
        slot_setAngle();
}

double BoardPilotVLMBoat::computeAngle(void) { /* we assume a boat exists => should be tested by caller */
    INIT_BOAT_T(0);
    double angle_val;
    double val=boat->getHeading()-boat->getWindDir();
    if(boat->getPilotType()==2)
        angle_val = boat->getPilotString().toDouble();
    else
    {
        angle_val = boat->getTWA();
        calcAngleSign(val,angle_val)
    }
    return angle_val;
}

void BoardPilotVLMBoat::slot_updateData(void) {
    INIT_BOAT;
    blocking=true;
    int pilotType=boat->getPilotType();

    /*restoring rd button bg*/
    for(int i=0;i<rdList.count();++i) {
        rdList.at(i)->setStyleSheet(rdStyleSheet);
    }

    if(pilotType>0 && pilotType <= rdList.count()) {
        rdList.at(pilotType-1)->setChecked(true);
        rdList.at(pilotType-1)->setStyleSheet(QString::fromUtf8("background-color: rgb(14,184,63);"));
    }

    /* validation button */
    btn_setHeading->setEnabled(false);
    btn_setAngle->setEnabled(false);

    editHeading->setValue(boat->getHeading());
    editAngle->setValue(computeAngle());

    blocking=false;

    updatePilototBtn();
}

void BoardPilotVLMBoat::updatePilototBtn(void) {
    INIT_BOAT;
    /* Pilototo btn */
    QStringList lst = boat->getPilototo();
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
        btn_Pilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(14,184,63);"));
    else
        btn_Pilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(255,255,127);"));

    btn_Pilototo->setText(pilototo_txt);
    btn_Pilototo->setToolTip(pilototo_toolTip);

    /* clear piltoto btn */
    if(nb!=0)
        btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(14,184,63);"));
    else
        btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(230,230,230);"));
}

void BoardPilotVLMBoat::slot_rdToggle(bool toggle) {
    INIT_BOAT;
    if(blocking || !toggle)
        return;
    int pilotMode;
    bool found=false;
    for(pilotMode=0;pilotMode<rdList.count();pilotMode++) {
        if(rdList.at(pilotMode)->isChecked()) {
            found=true;
            break;
        }
    }
    if(found) {
        switch(pilotMode) {
            case VLM_PILOT_HEADING:
                boat->set_pilotHeading(editHeading->value());
                break;
            case VLM_PILOT_ANGLE:
                boat->set_pilotAngle(editAngle->value());
                break;
            case VLM_PILOT_ORTHO:
                boat->set_pilotOrtho();
                break;
            case VLM_PILOT_VMG:
                boat->set_pilotVmg();
                break;
            case VLM_PILOT_VBVMG:
                boat->set_pilotVbvmg();
                break;
        }
    }
    //qWarning() << "New Pilot mode: " << pilotMode;
}

void BoardPilotVLMBoat::slot_headingValueChg(double heading) {
    INIT_BOAT;
    if(blocking)
        return;
    blocking=true;

    //qWarning() << "Heading chg: " << heading << ", cur=" << boat->get_heading();

    if(heading == boat->getHeading()) /* going back to original heading */ {
        editAngle->setValue(computeAngle());
        btn_setHeading->setEnabled(false);
        btn_setAngle->setEnabled(false);
        emit chg_speed(boat->getSpeed(),VLM_SPEED_MODE);
        emit set_newHeading(-1);
    }
    else {
        double angle=Util::A180(heading-boat->getWindDir());
        btn_setHeading->setEnabled(true);
        btn_setAngle->setEnabled(true);
        /* set new angle */
        editAngle->setValue(angle);
        if(boat->getPolarData()) {
            double newSpeed = boat->getPolarData()->getSpeed(boat->getWindSpeed(),angle);
            emit chg_speed(newSpeed,POLAR_SPEED_MODE);
        }
        else {
            emit chg_speed(boat->getSpeed(),NOPOLAR_SPEED_MODE);
        }
        emit set_newHeading(heading);
    }

    blocking=false;
}

void BoardPilotVLMBoat::slot_angleValueChg(double angle) {
    INIT_BOAT;
    if(blocking) return;
    blocking=true;
    double oldAngle=computeAngle();

    //qWarning() << "Angle chg: " << angle << ", cur=" << oldAngle;

    if(angle==oldAngle) /* going back to original angle */ {
        btn_setHeading->setEnabled(false);
        btn_setAngle->setEnabled(false);
        editHeading->setValue(boat->getHeading());
        emit chg_speed(boat->getSpeed(),VLM_SPEED_MODE);
        emit set_newHeading(-1);
    }
    else {
        btn_setHeading->setEnabled(true);
        btn_setAngle->setEnabled(true);
        double heading = boat->getWindDir() + angle;
        if(heading<0) heading+=360;
        else if(heading>360) heading-=360;
        editHeading->setValue(heading);
        if(boat->getPolarData()) {
            double newSpeed = boat->getPolarData()->getSpeed(boat->getWindSpeed(),angle);
            emit chg_speed(newSpeed,POLAR_SPEED_MODE);
        }
        else {
            emit chg_speed(boat->getSpeed(),NOPOLAR_SPEED_MODE);
        }
        emit set_newHeading(heading);
    }

    blocking=false;
}

void BoardPilotVLMBoat::slot_flipAngle(void) {
    double val = editAngle->value();
    editAngle->setValue(-val);
}

void BoardPilotVLMBoat::slot_setAngle(void) {
    INIT_BOAT;
    if(blocking) return;
    boat->set_pilotAngle(editAngle->value());
}

void BoardPilotVLMBoat::slot_setHeading(void) {
    INIT_BOAT;
    if(blocking) return;
    boat->set_pilotHeading(editHeading->value());
}

void BoardPilotVLMBoat::slot_clearPilototo(void) {
    btn_clearPilototo->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 0, 0);"));
    mainWindow->slot_clearPilototo();
}

/************************************************************/
/*   BoardPosition                                          */
/************************************************************/
BoardPosition::BoardPosition(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock(tr("Position"),BOARD_TYPE_POSITION);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

BoardPosition::~BoardPosition() {
    //disconnect(boardVlm,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

void BoardPosition::slot_updateData(void) {
    INIT_BOAT;
    QPointF position=boat->getPosition();
    longitude->setText(Util::formatLongitude(position.x()));
    latitude->setText(Util::formatLatitude(position.y()));
}

/************************************************************/
/*   BoardWP                                                */
/************************************************************/
BoardWP::BoardWP(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock(tr("WayPoint"),BOARD_TYPE_WP);

    QString str;
    str.sprintf("%c",176);
    deg_unit_1->setText(str);
    deg_unit_2->setText(str);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
    connect(mainWindow,SIGNAL(setChangeStatus(bool,bool,bool)),this,SLOT(slot_setChangeStatus(bool,bool,bool)));

    /* wpDialog */
    dialogWp = new DialogWp();
}

BoardWP::~BoardWP() {
    if(dialogWp) delete dialogWp;
}

void BoardWP::slot_updateData(void) {
    INIT_BOAT;

    /* updating data */

    dnm->setText(QString().sprintf("%.2f",boat->getDnm()));
    ortho->setText(QString().sprintf("%.2f",boat->getOrtho()));
    vmg->setText(QString().sprintf("%.2f",boat->getVmg()));
    angle->setText(QString().sprintf("%.2f",boat->getWPangle()));

    /* updating WP btn */

    QPointF WP = boat->getWP();
    double WPHd = boat->getWPHd();

    if(WP.x()==0 && WP.y()==0) {
        btn_WP->setText(tr("Prochaine balise (0 WP)"));
        btn_WP->setStyleSheet(QString::fromUtf8("background-color: rgb(230,230,230);"));
    }
    else
    {
        QString str = tr("WP: ");
        str = Util::formatPosition(WP.x(),WP.y());

        if(WPHd!=-1)
        {
            str+=" @";
            str+=QString().setNum(WPHd);
            str+=tr("deg");
        }

        btn_WP->setText(str);

        /* bg color*/
        if(boat->getPilotType()==1 || boat->getPilotType()==2)
            btn_WP->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 191, 21);"));
        else
            btn_WP->setStyleSheet(QString::fromUtf8("background-color: rgb(14,184,63);"));
    }
}

void BoardWP::slot_btnWP(void) {
    INIT_BOAT;
    dialogWp->show_WPdialog(boat);
}

void BoardWP::slot_setChangeStatus(bool status,bool,bool) {
    btn_WP->setEnabled(!status);
}

/************************************************************/
/*   BoardSpeedHeading                                      */
/************************************************************/

BoardSpeed::BoardSpeed(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock(tr("Speed"),BOARD_TYPE_SPEED);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
    connect(mainWindow,SIGNAL(setChangeStatus(bool,bool,bool)),this,SLOT(slot_setChangeStatus(bool,bool,bool)));
}

BoardSpeed::~BoardSpeed() {

}

void BoardSpeed::slot_updateData(void) {
    INIT_BOAT;

    slot_chgSpeed(boat->getSpeed(),VLM_SPEED_MODE);
    loch->setText(QString().sprintf("%.2f",boat->getLoch()));
    avg->setText(QString().sprintf("%.2f",boat->getAvg()));
}

void BoardSpeed::slot_chgSpeed(double value,int mode) {
    INIT_BOAT;

    speed->setText(QString().setNum(((double)qRound(value*100))/100));
    switch(mode) {
        case POLAR_SPEED_MODE:
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            speed_label->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            unit_kts_1->setStyleSheet(QString::fromUtf8(SPEED_COLOR_UPDATE));
            break;
        case NOPOLAR_SPEED_MODE:
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            speed_label->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            unit_kts_1->setStyleSheet(QString::fromUtf8(SPEED_COLOR_NO_POLAR));
            break;
        case VLM_SPEED_MODE:
        default:
            speed->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
            speed_label->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
            unit_kts_1->setStyleSheet(QString::fromUtf8(SPEED_COLOR_VLM));
            break;
    }
}

void BoardSpeed::slot_setChangeStatus(bool,bool,bool) {
    slot_updateData();
}

/************************************************************/
/*   BoardWind                                              */
/************************************************************/

BoardWind::BoardWind(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock(tr("Wind"),BOARD_TYPE_WIND);

    QString str;
    str.sprintf("%c",176);
    deg_unit_1->setText(str);
    deg_unit_2->setText(str);
    deg_unit_3->setText(str);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

BoardWind::~BoardWind() {

}

void BoardWind::slot_updateData(void) {
    INIT_BOAT;

    speed->setText(QString().sprintf("%.2f",boat->getWindSpeed()));
    dir->setText(QString().sprintf("%.2f",boat->getWindDir()));
    bvmgD->setText(QString().sprintf("%.1f",boat->getBvmgDown(boat->getWindSpeed())));
    bvmgU->setText(QString().sprintf("%.1f",boat->getBvmgUp(boat->getWindSpeed())));
}

/************************************************************/
/*   BoardWindTool                                              */
/************************************************************/

BoardWindTool::BoardWindTool(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock(tr("Wind tool"),BOARD_TYPE_WINDTOOL);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
    connect(mainWindow,SIGNAL(setChangeStatus(bool,bool,bool)),this,SLOT(slot_setChangeStatus(bool,bool,bool)));
}

BoardWindTool::~BoardWindTool() {

}

void BoardWindTool::slot_updateData(void) {
    INIT_BOAT;

    windAngle->setValues(boat->getHeading(),boat->getWindDir(),boat->getWindSpeed(), boat->getWPdir(), -1);
}

void BoardWindTool::slot_setNewHeading(double heading) {
    INIT_BOAT;

    if(heading<0) heading = -1;
    windAngle->setValues(boat->getHeading(),boat->getWindDir(),boat->getWindSpeed(), boat->getWPdir(), heading);
}

void BoardWindTool::slot_setChangeStatus(bool,bool,bool) {
    slot_updateData();
}

/*********************/
/* VLM20 windAngle  */

tool_windAngle::tool_windAngle(QWidget * parent):QWidget(parent)
{
    /* load image */
    //img_fond = new QImage("img/tool_1zone_2.png");
    //w=img_fond->width();
    //h=img_fond->height();
    //setFixedSize(w,h);

    img_fond=img_compas=img_boat2=img_boat=NULL;

    img_boat = new QImage("img/boat_compas.png");
    img_boat2 = new QImage("img/boat_compas_mvt.png");

    w=200;
    h=200;
    setFixedSize(w,h);

    /*img_boat = new QImage("img/tool_boat.png");
    img_boat2 = new QImage("img/tool_boat_2.png");*/

    heading =windDir=windSpeed=0;
    WPdir = -1;
    newHeading=-1;
}

void tool_windAngle::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    painter.setViewport(0,0,w,h);

    draw(&painter);
}

void tool_windAngle::draw(QPainter * painter)
{
    QFontMetrics fm(painter->font());

    painter->translate(w/2,h/2);

    /* compas */
    int compas_size=194;

    QPen pen(Qt::black);
    pen.setWidth(3);
    painter->setPen(pen);

    painter->drawEllipse(-compas_size/2,-compas_size/2,compas_size,compas_size);

    pen.setWidth(2);
    painter->setPen(pen);

    for(int i=0;i<360;i+=10) {
        painter->drawLine(0,-(compas_size/2),0,-(compas_size/2)+10);
        if(i%30==0) {
            QString txt = QString().setNum(i);
            int h=fm.height();
            int w=fm.width(txt);
            painter->drawText(-w/2,-(compas_size/2)+10+2,w,h,Qt::AlignCenter,txt);
        }
        painter->rotate(10);
    }

    /* restore transform */
    painter->resetTransform();
    painter->translate(w/2,h/2);


    /* rotate + boat */
    if(img_boat && !img_boat->isNull()) {
        painter->rotate(heading);
        painter->drawImage(-img_boat->width()/2,-img_boat->height()/2,*img_boat);
        painter->rotate(-heading);
    }

    /* newHeading boat */
    if(img_boat2 && !img_boat2->isNull() && newHeading!=-1) {
        painter->rotate(newHeading);
        painter->drawImage(-img_boat2->width()/2,-img_boat2->height()/2,*img_boat2);
        painter->rotate(-newHeading);
    }

    /* rotate + wind dir */
    if(windSpeed!=-1)
    {
        painter->rotate(windDir);
        QPen curPen = painter->pen();
        painter->setPen(windSpeed_toColor());
        painter->fillRect(-2,-(compas_size/2),4,40,QBrush(windSpeed_toColor()));
        painter->setPen(curPen);
        painter->rotate(-windDir);
    }

    /* rotate + WP dir */
    if(WPdir != -1)
    {
        painter->rotate(WPdir);
        QPen curPen = painter->pen();
        painter->setPen(QColor(0,0,0));
        painter->fillRect(-2,-(compas_size/2),4,20,QColor(0,0,0));
        painter->setPen(curPen);
        painter->rotate(-WPdir);
    }

#if 0
    if(!img_fond->isNull() && !img_compas->isNull() && !img_boat->isNull())
    {
        int x,y;
        //painter->drawImage(0,0,*img_fond);

        x=23+((w-23)-img_compas->width())/2;
        y=23; /*not centering */
        painter->drawImage(x,y,*img_compas);

        /* center */
        x=45+(203-45)/2;
        y=24+(163-24)/2;
        int h=163-24;
        painter->translate(x, y) ;

        /* rotate + boat */
        painter->rotate(heading);
        painter->drawImage(-img_boat->width()/2,-img_boat->height()/2,*img_boat);
        painter->rotate(-heading);

        /* rotate + new heading boat */
        if(newHeading!=-1 && !img_boat->isNull())
        {
            painter->rotate(newHeading);
            painter->drawImage(-img_boat2->width()/2,-img_boat2->height()/2,*img_boat2);
            painter->rotate(-newHeading);
        }

        /* rotate + wind dir */
        if(windSpeed!=-1)
        {
            painter->rotate(windDir);
            QPen curPen = painter->pen();
            painter->setPen(windSpeed_toColor());
            painter->fillRect(-2,-h/2,4,40,QBrush(windSpeed_toColor()));
            painter->setPen(curPen);
            painter->rotate(-windDir);
        }

        /* rotate + WP dir */
        if(WPdir != -1)
        {
            painter->rotate(WPdir);
            QPen curPen = painter->pen();
            painter->setPen(QColor(0,0,0));
            painter->fillRect(-2,-h/2,4,20,QColor(0,0,0));
            painter->setPen(curPen);
            painter->rotate(-WPdir);
        }

        painter->translate(-x, -y) ;
        /* title */
        /*
        painter->setPen(QColor(220, 200, 140));
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(11);
        painter->setFont(font);
        painter->translate(11, 176) ;
        painter->rotate(-90);
        painter->drawText(QRect(0,0,162,23),Qt::AlignCenter,"Wind-Angle");
        painter->rotate(90);
        painter->translate(-11, -176) ;*/

    }
#endif
}

QColor tool_windAngle::windSpeed_toColor()
{
   //color from VLM code: http://www.virtual-loup-de-mer.org
   // <=F0 : blanc
   if (windSpeed <= 1) return QColor(255, 255, 255);
   // <=F1 : bleu clair legerement gris
   else if (windSpeed <= 3) return QColor(150, 150, 225 );
   // <=F2 : bleu un peu plus soutenu
   else if (windSpeed <= 6) return QColor(80, 140, 205);
   // <=F3 : bleu plus fonc�
   else if (windSpeed <= 10) return QColor(60, 100, 180);
   // <=F4 : vert
   else if (windSpeed <= 15) return QColor(65, 180, 100);
   // <=F5 : jaune l�g�rement vert
   else if (windSpeed <= 21) return QColor(180, 205, 10);
   // <=F6 : jaune orang�
   else if (windSpeed <= 26) return QColor(210, 210, 22);
   // <=F7 : jaune orang� un peu plus rougeatre
   else if (windSpeed <= 33) return QColor(225, 210, 32);
   // <=F8 : orange fonc�
   else if (windSpeed <= 40) return QColor(255, 179, 0);
   // <=F9 : rouge
   else if (windSpeed <= 47) return QColor(255, 111, 0);
   // <=F10 rouge / marron
   else if (windSpeed <= 55) return QColor(255, 43, 0);
   // <=F11 marron
   else if (windSpeed <= 63) return QColor(230, 0, 0);
   // F12  rouge/noir
   else return QColor(127, 0, 0);

}

void tool_windAngle::setValues(double heading,double windDir, double windSpeed, double WPdir,double newHeading)
{
    //qWarning() << "windAngle set: heading=" << heading << " windDir=" << windDir << " windSpeed=" << windSpeed << " WPdir=" << WPdir << " " << newHeading;
    this->heading=heading;
    this->windDir=windDir;
    this->windSpeed=windSpeed;
    this->WPdir=WPdir;
    this->newHeading=newHeading;
    update();
}
