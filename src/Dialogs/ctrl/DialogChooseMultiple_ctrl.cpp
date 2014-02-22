/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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

#include "mycentralwidget.h"
#include "BarrierSet.h"
#include "boat.h"

#include "DialogChooseMultiple_view_pc.h"

#include "DialogChooseMultiple_ctrl.h"

void DialogChooseMultiple_ctrl::chooseMultipleBoat(myCentralWidget *centralWidget,BarrierSet * barrierSet, QList<boat*> boatList) {
    // Create dialog
    DialogChooseMultiple_view * view=new DialogChooseMultiple_view_pc(centralWidget);

    // init data
    QList<dataItem> list;
    for(int i=0;i<boatList.count();++i) {
        boat * boatPtr = boatList.at(i);
        if(boatPtr->getStatus()) {
            dataItem it;
            it.str=boatPtr->getBoatPseudo();
            it.dataVariant=VPtr<boat>::asQVariant(boatPtr);
            it.checked=boatPtr->get_barrierSets()->contains(barrierSet);
            list.append(it);
        }
    }

    // call dialog
    list=view->launchDialog(tr("Choose boats"),tr("Available boat"),list);
    if(list.count()!=0) {
        for(int i=0;i<list.count();++i) {
            boat * boatPtr = VPtr<boat>::asPtr(list.at(i).dataVariant);
            if(list.at(i).checked)
                boatPtr->add_barrierSet(barrierSet);
            else
                boatPtr->rm_barrierSet(barrierSet);
        }
    }

    delete view;
}

void DialogChooseMultiple_ctrl::chooseMultipleBarrierSet(myCentralWidget *centralWidget,QList<BarrierSet *> *activeSets, boat *myBoat) {
    // Create dialog
    DialogChooseMultiple_view * view=new DialogChooseMultiple_view_pc(centralWidget);

    // init data
    QList<dataItem> list;
    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * barrierSet=::barrierSetList.at(i);
        dataItem item;
        item.str=barrierSet->get_name();
        item.dataVariant=VPtr<BarrierSet>::asQVariant(barrierSet);
        item.checked=activeSets->contains(barrierSet)?Qt::Checked:Qt::Unchecked;
        list.append(item);
    }
    // call dialog
    list=view->launchDialog(tr("Choose barrier set"),tr("Available set"),list);
    if(list.count()!=0) {
        for(int i=0;i<list.count();++i) {
            BarrierSet * barrierSet=VPtr<BarrierSet>::asPtr(list.at(i).dataVariant);
            if(list.at(i).checked) {
                activeSets->append(barrierSet);
                if(myBoat->getIsSelected())
                    barrierSet->set_isHidden(false);
            }
            else {
                if(myBoat->getIsSelected())
                    barrierSet->set_isHidden(true);
            }
        }
    }

    delete view;
}
