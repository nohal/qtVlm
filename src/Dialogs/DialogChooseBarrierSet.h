/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

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


#ifndef DIALOGCHOOSEBARRIER_H
#define DIALOGCHOOSEBARRIER_H

#include "QDialog"

#include "class_list.h"
#include "dataDef.h"

#include "ui_DialogChooseBarrierSet.h"

class DialogChooseBarrierSet: public QDialog, public Ui::DialogChooseBarrierSet_ui {
    public:
        DialogChooseBarrierSet(MainWindow *parent);
        ~DialogChooseBarrierSet();

        static BarrierSet * chooseBarrierSet(MainWindow *parent);

        FCT_GET(BarrierSet*,choice)

        void done(int result);
        int init_dialog(void);

    private:
        MainWindow * mainWindow;
        BarrierSet * choice;
};

#endif // DIALOGCHOOSEBARRIER_H
