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

#ifndef DIALOGCHOOSEMULTIPLE_VIEW_PC_H
#define DIALOGCHOOSEMULTIPLE_VIEW_PC_H

#include "Dialog_view_pc.h"
#include "DialogChooseMultiple_view.h"
#include "ui_DialogChooseMultiple_pc.h"
#include "class_list.h"

class DialogChooseMultiple_view_pc: public Dialog_view_pc, public DialogChooseMultiple_view, public Ui::DialogChooseMultiple_pc_ui
{ Q_OBJECT
    public:
        DialogChooseMultiple_view_pc(myCentralWidget * centralWidget);

        ~DialogChooseMultiple_view_pc(void) { }

        QList<dataItem> launchDialog(QString dialogTitle, QString title, QList<dataItem> dataItemList);
};

#endif // DIALOGCHOOSEMULTIPLE_VIEW_PC_H
