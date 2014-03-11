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

#ifndef DIALOGVLMGRIB_VIEW_PC_H
#define DIALOGVLMGRIB_VIEW_PC_H

#ifdef QT_V5
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QRadioButton>
#else
#include <QMessageBox>
#include <QRadioButton>
#endif

#include "ui_DialogVLMGrib_pc.h"

#include "DialogVlmGrib_view.h"
#include "Dialog_view_pc.h"

#include "class_list.h"

class DialogVlmGrib_view_pc: public Dialog_view_pc, public DialogVlmGrib_view, public Ui::DialogVLMGrib_pc_ui
{ Q_OBJECT
    public:
        DialogVlmGrib_view_pc(myCentralWidget * centralWidget,DialogVlmGrib_ctrl * ctrl);
        ~DialogVlmGrib_view_pc();

        void updateList(QStringList lst);
        void set_waitBoxVisibility(bool visible);
        void set_dialogVisibility(bool visible);

        void closeEvent(QCloseEvent * );

    public slots:
         void slot_download(void);
         void slot_cancel(void);

    private:
        QRadioButton * listRadio[5];
        QMessageBox * waitBox;
};

#endif // DIALOGVLMGRIB_VIEW_PC_H
