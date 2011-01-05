/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#include "DialogFinePosit.h"
#include "POI.h"
#include "mycentralwidget.h"
#include "settings.h"


DialogFinePosit::DialogFinePosit(POI * poi,myCentralWidget *parent)
    : QDialog(parent)
{
    this->poi=poi;
    this->parent=parent;
    setupUi(this);
    setWindowTitle(tr("Parametres du positionnement automatique"));
    etendueLon->setValue(poi->getSearchRangeLon());
    etendueLat->setValue(poi->getSearchRangeLat());
    step->setValue(poi->getSearchStep());
    drawRoute->setChecked(poi->getOptimizing());
    this->etendueLon->setFocus();
    this->etendueLon->selectAll();
    this->keepOldMe->setChecked(Settings::getSetting("KeepOldPoi","0").toInt()==1);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
}
DialogFinePosit::~DialogFinePosit()
{
}
//---------------------------------------
void DialogFinePosit::done(int result)
{
    if(result == QDialog::Accepted)
    {
        poi->setSearchRangeLon(etendueLon->value());
        poi->setSearchRangeLat(etendueLat->value());
        poi->setSearchStep(step->value());
        poi->setOptimizing(drawRoute->isChecked());
        Settings::setSetting("KeepOldPoi",keepOldMe->isChecked()?"1":"0");
    }
    if(result == QDialog::Rejected)
    {
    }
    QDialog::done(result);
}


