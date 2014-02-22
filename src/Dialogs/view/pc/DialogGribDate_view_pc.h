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

#ifndef DIALOGGRIBDATE_VIEW_PC_H
#define DIALOGGRIBDATE_VIEW_PC_H

#include <set>

#include "Dialog_view_pc.h"
#include "DialogGribDate_view.h"
#include "ui_DialogGribDate_pc.h"
#include "class_list.h"

class DialogGribDate_view_pc: public Dialog_view_pc, public DialogGribDate_view, public Ui::DialogGribDate_pc_ui
{ Q_OBJECT
    public:
        DialogGribDate_view_pc(myCentralWidget * centralWidget,DialogGribDate_ctrl * ctrl);
        ~DialogGribDate_view_pc(void);

        void initData(time_t current,std::set<time_t>  * listGrib);
        time_t launchDialog(void);

    public slots:
        void slot_listChanged(int index);
        void slot_timeChanged(QDateTime);
};

#endif // DIALOGGRIBDATE_VIEW_PC_H
