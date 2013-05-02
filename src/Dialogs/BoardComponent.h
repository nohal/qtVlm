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

#ifndef BOARDCOMPONENT_H
#define BOARDCOMPONENT_H

#include <QDockWidget>
#include <QWidget>

#include "dataDef.h"
#include "class_list.h"

#define INIT_BOAT_T(RET)                     \
    int boatType=mainWindow->get_boatType(); \
    ACTIVE_BOAT_CAST * boat=NULL;                     \
    if(boatType==BOAT_NOBOAT)                \
        return RET;                          \
    boat=(ACTIVE_BOAT_CAST *)mainWindow->getSelectedBoat();            \
    if(!boat || boatType!=ACTIVE_BOAT_TYPE)  \
        return RET;

#define INIT_BOAT                            \
    int boatType=mainWindow->get_boatType(); \
    ACTIVE_BOAT_CAST * boat=NULL;                     \
    if(boatType==BOAT_NOBOAT)                \
        return ;                             \
    boat=(ACTIVE_BOAT_CAST *)mainWindow->getSelectedBoat();            \
    if(!boat || boatType!=ACTIVE_BOAT_TYPE)  \
        return ;

class MyDockWidget : public QDockWidget {
    public:
        MyDockWidget(BoardComponent * boardComponent,QString title);

    protected:
        void closeEvent ( QCloseEvent * event );

    private:
        BoardComponent * boardComponent;
};

class BoardComponent : public QWidget {
    Q_OBJECT
    public:
        BoardComponent(MainWindow * mainWindow);
        ~BoardComponent(void);

        FCT_GET(bool,displayed)
        void set_displayed(bool displayed) { if(!chgingVisibility) this->displayed=displayed; }
        FCT_GET(QString,name)
        FCT_GET(int,typeNum)

        void apply_displayed(bool displayed);
        void set_visible(bool visible);

    public slots:
        virtual void slot_updateData(void)=0;
        void slot_visibilityChanged(bool);

    protected:
        MyDockWidget * dockWidget;
        MainWindow * mainWindow;

        bool displayed;
        QString name;
        int typeNum;

        bool chgingVisibility;

        void init_dock(QString title,int typeNum);
};

#endif // BOARDCOMPONENT_H
