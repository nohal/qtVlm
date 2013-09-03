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

#include <QMessageBox>

#include "BarrierSet.h"
#include "DialogChooseBarrierSet.h"
#include "Util.h"

DialogChooseBarrierSet::DialogChooseBarrierSet(QWidget * parent): QDialog(parent) {
    setupUi(this);
    Util::setFontDialog(this);
}

BarrierSet * DialogChooseBarrierSet::chooseBarrierSet(QWidget *parent) {
    /* no set => return NULL */
    if(::barrierSetList.isEmpty()) {
        QMessageBox::warning(parent,tr("Barrier set choice"),tr("No barrier set define, create one before doing this action"));
        return NULL;
    }

    /* only one set => no dialog */
    if(::barrierSetList.count() == 1) return ::barrierSetList.first();

    /* more than one set => use dialog to get dialog */
    DialogChooseBarrierSet dialogChooseBarrierSet(parent);
    int res= dialogChooseBarrierSet.init_dialog();
    if(res==0) return NULL;
    if(res==1) {
        // search first none hidden set
        for(int i=0;i<(::barrierSetList.count());++i) {
            BarrierSet * set = ::barrierSetList.at(i);
            if(!set->get_isHidden())
                return set;
        }
    }



    if(dialogChooseBarrierSet.exec() == QDialog::Accepted)
        return dialogChooseBarrierSet.get_choice();
    else
        return NULL;

}

int DialogChooseBarrierSet::init_dialog(void) {
    int nbSet=0;
    cb_barrierSets->clear();
    qSort(::barrierSetList.begin(),::barrierSetList.end(),BarrierSet::myLessThan);
    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * set = ::barrierSetList.at(i);
        if(!set->get_isHidden()) {
            nbSet++;
            QVariant data = VPtr<BarrierSet>::asQVariant(set);
            cb_barrierSets->addItem(set->get_name(),data);
        }
    }
    return nbSet;
}

void DialogChooseBarrierSet::done(int result) {
    if(result == QDialog::Accepted) {
        choice=VPtr<BarrierSet>::asPtr(cb_barrierSets->itemData(cb_barrierSets->currentIndex()));
    }
    QDialog::done(result);
}
