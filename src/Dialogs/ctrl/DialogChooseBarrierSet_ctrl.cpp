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

#include <QMessageBox>

#include "mycentralwidget.h"
#include "BarrierSet.h"
#include "boat.h"

#include "DialogChooseBarrierSet_ctrl.h"

#include "DialogChooseBarrierSet_view_pc.h"

DialogChooseBarrierSet_ctrl::DialogChooseBarrierSet_ctrl(myCentralWidget *centralWidget,QList<BarrierSet *> lst) {
    view=new DialogChooseBarrierSet_view_pc(centralWidget,this);
    view->init_list(lst);
}

BarrierSet * DialogChooseBarrierSet_ctrl::chooseBarrierSet(myCentralWidget *centralWidget) {
    QList<BarrierSet *> lst;
    /* no set => return NULL */
    if(::barrierSetList.isEmpty()) {
        QMessageBox::warning(centralWidget,QObject::tr("Barrier set choice"),QObject::tr("No barrier set define, create one before doing this action"));
        return NULL;
    }

    boat * curBoat = centralWidget->getSelectedBoat();

    /* only one set => no dialog */
    if(::barrierSetList.count() == 1 && curBoat->has_barrierSet(::barrierSetList.first())) return ::barrierSetList.first();

    qSort(::barrierSetList.begin(),::barrierSetList.end(),BarrierSet::myLessThan);

    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * set = ::barrierSetList.at(i);
        if(!set->get_isHidden() && curBoat->has_barrierSet(set) )
            lst.append(set);
    }

    if(lst.size()==0) {
        QMessageBox::warning(centralWidget,QObject::tr("Barrier set choice"),QObject::tr("No barrier associate to current boat"));
        return NULL;
    }

    if(lst.size()==1) return lst.at(0);

    DialogChooseBarrierSet_ctrl dialogChooseBarrierSet(centralWidget,lst);

    return dialogChooseBarrierSet.view->launchDialog();

}
