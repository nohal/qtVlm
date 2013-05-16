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

#ifndef BOARDVLM_H
#define BOARDVLM_H

#include <QDoubleSpinBox>

#include "class_list.h"
#include "dataDef.h"

#include "BoardComponent.h"

#include "ui_BoardPilotVLMBoat.h"
#include "ui_BoardPosition.h"
#include "ui_BoardWP.h"
#include "ui_BoardSpeed.h"
#include "ui_BoardWind.h"
#include "ui_BoardWindTool.h"
#include "ui_BoardVlm.h"

/************************************************************/
/*   BoardVlmUi                                             */
/************************************************************/
class BoardVlmUi : public BoardComponent, public Ui::BoardVlmUi {
    Q_OBJECT
    public:
        BoardVlmUi(MainWindow * mainWindow,Board * board);
        ~BoardVlmUi(void);

    public slots:
        void slot_updateData(void);
        void slot_setCurrentBoat(boat * myBoat);
        void slot_boatSelected(int);
        void slot_initBoatList(Player*);
        void slot_showBoatInfo(void);
        void slot_vlmSynch(void);
        void slot_toggleGps(bool);
        void slot_updateLockIcon(QIcon ic);
        void slot_setChangeStatus(bool status,bool pilototo,bool btnSync);
        void slot_outDatedVlmData(void);

    signals:
        void VLM_Sync(void);
        void selectBoat(boat * newBoat);
};

/************************************************************/
/*   BoardPilotVLMBoat                                      */
/************************************************************/
class BoardPilotVLMBoat : public BoardComponent, public Ui::BoardPilotVLMBoat {
    Q_OBJECT
    public:
        BoardPilotVLMBoat(MainWindow * mainWindow,Board * board);
        ~BoardPilotVLMBoat(void);

    public slots:
        void slot_updateData(void);
        void slot_rdToggle(bool);
        void slot_headingValueChg(double);
        void slot_angleValueChg(double);
        void slot_flipAngle(void);
        void slot_setAngle(void);
        void slot_setHeading(void);
        void slot_hasEvent(void);
        void slot_clearPilototo(void);
        void slot_selectPOI(bool);
        void slot_setChangeStatus(bool status,bool pilototo,bool btnSync);

    signals:
        void chg_speed(double value,int mode);
        void set_newHeading(double heading);


    private:
        bool blocking;
        QList<QRadioButton*> rdList;
        QString rdStyleSheet;

        void updatePilototBtn(void);

        double computeAngle(void);        
};



/************************************************************/
/*   BoardPosition                                          */
/************************************************************/
class BoardPosition : public BoardComponent, public Ui::BoardPosition {
    Q_OBJECT
    public:
        BoardPosition(MainWindow * mainWindow,Board * board);
        ~BoardPosition(void);

    public slots:
        void slot_updateData(void);
};

/************************************************************/
/*   BoardWP                                                */
/************************************************************/
class BoardWP : public BoardComponent, public Ui::BoardWP {
    Q_OBJECT
    public:
        BoardWP(MainWindow * mainWindow,Board * board);
        ~BoardWP(void);

    public slots:
        void slot_updateData(void);
        void slot_btnWP(void);
        void slot_setChangeStatus(bool status,bool pilototo,bool btnSync);

    private:
        DialogWp * dialogWp;
};

/************************************************************/
/*   BoardSpeedHeading                                      */
/************************************************************/
class BoardSpeed : public BoardComponent, public Ui::BoardSpeed {
    Q_OBJECT
    public:
        BoardSpeed(MainWindow * mainWindow,Board * board);
        ~BoardSpeed(void);

    public slots:
        void slot_updateData(void);
        void slot_chgSpeed(double value,int mode);
        void slot_setChangeStatus(bool status,bool pilototo,bool btnSync);
};

/************************************************************/
/*   BoardWind                                              */
/************************************************************/
class BoardWind : public BoardComponent, public Ui::BoardWind {
    Q_OBJECT
    public:
        BoardWind(MainWindow * mainWindow,Board * board);
        ~BoardWind(void);

    public slots:
        void slot_updateData(void);
};

/************************************************************/
/*   BoardWindTool                                              */
/************************************************************/
class BoardWindTool : public BoardComponent, public Ui::BoardWindTool {
    Q_OBJECT
    public:
        BoardWindTool(MainWindow * mainWindow,Board * board);
        void setBoardPilot(BoardPilotVLMBoat * bo){this->boardPilotVLMBoat=bo;}
        ~BoardWindTool(void);

public slots:
        void buttonClicked(int pilotMode);
        void slot_updateData(void);
        void slot_setNewHeading(double heading);
        void slot_setChangeStatus(bool status,bool pilototo,bool btnSync);
private:
        BoardPilotVLMBoat * boardPilotVLMBoat;
};

#endif // BOARDVLM_H
