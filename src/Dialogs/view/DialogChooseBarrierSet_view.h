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

#ifndef DIALOGCHOOSEBARRIERSET_VIEW_H
#define DIALOGCHOOSEBARRIERSET_VIEW_H

#include "class_list.h"

class DialogChooseBarrierSet_view
{
    public:
        DialogChooseBarrierSet_view(myCentralWidget *centralWidget, DialogChooseBarrierSet_ctrl *ctrl);

        virtual void init_list(QList<BarrierSet *> lst) =0;
        virtual BarrierSet * launchDialog(void) =0;

    protected:
        DialogChooseBarrierSet_ctrl * ctrl;
        myCentralWidget *centralWidget;

};

#endif // DIALOGCHOOSEBARRIERSET_VIEW_H
