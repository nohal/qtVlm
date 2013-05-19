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

#include "DialogEditBarrier.h"
#include "BarrierSet.h"

DialogEditBarrier::DialogEditBarrier(QWidget * parent): QDialog(parent) {
    setupUi(this);
    initDialog(NULL);
}

DialogEditBarrier::~DialogEditBarrier(void) {

}

void DialogEditBarrier::initDialog(BarrierSet * barrierSet) {
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
}

void DialogEditBarrier::done(int result) {
    if(result == QDialog::Accepted) {
        if(barrierSet) {
            barrierSet->set_name(name->text());
            barrierSet->set_color(color);
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
