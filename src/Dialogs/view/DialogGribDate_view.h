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

#ifndef DIALOGGRIBDATE_VIEW_H
#define DIALOGGRIBDATE_VIEW_H

#include <set>

#include "class_list.h"
#include "dataDef.h"

class DialogGribDate_view
{
    public:
        DialogGribDate_view(myCentralWidget *centralWidget, DialogGribDate_ctrl *ctrl);


        virtual void initData(time_t current,std::set<time_t>  * listGrib) =0;
        virtual time_t launchDialog(void) =0;


    protected:
        DialogGribDate_ctrl * ctrl;
        myCentralWidget *centralWidget;
        time_t initialTime;
};

#endif // DIALOGGRIBDATE_VIEW_H
