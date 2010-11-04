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

board::board(MainWindow * mainWin, inetConnexion * inet,QStatusBar * statusBar)
{

    vlm_board=new boardVLM(mainWin,inet,this);
    connect(this,SIGNAL(sig_paramChanged()),vlm_board,SLOT(paramChanged()));
    connect(vlm_board,SIGNAL(showMessage(QString,int)),statusBar,SLOT(showMessage(QString,int)));
    connect(this,SIGNAL(hideShowCompass()),vlm_board,SLOT(slot_hideShowCompass()));
    VLMDock = new QDockWidget("Virtual Loup de Mer");
    VLMDock->setWidget(vlm_board);
    VLMDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
#if 1
    mainWin->addDockWidget(Qt::LeftDockWidgetArea,VLMDock);
    mainWin->setBoardToggleAction(VLMDock->toggleViewAction());
    vlm_board->show();
#else
    vlm_board->hide();
#endif

/*
    real_board=new boardReal(mainWin,this);
    connect(this,SIGNAL(sig_paramChanged()),real_board,SLOT(paramChanged()));
    connect(real_board,SIGNAL(showMessage(QString,int)),statusBar,SLOT(showMessage(QString,int)));
    connect(this,SIGNAL(hideShowCompass()),real_board,SLOT(slot_hideShowCompass()));
    realDock = new QDockWidget(tr("Mon bateau"));
    realDock->setWidget(real_board);
    realDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
#if 1
    mainWin->addDockWidget(Qt::LeftDockWidgetArea,realDock);
    real_board->show();
#else
    real_board->hide();
#endif
*/
    this->mainWin = mainWin;
    curBoat = NULL;

    connect(mainWin,SIGNAL(setChangeStatus(bool)),this,SLOT(setChangeStatus(bool)));
    connect(mainWin,SIGNAL(boatHasUpdated(boat*)),
            this,SLOT(boatUpdated(boat*)));




}

int board::currentBoardType(void)
{
    if(curBoat)
        return curBoat->getType();
    else
        return BOAT_NOBOAT;
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
#if 0
    /*  mainWin->setBoardToggleAction(VLMDock->toggleViewAction());*/

    if(!myBoat)
    {
        /* hiding all */
        qWarning() << "board::boatUpdated : new boat = NULL";
        mainWin->removeDockWidget(VLMDock);
        mainWin->removeDockWidget(realDock);
    }
    else
    {
        qWarning() << "board::boatUpdated : new boat exist";
        if(myBoat!=curBoat)
        {
            qWarning() << "board::boatUpdated : new boat different from previsous";
            /* make sure the correct board is shown */
            if(myBoat->getType()==BOAT_VLM)
            {
                qWarning() << "board::boatUpdated : VLM boat";
                mainWin->removeDockWidget(realDock);
                mainWin->addDockWidget(Qt::LeftDockWidgetArea,VLMDock);
                vlm_board->show();
            }
            else
            {
                qWarning() << "board::boatUpdated : real boat";
                mainWin->removeDockWidget(VLMDock);
                mainWin->addDockWidget(Qt::LeftDockWidgetArea,realDock);
                realDock->setEnabled(true);
                real_board->show();
                real_board->update();
            }
        }

    }
    qWarning() << "board::boatUpdated : calling VLM and real board update";
    curBoat=myBoat;
    vlm_board->boatUpdated();
    real_board->boatUpdated();
#else
    curBoat=myBoat;
    vlm_board->boatUpdated();
#endif
}

void board::setChangeStatus(bool val)
{
    if(curBoat)
    {
        if(curBoat->getType()==BOAT_VLM)
            vlm_board->setChangeStatus(val);
        else
            real_board->setChangeStatus(val);
    }
}
