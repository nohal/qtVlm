/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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



#ifndef BOATACCOUNT_DIALOG_H
#define BOATACCOUNT_DIALOG_H

#include <QDialog>

#include "ui_boatAccount_dialog.h"

#include "class_list.h"

class boatSetup
{
    public :
        boatSetup(void);
        boatSetup(boatVLM * boat);
        
        void updateBoat(void);

        bool useAlias;
        QString alias;
        bool usePolar;
        QString polar;
        bool activated;
        bool blocked;
        bool useSkin;
        QString boardSkin;

        boatVLM * boat;
};

class DialogBoatAccount : public QDialog, public Ui::boatAccount_dialog
{
    Q_OBJECT
    public:
        DialogBoatAccount(Projection * proj,MainWindow * main, myCentralWidget * parent,inetConnexion * inet);
        ~DialogBoatAccount();
        void done(int result);
        bool initList(QList<boatVLM*> * boat_list, Player * player);

    public slots:        
        void slot_selectItem_boat(QListWidgetItem *,QListWidgetItem *);
        void slot_selectItem_boatSit(QListWidgetItem *,QListWidgetItem *);

        void chkAlias_changed(int);
        void chkPolar_changed(int);
        void slot_boatUp(void);
        void slot_boatDown(void);
        void slot_boatSitUp(void);
        void slot_boatSitDown(void);
        void slot_enableChanged(bool);
        void slot_browseSkin();

    signals:
        void accountListUpdated(void);
        void writeBoat(void);
        
        void boatPointerHasChanged(boat*);

    private:
        QList<boatVLM*> * boat_list;
        QMap<int,boatSetup*> boats;
        int boat_idx;
        Projection * proj;
        MainWindow * main;
        myCentralWidget * parent;
        inetConnexion * inet;

        void setBoatItemName(QListWidgetItem * item,boatVLM * boat);
        void saveItem(QListWidgetItem * item);
        void setItem(QListWidgetItem * item);
        void boatUp(QListWidget * list);
        void boatDown(QListWidget * list);
        QListWidgetItem *currentItem;
};



#endif
