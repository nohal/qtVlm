/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#ifndef BOARD_H
#define BOARD_H

#include <QMainWindow>

#include "class_list.h"
#include "dataDef.h"

class board : public QWidget
{ Q_OBJECT
    public:

        board(MainWindow * mainWin, inetConnexion * inet);

        boat * currentBoat(void) { return curBoat; }

        boardVLM * VLMBoard(void) {return vlm_board; }
        boardReal * realBoard(void) {return real_board; }

        void playerChanged(Player * player);

        int currentBoardType(void);
        void floatingBoard(bool status);
        void outdatedVLM(void);
        void showCurrentBoard(const bool &b);

        bool currentBoardIsVisibe();
public slots:
        void boatUpdated(boat * boat);
        void setChangeStatus(bool);
        void paramChanged(void);
        void slot_hideShowCompass();

    signals:
        void sig_paramChanged();
        void hideShowCompass();

    private:
        MainWindow * mainWin;

        boardVLM * vlm_board;
        QDockWidget * VLMDock;
        boardReal * real_board;
        QDockWidget * realDock;

        boat * curBoat;
        int playerType;
        bool isFloatingBoard;
};

#endif // BOARD_H
