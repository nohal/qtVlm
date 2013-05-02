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

#ifndef BOARDREAL_H
#define BOARDREAL_H

#include "class_list.h"
#include "dataDef.h"
#include "BoardComponent.h"

#include "ui_BoardReal.h"
#include "ui_BoardPosition.h"
#include "ui_BoardSpeedHeading.h"
#include "ui_BoardRealBoatWP.h"

/************************************************************/
/*   BoardRealUi                                             */
/************************************************************/
class BoardRealUi : public BoardComponent, public Ui::BoardRealUi {
    Q_OBJECT
    public:
        BoardRealUi(MainWindow * mainWindow,Board * board);
        ~BoardRealUi(void);

    public slots:
        void slot_updateData(void);
        void slot_showBoatInfo(void);
        void slot_moveBoat(void);

    private:
        DialogRealBoatPosition * dialogRealBoatPosition;
};

/************************************************************/
/*   BoardPosition                                          */
/************************************************************/
class BoardRealPosition : public BoardComponent, public Ui::BoardPosition {
    Q_OBJECT
    public:
        BoardRealPosition(MainWindow * mainWindow,Board * board);
        ~BoardRealPosition(void);

    public slots:
        void slot_updateData(void);
};

/************************************************************/
/*   BoardSpeedHeading                                      */
/************************************************************/
class BoardSpeedHeading : public BoardComponent, public Ui::BoardSpeedHeading {
    Q_OBJECT
    public:
        BoardSpeedHeading(MainWindow * mainWindow,Board * board);
        ~BoardSpeedHeading(void);

    public slots:
        void slot_updateData(void);
};

/************************************************************/
/*   BoardWP                                                */
/************************************************************/
class BoardWP_Real : public BoardComponent, public Ui::BoardRealBoatWP {
    Q_OBJECT
    public:
        BoardWP_Real(MainWindow * mainWindow,Board * board);
        ~BoardWP_Real(void);

    public slots:
        void slot_updateData(void);
        void slot_btnWP(void);

    private:
        DialogWp * dialogWp;
};

#endif // BOARDREAL_H
