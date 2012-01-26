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

#include <QStatusBar>
#include <QDockWidget>
#include <QDebug>

#include "Board.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "boat.h"
#include "BoardReal.h"
#include "BoardVLM.h"
#include "Player.h"
#include "boatVLM.h"

board::board(MainWindow * mainWin, inetConnexion * inet,QStatusBar * statusBar)
{
    playerType = BOAT_NOBOAT;

    vlm_board=new boardVLM(mainWin,inet,this);
    connect(this,SIGNAL(sig_paramChanged()),vlm_board,SLOT(paramChanged()));
    connect(vlm_board,SIGNAL(showMessage(QString,int)),statusBar,SLOT(showMessage(QString,int)));
    connect(this,SIGNAL(hideShowCompass()),vlm_board,SLOT(slot_hideShowCompass()));
    VLMDock = new QDockWidget("Virtual Loup de Mer");
    VLMDock->setWidget(vlm_board);
    VLMDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

    real_board=new boardReal(mainWin,this);
    connect(this,SIGNAL(sig_paramChanged()),real_board,SLOT(paramChanged()));
    connect(real_board,SIGNAL(showMessage(QString,int)),statusBar,SLOT(showMessage(QString,int)));
    connect(mainWin->getMy_centralWidget(),SIGNAL(redrawGrib()),real_board,SLOT(gribUpdated()));
    connect(this,SIGNAL(hideShowCompass()),real_board,SLOT(slot_hideShowCompass()));
    realDock = new QDockWidget(tr("Mon bateau"));
    realDock->setWidget(real_board);
    realDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);


    this->mainWin = mainWin;
    curBoat = NULL;

    connect(mainWin,SIGNAL(setChangeStatus(bool)),this,SLOT(setChangeStatus(bool)));
    connect(mainWin,SIGNAL(boatHasUpdated(boat*)),
            this,SLOT(boatUpdated(boat*)));

    isFloatingBoard=false;
}

void board::floatingBoard(bool status)
{
    isFloatingBoard=status;
    realDock->setFloating(status);
    VLMDock->setFloating(status);
}

int board::currentBoardType(void)
{
    return playerType;
}

void board::paramChanged(void)
{
    emit sig_paramChanged();
}

void board::slot_hideShowCompass(void)
{
    emit hideShowCompass();
}

void board::boatUpdated(boat* myBoat)
{
    if(curBoat!=NULL && curBoat->getType()==BOAT_REAL && myBoat!=curBoat)
        curBoat->setSelected(false);
    curBoat=myBoat;
    if(playerType!=BOAT_NOBOAT)
    {
        if(playerType == BOAT_VLM)
        {
            vlm_board->boatUpdated();
        }
        else
            real_board->boatUpdated();
    }
}


void board::playerChanged(Player * player)
{
    if(!player)
        return;

    if(playerType!=BOAT_NOBOAT)
    {
        if(playerType == BOAT_VLM)
        {
            mainWin->removeDockWidget(VLMDock);
        }
        else
        {
            mainWin->removeDockWidget(realDock);
        }
    }

    playerType=player->getType();
    qWarning()<<"playerType="<<playerType;
    if(playerType!=BOAT_NOBOAT)
    {
        if(playerType == BOAT_VLM)
        {
            mainWin->addDockWidget(Qt::LeftDockWidgetArea,VLMDock);
            floatingBoard(isFloatingBoard);
            VLMDock->show();
            vlm_board->show();
        }
        else
        {
            mainWin->addDockWidget(Qt::LeftDockWidgetArea,realDock);
            floatingBoard(isFloatingBoard);
            realDock->show();
            real_board->show();
        }
    }
}

void board::setChangeStatus(bool val)
{
    if(playerType!=BOAT_NOBOAT)
    {
        if(playerType == BOAT_VLM)
        {
            vlm_board->setChangeStatus(val);
        }
        else
        {
            real_board->setChangeStatus(val);
        }
    }
}

void board::outdatedVLM()
{

    if (vlm_board)
        if (!(vlm_board->btn_Synch->styleSheet()).contains(QColor(255, 0, 0).name())) //if red stays red
            vlm_board->set_style(vlm_board->btn_Synch,QColor(255, 191, 21));
}
