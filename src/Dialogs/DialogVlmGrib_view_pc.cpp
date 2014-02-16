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

#ifdef QT_V5
#include "QScroller"
#endif

#include "mycentralwidget.h"
#include "Util.h"
#include "settings.h"

#include "DialogVlmGrib_ctrl.h"
#include "DialogVlmGrib_view_pc.h"

DialogVlmGrib_view_pc::DialogVlmGrib_view_pc(myCentralWidget * centralWidget,DialogVlmGrib_ctrl * ctrl): QDialog(centralWidget), DialogVlmGrib_view(centralWidget,ctrl)
{
    setupUi(this);
#ifdef QT_V5
    QScroller::grabGesture(this->scrollArea->viewport());
#endif
    connect(centralWidget,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    listRadio[0]=radio1;
    listRadio[1]=radio2;
    listRadio[2]=radio3;
    listRadio[3]=radio4;
    listRadio[4]=radio5;

    waitBox = new QMessageBox(QMessageBox::Information,
                             tr("VLM Grib"),
                             tr("Chargement de la liste de grib"),QMessageBox::NoButton,this);
}

void DialogVlmGrib_view_pc::set_waitBoxVisibility(bool visible) {
    if(visible)
        waitBox->exec();
    else
        waitBox->hide();
}

void DialogVlmGrib_view_pc::set_dialogVisibility(bool visible) {
    if(visible) {
        show();
        setWindowModality(Qt::ApplicationModal);
    }
    else
        hide();
}

void DialogVlmGrib_view_pc::updateList(QStringList lst) {
    for(int i=0;i<5;++i)
        listRadio[i]->setEnabled(false);

    for(int i=0;i<lst.size();++i) {
        listRadio[i]->setText(lst.at(i) );
        listRadio[i]->setEnabled(true);
    }

    listRadio[lst.size()-1]->setChecked(true);
}

void DialogVlmGrib_view_pc::slot_screenResize() {
    Util::setWidgetSize(this,this->sizeHint());
}

void DialogVlmGrib_view_pc::slot_download(void) {
    int i;
    for(i=0;i<5;i++)
        if(listRadio[i]->isChecked())
            break;
    if(i!=5) {
        Settings::saveGeometry(this);
        ctrl->downloadGrib(i);
    }
}

void DialogVlmGrib_view_pc::slot_cancel(void) {
    Settings::saveGeometry(this);
    ctrl->exitDialog();
}
