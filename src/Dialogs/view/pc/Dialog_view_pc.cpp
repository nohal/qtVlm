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

#include <QDebug>

#include "mycentralwidget.h"
#include "Util.h"

#include "Dialog_view_pc.h"

Dialog_view_pc::Dialog_view_pc(myCentralWidget *centralWidget):
    QDialog(centralWidget) {    
    this->cWt=centralWidget;
}

Dialog_view_pc::~Dialog_view_pc() {
    disconnect(cWt,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
}

void Dialog_view_pc::slot_screenResize() {
    Util::setWidgetSize(this,this->sizeHint());
}

void Dialog_view_pc::initDialog(void) {
    connect(cWt,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
}
