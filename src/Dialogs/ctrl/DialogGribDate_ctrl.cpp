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
#include <QDebug>

#include "DialogGribDate_ctrl.h"

#include "DialogGribDate_view_pc.h"

#include "mycentralwidget.h"

DialogGribDate_ctrl::DialogGribDate_ctrl(myCentralWidget * centralWidget) {
    view=new DialogGribDate_view_pc(centralWidget,this);
}

DialogGribDate_ctrl::~DialogGribDate_ctrl(void) {
    delete view;
}

time_t DialogGribDate_ctrl::choose_gribDate(myCentralWidget * centralWidget,time_t current,std::set<time_t>  * listGrib) {
    DialogGribDate_ctrl * ctrl = new DialogGribDate_ctrl(centralWidget);
    ctrl->view->initData(current,listGrib);
    time_t res=ctrl->view->launchDialog();
    delete ctrl;
    return res;
}
