/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

#ifndef PILOTOTO_PARAM_H
#define PILOTOTO_PARAM_H

#include "class_list.h"

#include "ui_Pilototo_param.h"

class DialogPilototoInstruction;

//===================================================================
class DialogPilototoParam : public QDialog, public Ui::pilototo_param_ui
{Q_OBJECT
    public:
        DialogPilototoParam(QWidget *parent);
        
        void done(int);

    public slots:
        void editInstruction(DialogPilototoInstruction *);
        void editInstructionPOI(DialogPilototoInstruction *,POI*);

    private slots:
        void modeChanged(int);
        void pastePOI(void);
        void selectPOI(void);
        void copyPOI(void);

    signals:
        void doSelectPOI(DialogPilototoInstruction *,int type);

    private:
        DialogPilototoInstruction * instruction;

        double getValue(int type);
        void setValue(int type,double val);
};

#endif
