/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#include "DialogChooseMultipleBoat.h"

DialogChooseMultipleBoat::DialogChooseMultipleBoat(QWidget *parent): QDialog(parent) {
    setupUi(this);
    Util::setFontDialog(this);
}

void DialogChooseMultipleBoat::init_dialog(BarrierSet *barrierSet,QList<boat*> boatList) {

    lst_barrierSet->clear();
    this->barrierSet=barrierSet;
    /* list init */
    for(int i=0;i<boatList.count();++i) {
        boat * boatPtr = boatList.at(i);
        if(boatPtr->getStatus()) {
            QListWidgetItem * item = new QListWidgetItem(boatPtr->getBoatPseudo(),lst_barrierSet);
            /* add barrierSet pointer as first UserRole */
            item->setData(Qt::UserRole,VPtr<boat>::asQVariant(boatPtr));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(boatPtr->get_barrierSets()->contains(barrierSet)?Qt::Checked:Qt::Unchecked);
           // item->setSelected(boatPtr->get_barrierSets()->contains(barrierSet));
        }
    }
}

void DialogChooseMultipleBoat::done(int result) {
    if(result == QDialog::Accepted) {
        for(int i=0;i<lst_barrierSet->count();++i) {
            QListWidgetItem * item = lst_barrierSet->item(i);
            boat * boatPtr = VPtr<boat>::asPtr(item->data(Qt::UserRole));
            if(item->checkState()==Qt::Checked)
                boatPtr->add_barrierSet(barrierSet);
            else
                boatPtr->rm_barrierSet(barrierSet);

        }
    }
    QDialog::done(result);
}

void DialogChooseMultipleBoat::chooseBoat(QWidget *parent, BarrierSet * barrierSet, QList<boat *> boatList) {
    if(!barrierSet) return;
    DialogChooseMultipleBoat dialog(parent);
    dialog.init_dialog(barrierSet,boatList);
    dialog.exec();
}
