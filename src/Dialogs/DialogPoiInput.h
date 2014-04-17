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



#ifndef POI_INPUT_H
#define POI_INPUT_H

#include <QDialog>

#include "class_list.h"
#include "ui_POI_input.h"

class DialogPoiInput : public QDialog, public Ui::POI_input_ui
{
    Q_OBJECT
    public:
        DialogPoiInput(myCentralWidget * parent);
        void done(int result);

    public slots:
        void txtHasChanged(void);
        void slot_showPOI_input(void);
        
        void slot_screenResize();
signals:
        void addPOI(QString,int,double,double,double);
};



#endif
