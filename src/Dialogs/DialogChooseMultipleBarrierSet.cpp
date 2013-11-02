/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

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

#include "BarrierSet.h"
#include "boat.h"
#include "settings.h"
#include "Util.h"

#include "DialogChooseMultipleBarrierSet.h"

DialogChooseMultipleBarrierSet::DialogChooseMultipleBarrierSet(QWidget *parent): QDialog(parent) {
    setupUi(this);
    Util::setFontDialog(this);
    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setPointSizeF(9.0);
    exceptions.insert(lst_barrierSet,wfont);
    Util::setSpecificFont(exceptions);
    activeSets=NULL;
}
DialogChooseMultipleBarrierSet::~DialogChooseMultipleBarrierSet() {
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}

void DialogChooseMultipleBarrierSet::init_dialog(QList<BarrierSet*> * activeSets, boat* myBoat) {
    this->activeSets = activeSets;
    this->myBoat = myBoat;

    lst_barrierSet->clear();

    /* list init */
    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * barrierSet=::barrierSetList.at(i);
        QListWidgetItem * item = new QListWidgetItem(barrierSet->get_name(),lst_barrierSet);
        /* add barrierSet pointer as first UserRole */
        item->setData(Qt::UserRole,VPtr<BarrierSet>::asQVariant(barrierSet));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(activeSets->contains(barrierSet)?Qt::Checked:Qt::Unchecked);
        //qWarning() << i << ": " << item->text() << ", selected: " << activeSets->contains(barrierSet);
        //item->setSelected(activeSets->contains(barrierSet));
    }
}

void DialogChooseMultipleBarrierSet::done(int result) {
    if(result == QDialog::Accepted) {
        /* update list */
        activeSets->clear();
        if(myBoat->getIsSelected())
            BarrierSet::releaseState();
        for(int i=0;i<lst_barrierSet->count();++i) {
            BarrierSet * barrierSet=VPtr<BarrierSet>::asPtr(lst_barrierSet->item(i)->data(Qt::UserRole));
            if(lst_barrierSet->item(i)->checkState()==Qt::Checked) {                               
                activeSets->append(barrierSet);
                if(myBoat->getIsSelected())
                    barrierSet->set_isHidden(false);
            }
            else {
                if(myBoat->getIsSelected())
                    barrierSet->set_isHidden(true);
            }
        }
    }
    QDialog::done(result);
}

void DialogChooseMultipleBarrierSet::chooseBarrierSet(QWidget *parent, QList<BarrierSet *> * activeSets, boat* myBoat) {
    if(!activeSets || !myBoat) return;
    DialogChooseMultipleBarrierSet dialog(parent);
    dialog.init_dialog(activeSets,myBoat);
    dialog.exec();
}
