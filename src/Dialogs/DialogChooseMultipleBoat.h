/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGCHOOSEMULTIPLEBOAT_H
#define DIALOGCHOOSEMULTIPLEBOAT_H

#include "QDialog"

#include "class_list.h"
#include "dataDef.h"

#include "ui_DialogChooseMultipleBoat.h"

class DialogChooseMultipleBoat: public QDialog, Ui::DialogChooseMultipleBoat_ui
{
    public:
        DialogChooseMultipleBoat(QWidget *parent);
        void done(int result);
        void init_dialog(BarrierSet * barrierSet, QList<boat *> boatList);

        static void chooseBoat(QWidget *parent,BarrierSet * barrierSet,QList<boat*> boatList);

    private:
        BarrierSet *barrierSet;
};

#endif // DIALOGCHOOSEMULTIPLEBOAT_H
