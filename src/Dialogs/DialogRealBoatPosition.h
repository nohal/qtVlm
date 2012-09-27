/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGREALBOATPOSITION_H
#define DIALOGREALBOATPOSITION_H

#include "class_list.h"
#include "ui_realBoatPosition.h"

class DialogRealBoatPosition: public QDialog, public Ui::realBoatPos
{ Q_OBJECT
    public:
        DialogRealBoatPosition(QWidget * parent);
        void showDialog(boatReal * boat);
        void done(int result);
    public slots:
        void latLonChg(double);
        void latLonSignChg(int i);
    private:
        boatReal * currentBoat;
        void formatLatLon();
        bool formatWithSeconds;
        double getValue(int type);
        void setValue(int type,double val);
};

#endif // DIALOGREALBOATPOSITION_H
