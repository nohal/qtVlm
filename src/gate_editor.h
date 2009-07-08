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

#ifndef GATE_EDITOR_H
#define GATE_EDITOR_H

#include <QDialog>

#include "gate.h"
#include "ui_gate_editor.h"

class gate_editor: public QDialog, public Ui::gate_editor_ui
{ Q_OBJECT
    public:
        gate_editor(QWidget *ownerMeteotable,QWidget *parent);
        void done(int result);

    public slots:
        void gate_remove(void);
        void gate_copy(void);
        void gate_paste(void);
        void gate_chgColor(void);

        void editGate(gate * gate_);
        void newGate(float lon_1, float lat_1,float lon_2, float lat_2,Projection *proj);

    signals:
        void addGate_list(gate*);
        void delGate_list(gate*);

    private:
        QWidget *ownerMeteotable;
        QWidget *parent;

        gate * curGate;

        bool modeCreation;

        QDoubleSpinBox * lat[2][2];
        QDoubleSpinBox * lon[2][2];
        float getValue(int num,int type);
        void setValue(int num,int type,float val);

        QColor gate_color;

        void initEditor(void);
};

#endif // GATE_EDITOR_H
