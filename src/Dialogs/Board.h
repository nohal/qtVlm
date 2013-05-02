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

#ifndef BOARD_H
#define BOARD_H

#include <QObject>

#include "dataDef.h"
#include "class_list.h"

#define BOARD_TYPE_VLM           0
#define BOARD_TYPE_PILOT         1
#define BOARD_TYPE_POSITION      2
#define BOARD_TYPE_WIND          3
#define BOARD_TYPE_WINDTOOL      4
#define BOARD_TYPE_SPEED         5
#define BOARD_TYPE_WP            6
#define BOARD_TYPE_REAL          7
#define BOARD_TYPE_POSITION_REAL 8
#define BOARD_TYPE_SPEEDHEADING  9


class Board : public QObject {
    Q_OBJECT
    public:
        Board(MainWindow * mainWindow);
        ~Board(void);

        //void update_data(void);
        void set_boat(boatVLM * boatVlm);

        QList<BoardComponent*> * get_boardList(void);

        void load_settings(void);
        void save_settings(void);

        void set_newType(int type);
        void show_boards(QList<BoardComponent*> boardList);
        void hide_boards(QList<BoardComponent*> boardList);

    signals:
        void sig_setBoat(boatVLM * boatVlm);
        void sig_updateData(void);

    private:
        MainWindow * mainWindow;
        QList<BoardComponent*> boardList_VLM;
        QList<BoardComponent*> boardList_Real;

};


#endif // BOARD_H
