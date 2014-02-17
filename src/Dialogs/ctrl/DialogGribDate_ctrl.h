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

#ifndef DIALOGGRIBDATE_CTRL_H
#define DIALOGGRIBDATE_CTRL_H

#include <time.h>
#include <set>

#include "class_list.h"

class DialogGribDate_ctrl
{
    public:
        DialogGribDate_ctrl(myCentralWidget *centralWidget);

        static time_t choose_gribDate(myCentralWidget * centralWidget,time_t current,std::set<time_t>  * listGrib);

    private:

        DialogGribDate_view * view;
};

#endif // DIALOGGRIBDATE_CTRL_H
