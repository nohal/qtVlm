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
#include "settings.h"
#include "BarrierSet.h"
#include "DialogChooseBarrierSet_ctrl.h"

#include "DialogChooseBarrierSet_view_pc.h"

DialogChooseBarrierSet_view_pc::DialogChooseBarrierSet_view_pc(myCentralWidget * centralWidget,DialogChooseBarrierSet_ctrl * ctrl) :
    Dialog_view_pc(centralWidget),
    DialogChooseBarrierSet_view(centralWidget,ctrl)
{
    INIT_DIALOG
}

void DialogChooseBarrierSet_view_pc::init_list(QList<BarrierSet *> lst) {
    for(int i=0;i<lst.count();++i) {
        BarrierSet * set = lst.at(i);
        QVariant data = VPtr<BarrierSet>::asQVariant(set);
        cb_barrierSets->addItem(set->get_name(),data);
    }
}

BarrierSet * DialogChooseBarrierSet_view_pc::launchDialog(void) {
    Settings::saveGeometry(this);
    if(exec()==QDialog::Accepted) {
        Settings::saveGeometry(this);
        return VPtr<BarrierSet>::asPtr(cb_barrierSets->itemData(cb_barrierSets->currentIndex()));
    }
    else {
        Settings::saveGeometry(this);
        return NULL;
    }
}
