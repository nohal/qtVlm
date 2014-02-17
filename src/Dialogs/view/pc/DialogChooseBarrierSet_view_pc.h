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

#ifndef DIALOGCHOOSEBARRIERSET_VIEW_PC_H
#define DIALOGCHOOSEBARRIERSET_VIEW_PC_H

#include "Dialog_view_pc.h"
#include "DialogChooseBarrierSet_view.h"

#include "ui_DialogChooseBarrierSet_pc.h"

#include "class_list.h"

class DialogChooseBarrierSet_view_pc: public Dialog_view_pc, public DialogChooseBarrierSet_view, public Ui::DialogChooseBarrierSet_pc_ui
{   Q_OBJECT
    public:
        DialogChooseBarrierSet_view_pc(myCentralWidget * centralWidget,DialogChooseBarrierSet_ctrl * ctrl);

        void init_list(QList<BarrierSet *> lst);
        BarrierSet * launchDialog(void);
};

#endif // DIALOGCHOOSEBARRIERSET_VIEW_PC_H
