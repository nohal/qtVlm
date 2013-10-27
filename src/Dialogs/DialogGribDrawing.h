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

#ifndef DIALOGGRIBDRAWING_H
#define DIALOGGRIBDRAWING_H

#include <QDialog>

#include "class_list.h"

#include "ui_DialogGribDrawing.h"

class DialogGribDrawing: public QDialog,  Ui::DialogGribDrawing_ui {
    public:
        DialogGribDrawing(QWidget *parent,myCentralWidget * centralWidget);

        void done(int result);

    public slots:
        void slot_bgDataType(int);
        void slot_bgDataAlt(int);
        void slot_frstArwType(int);
        void slot_showTemp(bool);
        void slot_smooth(bool);
        void slot_frstArwAlt(int);
        void slot_secArwType(int);
        void slot_secArwAlt(int);
        void slot_showBarbule(bool);
        void slot_showIsoBar(bool);
        void slot_showIsoTherm(bool);
        void slot_isoBarSpacing(int);
        void slot_isoThermSpacing(int);
        void slot_isoBarShowLabel(bool);
        void slot_isoThermShowLabel(bool);
        void slot_isoBarShowMinMax(bool);

    private:
        Terrain * terrain;
        myCentralWidget * centralWidget;

        QStringList levelTypes;
        QStringList levelTypesUnit;
        QStringList dataTypes;
        QStringList arrowTypesFst;
        QStringList arrowTypesSec;

        void init_state(void);
        void init_stringList(void);

};

#endif // DIALOGGRIBDRAWING_H
