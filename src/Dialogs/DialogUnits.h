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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#ifndef DIALOGUNITS_H
#define DIALOGUNITS_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QLineEdit>

class DialogUnits : public QDialog
{ Q_OBJECT
    public:
        DialogUnits();
    
    public slots:
        void slotBtOK();
        void slotBtCancel();
    
    private:
        QFrame *frameGui;
        QGridLayout *layout;
        
        QPushButton *btOK;
        QPushButton *btCancel;

        QComboBox *cbWindSpeedUnit;
        QComboBox *cbPositionUnit;
        QComboBox *cbDistanceUnit;
        
        QComboBox *cbLongitude;
        QComboBox *cbLatitude;
        
        QFrame * createFrameGui(QWidget *parent);
};


#endif
