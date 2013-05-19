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

#include "DialogChooseMultipleBarrierSet.h"

DialogChooseMultipleBarrierSet::DialogChooseMultipleBarrierSet(QWidget *parent): QDialog(parent) {
    setupUi(this);
    activeSets=NULL;
}

void DialogChooseMultipleBarrierSet::init_dialog(QList<BarrierSet*> * activeSets) {
    this->activeSets = activeSets;

    lst_barrierSet->clear();

    /* list init */
    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * barrierSet=::barrierSetList.at(i);
        QListWidgetItem * item = new QListWidgetItem(barrierSet->get_name(),lst_barrierSet);
        /* add barrierSet pointer as first UserRole */
        item->setData(Qt::UserRole,VPtr<BarrierSet>::asQVariant(barrierSet));
        qWarning() << i << ": " << item->text() << ", selected: " << activeSets->contains(barrierSet);
        item->setSelected(activeSets->contains(barrierSet));
    }
}

void DialogChooseMultipleBarrierSet::done(int result) {
    if(result == QDialog::Accepted) {
        /* update list */
        activeSets->clear();
        QList<QListWidgetItem *> selectedItems = lst_barrierSet->selectedItems();
        for(int i=0;i<selectedItems.count();++i)
            activeSets->append(VPtr<BarrierSet>::asPtr(selectedItems.at(i)->data(Qt::UserRole)));
    }
    QDialog::done(result);
}

void DialogChooseMultipleBarrierSet::chooseBarrierSet(QWidget *parent, QList<BarrierSet *> * activeSets) {
    if(!activeSets) return;
    DialogChooseMultipleBarrierSet dialog(parent);
    dialog.init_dialog(activeSets);
    dialog.exec();
}
