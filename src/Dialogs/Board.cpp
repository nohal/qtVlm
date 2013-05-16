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
#include <QDebug>

#include "Board.h"
#include "MainWindow.h"
#include "settings.h"
#include "BoardVlm.h"
#include "BoardReal.h"

Board::Board(MainWindow * mainWindow)
{
    this->mainWindow=mainWindow;

    boardList_VLM.clear();
    boardList_Real.clear();
    /* init all VLM boards */
    boardList_VLM.append(new BoardVlmUi(mainWindow,this));
    BoardPilotVLMBoat * boardPilotVLMBoat = new BoardPilotVLMBoat(mainWindow,this);
    boardList_VLM.append(boardPilotVLMBoat);
    boardList_VLM.append(new BoardPosition(mainWindow,this));
    boardList_VLM.append(new BoardWP(mainWindow,this));
    BoardSpeed * boardSpeed = new BoardSpeed(mainWindow,this);
    boardList_VLM.append(boardSpeed);
    boardList_VLM.append(new BoardWind(mainWindow,this));
    BoardWindTool * boardWindTool = new BoardWindTool(mainWindow,this);
    boardList_VLM.append(boardWindTool);

    /* init all Real boards */
    boardList_Real.append(new BoardRealUi(mainWindow,this));
    boardList_Real.append(new BoardRealPosition(mainWindow,this));
    boardList_Real.append(new BoardSpeedHeading(mainWindow,this));
    boardList_Real.append(new BoardWP_Real(mainWindow,this));


    set_newType(mainWindow->get_boatType());

    /* inter board signals */
    connect(boardPilotVLMBoat,SIGNAL(chg_speed(double,int)),boardSpeed,SLOT(slot_chgSpeed(double,int)));
    connect(boardPilotVLMBoat,SIGNAL(set_newHeading(double)),boardWindTool,SLOT(slot_setNewHeading(double)));
    boardWindTool->setBoardPilot(boardPilotVLMBoat);
    connect(mainWindow,SIGNAL(boatHasUpdated(boat*)),this,SIGNAL(sig_updateData()));
    connect(mainWindow,SIGNAL(boatSelected(boat*)),this,SLOT(slot_setCurrentBoat(boat*)));
}

Board::~Board(void) {

}

void Board::slot_setCurrentBoat(boat* upBoat) {
    emit sig_setCurrentBoat(upBoat);
    emit sig_updateData();
}

/*
void Board::update_data(void) {
    emit sig_updateData();
}
*/
void Board::load_settings(void) {
    QList<BoardComponent*> boardList;
    for(int i=0;i<2;i++) {
        boardList=i==0?boardList_VLM:boardList_Real;

        QListIterator<BoardComponent*> it (boardList);
        while(it.hasNext()) {
            BoardComponent* boardComponent=it.next();
            QString key="BOARD_"+QString().setNum(i)+"_"+QString().setNum(boardComponent->get_typeNum());
            boardComponent->set_displayed(Settings::getSetting(key,"1","Board").toBool());
        }
    }
}

void Board::save_settings(void) {
    QList<BoardComponent*> boardList;
    for(int i=0;i<2;i++) {
        boardList=i==0?boardList_VLM:boardList_Real;
        QListIterator<BoardComponent*> it (boardList);
        while(it.hasNext()) {
            BoardComponent* boardComponent=it.next();
            QString key="BOARD_"+QString().setNum(i)+"_"+QString().setNum(boardComponent->get_typeNum());
            Settings::setSetting(key,boardComponent->get_displayed(),"Board");
        }
    }
}

QList<BoardComponent*> * Board::get_boardList(void) {
    int boatType=mainWindow->get_boatType();
    switch(boatType) {
        case BOAT_VLM:
            return &boardList_VLM;
        case BOAT_REAL:
            return &boardList_Real;
        case BOAT_NOBOAT:
        default:
            return NULL;
    }
}

int Board::build_showHideMenu(QMenu *menu) {
    int boatType=mainWindow->get_boatType();

    QList<BoardComponent*> * curBoard;
    switch(boatType) {
        case BOAT_VLM:
            curBoard=&boardList_VLM;
            break;
        case BOAT_REAL:
            curBoard=&boardList_Real;
            break;
        case BOAT_NOBOAT:
        default:
            curBoard=NULL;
            break;
    }

    if(curBoard) {
        int i;
        for(i=0;i<curBoard->count();++i)
            menu->addAction(curBoard->at(i)->get_toggleViewAction());
        return i;
    }

    else
        return 0;
}

void Board::set_newType(int type) {
    if(type==BOAT_NOBOAT) {
        hide_boards(boardList_Real);
        hide_boards(boardList_VLM);
    }

    if(type==BOAT_VLM) {
        hide_boards(boardList_Real);
        show_boards(boardList_VLM);
    }

    if(type==BOAT_REAL) {
        hide_boards(boardList_VLM);
        show_boards(boardList_Real);
    }

    if(type!=BOAT_NOBOAT)
        //update_data();
        emit sig_updateData();
}

void Board::show_boards(QList<BoardComponent*> boardList) {
    QListIterator<BoardComponent*> i (boardList);
    while(i.hasNext()) {
        i.next()->set_visible(true);
    }
}

void Board::hide_boards(QList<BoardComponent*> boardList) {
    QListIterator<BoardComponent*> i (boardList);
    while(i.hasNext()) {
        i.next()->set_visible(false);
    }
}

/************************************************************/
/*   tool_edtSpinBox                                        */
/************************************************************/
tool_edtSpinBox::tool_edtSpinBox(QWidget * parent): QDoubleSpinBox(parent) {
    this->parent=parent;
}

void tool_edtSpinBox::keyPressEvent ( QKeyEvent * event ) {
    QKeyEvent *ke = static_cast<QKeyEvent *>(event);
    if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
    {
        emit hasEvent();
        return;
    }
    QDoubleSpinBox::keyPressEvent(event);
}

void tool_edtSpinBox::keyReleaseEvent ( QKeyEvent * event ) {
    QDoubleSpinBox::keyReleaseEvent (event);
}
