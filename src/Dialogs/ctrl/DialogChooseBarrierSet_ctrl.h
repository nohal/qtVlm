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

#ifndef DIALOGCHOOSEBARRIERSET_CTRL_H
#define DIALOGCHOOSEBARRIERSET_CTRL_H

#include <QList>

#include "class_list.h"

class DialogChooseBarrierSet_ctrl
{
    public:
        DialogChooseBarrierSet_ctrl(myCentralWidget *centralWidget,QList<BarrierSet *> lst);

        static BarrierSet * chooseBarrierSet(myCentralWidget *centralWidget);

    private:
        DialogChooseBarrierSet_view * view;
};

#endif // DIALOGCHOOSEBARRIERSET_CTRL_H
