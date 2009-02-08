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
***********************************************************************/

#include "race_dialog.h"

race_dialog::race_dialog(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
}

race_dialog::~race_dialog()
{
    
}

void race_dialog::initList(QList<boatAccount*> & acc_list,QList<raceData*> & race_list)
{
    this->acc_list = & acc_list;
    this->race_list = &race_list;
}

void race_dialog::done(int result)
{
    QDialog::done(result);
}

void race_dialog::chgRace(int id)
{

}

void race_dialog::addBoat(void)
{
    
}

void race_dialog::delBoat(void)
{
    
}

