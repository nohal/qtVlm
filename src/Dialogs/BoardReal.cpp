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

#include <QMessageBox>

#include "class_list.h"
#include "dataDef.h"

#define ACTIVE_BOAT_TYPE BOAT_REAL
#define ACTIVE_BOAT_CAST boatReal

#include "BoardReal.h"
#include "Board.h"
#include "BoardComponent.h"
#include "MainWindow.h"
#include "boatReal.h"
#include "DialogRealBoatPosition.h"
#include "DialogWp.h"

/************************************************************/
/*   BoardRealUi                                            */
/************************************************************/

BoardRealUi::BoardRealUi(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);

    init_dock("BoatReal",tr("Boat"),BOARD_TYPE_REAL);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));

    dialogRealBoatPosition = new DialogRealBoatPosition(mainWindow);
}

BoardRealUi::~BoardRealUi() {

}

void BoardRealUi::slot_updateData(void) {
    INIT_BOAT;
    boatName->setText(boat->getBoatPseudo());
}

void BoardRealUi::slot_showBoatInfo(void) {
    INIT_BOAT;

    QString polarName="<"+tr("No Polar")+">";

    if(boat->getPolarData())
        polarName=boat->getPolarName();

    QMessageBox::information(this,tr("Information"),
                             tr("Boat: ")+boat->getBoatPseudo() + "\n"
                             + tr("Polar: ") + polarName
                             //+ "\n"
                             //+ tr("Movable: " ) + (boat->get_manualPosition()?tr("yes"):tr("no"))
                             );
}

void BoardRealUi::slot_moveBoat(void) {
    INIT_BOAT;
    if(dialogRealBoatPosition)
        dialogRealBoatPosition->showDialog(boat);
}

/************************************************************/
/*   BoardPosition                                          */
/************************************************************/
BoardRealPosition::BoardRealPosition(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock("PositionReal",tr("Position"),BOARD_TYPE_POSITION_REAL);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

BoardRealPosition::~BoardRealPosition() {
    //disconnect(boardVlm,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

void BoardRealPosition::slot_updateData(void) {
    INIT_BOAT;

    QPointF position=boat->getPosition();
    longitude->setText(Util::pos2String(TYPE_LON,position.x()));
    latitude->setText(Util::pos2String(TYPE_LAT,position.y()));
}

/************************************************************/
/*   BoardSpeedHeading                                      */
/************************************************************/
BoardSpeedHeading::BoardSpeedHeading(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock("SpdReal",tr("Speed & heading"),BOARD_TYPE_SPEEDHEADING);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

BoardSpeedHeading::~BoardSpeedHeading() {
    //disconnect(boardVlm,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));
}

void BoardSpeedHeading::slot_updateData(void) {
    INIT_BOAT;

    speed->display(boat->getSpeed());
    heading->display(boat->getHeading());
}

/************************************************************/
/*   BoardWP                                                */
/************************************************************/
BoardWP_Real::BoardWP_Real(MainWindow * mainWindow,Board * board): BoardComponent(mainWindow) {
    setupUi(this);
    init_dock("WP_real",tr("WayPoint"),BOARD_TYPE_WP);

    QString str;
    str.sprintf("%c",176);
    deg_unit_1->setText(str);
    deg_unit_2->setText(str);
    deg_unit_3->setText(str);

    connect(board,SIGNAL(sig_updateData()),this,SLOT(slot_updateData()));

    /* wpDialog */
    dialogWp = new DialogWp();
}

BoardWP_Real::~BoardWP_Real() {
    if(dialogWp) delete dialogWp;
}

void BoardWP_Real::slot_updateData(void) {
    INIT_BOAT;

    /* updating data */
    dnm->setText(QString().setNum(boat->getDnm()));
    ortho->setText(QString().setNum(boat->getOrtho()));
    vmg->setText(QString().setNum(boat->getVmg()));
    loxo->setText(QString().setNum(boat->getLoxo()));
    //angle->setText(QString().setNum(boat->get_WPangle()));

    /* updating WP btn */

    QPointF WP = boat->getWP();
    double WPHd = boat->getWPHd();


    if(WP.x()==0 && WP.y()==0)
        btn_WP->setText(tr("Prochaine balise (0 WP)"));
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
    }
}

void BoardWP_Real::slot_btnWP(void) {
    INIT_BOAT;
    dialogWp->show_WPdialog(boat);
}
