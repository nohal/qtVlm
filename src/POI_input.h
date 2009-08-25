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

#include "ui_POI_input.h"

class POI_input : public QDialog, public Ui::POI_input_ui
{
    Q_OBJECT
    public:
        POI_input(QWidget * parent = 0);
        void done(int result);

    public slots:
        void txtHasChanged(void);
        
    signals:
        void addPOI(QString,float,float,float,int,bool);
};



#endif
