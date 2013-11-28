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
#include "settings.h"
#include "boat.h"
#include "MainWindow.h"

DialogChooseBarrierSet::DialogChooseBarrierSet(MainWindow * parent): QDialog(parent) {
    mainWindow=parent;
    setupUi(this);
    Util::setFontDialog(this);
}

DialogChooseBarrierSet::~DialogChooseBarrierSet() {
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}

BarrierSet * DialogChooseBarrierSet::chooseBarrierSet(MainWindow *parent) {
    /* no set => return NULL */
    if(::barrierSetList.isEmpty()) {
        QMessageBox::warning(parent,QObject::tr("Barrier set choice"),QObject::tr("No barrier set define, create one before doing this action"));
        return NULL;
    }

    boat * curBoat = parent->getSelectedBoat();

    /* only one set => no dialog */
    if(::barrierSetList.count() == 1 && curBoat->has_barrierSet(::barrierSetList.first())) return ::barrierSetList.first();

    /* more than one set => use dialog to get dialog */
    DialogChooseBarrierSet dialogChooseBarrierSet(parent);
    int nbSetFound= dialogChooseBarrierSet.init_dialog();
    if(nbSetFound==0) {
        QMessageBox::warning(parent,QObject::tr("Barrier set choice"),QObject::tr("No barrier associate to current boat"));
        return NULL;
    }
    if(nbSetFound==1) {
        // search first none hidden set
        for(int i=0;i<(::barrierSetList.count());++i) {
            BarrierSet * set = ::barrierSetList.at(i);
            if(!set->get_isHidden() && curBoat->has_barrierSet(set))
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
    boat * curBoat=mainWindow->getSelectedBoat();
    for(int i=0;i<(::barrierSetList.count());++i) {
        BarrierSet * set = ::barrierSetList.at(i);
        if(!set->get_isHidden() && curBoat->has_barrierSet(set) ) {
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
