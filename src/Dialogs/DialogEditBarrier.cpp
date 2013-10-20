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

#include <QColorDialog>
#include <QMessageBox>

#include "DialogEditBarrier.h"
#include "BarrierSet.h"
#include "boat.h"
#include "settings.h"
#include "Util.h"

DialogEditBarrier::DialogEditBarrier(QWidget * parent): QDialog(parent) {
    setupUi(this);
    Util::setFontDialog(this);
    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setPointSizeF(9.0);
    exceptions.insert(lst_boat,wfont);
    Util::setSpecificFont(exceptions);
    QList<boat *> lst;
    initDialog(NULL,lst);
}

DialogEditBarrier::~DialogEditBarrier(void) {
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}

void DialogEditBarrier::initDialog(BarrierSet * barrierSet,QList<boat *> boatList) {
    if(!barrierSet) {
        this->barrierSet = NULL;
        name->setText("");
        color=Qt::black;
        updateBtnColor();
    }
    else {
        name->setText(barrierSet->get_name());
        color=barrierSet->get_color();
        updateBtnColor();
        this->barrierSet = barrierSet;
    }

    for(int i=0;i<boatList.count();++i) {
        boat * boatPtr = boatList.at(i);
        if(boatPtr->getStatus()) {
            QListWidgetItem * item = new QListWidgetItem(boatPtr->getBoatPseudo(),lst_boat);
            /* add barrierSet pointer as first UserRole */
            item->setData(Qt::UserRole,VPtr<boat>::asQVariant(boatPtr));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(boatPtr->get_barrierSets()->contains(barrierSet)?Qt::Checked:Qt::Unchecked);
        }
    }
}

void DialogEditBarrier::done(int result) {
    if(result == QDialog::Accepted) {        
        if(barrierSet) {
            if(!BarrierSet::validateName(barrierSet,name->text())) {
                QMessageBox::warning(this,tr("Barrier set edit"),tr("Barrier with same name already exists, choose a different name"));
                return;
            }
            barrierSet->set_name(name->text());
            barrierSet->set_color(color);
            for(int i=0;i<lst_boat->count();++i) {
                QListWidgetItem * item = lst_boat->item(i);
                boat * boatPtr = VPtr<boat>::asPtr(item->data(Qt::UserRole));
                if(item->checkState()==Qt::Checked)
                    boatPtr->add_barrierSet(barrierSet);
                else
                    boatPtr->rm_barrierSet(barrierSet);

            }
        }
    }
    QDialog::done(result);
}

void DialogEditBarrier::slot_chgColor(void) {
    if(barrierSet) {
        QColor colorSelected = QColorDialog::getColor(color, this);
        if(colorSelected.isValid()) {
            color=colorSelected;
            updateBtnColor();
        }
    }
}

void DialogEditBarrier::updateBtnColor(void) {
    QString styleSheet=QString().sprintf("background-color: rgb(%d, %d, %d);",color.red(),color.green(),color.blue());
    color_btn->setStyleSheet(styleSheet);
}
