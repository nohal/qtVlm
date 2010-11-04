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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#include <cmath>
#include <QMessageBox>
#include <QDebug>

#include "Route_Editor.h"
#include "Util.h"
#include "MainWindow.h"
#include "DialogGraphicsParams.h"
#include "mycentralwidget.h"
#include "route.h"
#include "boatVLM.h"


//-------------------------------------------------------
// ROUTE_Editor: Constructor for edit an existing ROUTE
//-------------------------------------------------------
ROUTE_Editor::ROUTE_Editor(ROUTE *route,myCentralWidget *parent)
    : QDialog(parent)
{
    this->route=route;
    this->parent=parent;
    setupUi(this);
    inputTraceColor =new InputLineParams(route->getWidth(),route->getColor(),1.6,  QColor(Qt::red),this,0.1,5);
    verticalLayout_2->addWidget( inputTraceColor);
    setWindowTitle(tr("Parametres Route"));
    editName->setText(route->getName());
    editFrozen->setChecked(route->getFrozen());


    startFromBoat->setChecked(route->getStartFromBoat());
    startFromMark->setChecked(!route->getStartFromBoat());

    editDateBox->setDateTime(route->getStartTime());

    editLive->setChecked(route->isLive());
    hidePois->setChecked(route->getHidePois());
    vacStep->setValue(route->getMultVac());
    switch(route->getStartTimeOption())
    {
    case 1:
        editVac->setChecked(true);
        break;
    case 2:
        editGrib->setChecked(true);
        break;
    case 3:
        editDate->setChecked(true);
        editDateBox->setEnabled(true);
    }
    int n=0;
    if(parent->getBoats())
    {
        QListIterator<boatVLM*> i (*parent->getBoats());
        while(i.hasNext())
        {
            boatVLM * acc = i.next();
            if(acc->getStatus())
            {
                if(acc->getAliasState())
                    editBoat->addItem(acc->getAlias() + "(" + acc->getBoatName() + ")");
                else
                    editBoat->addItem(acc->getBoatName());
                if(acc->getId()==route->getBoat()->getId()) editBoat->setCurrentIndex(n);
                n++;
            }
        }
    }
    //if(route->isImported()) this->editFrozen->setEnabled(false);
}
ROUTE_Editor::~ROUTE_Editor()
{
}
//---------------------------------------
void ROUTE_Editor::done(int result)
{
    if(result == QDialog::Accepted)
    {
        if (!parent->freeRouteName((editName->text()).trimmed(),route))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Ce nom est deja utilise, choisissez en un autre"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        route->setName((editName->text()).trimmed());
        route->setWidth(inputTraceColor->getLineWidth());
        route->setColor(inputTraceColor->getLineColor());
        route->setMultVac(vacStep->value());
        if(editVac->isChecked())
            route->setStartTimeOption(1);
        if (editGrib->isChecked())
            route->setStartTimeOption(2);
        if (editDate->isChecked())
        {
            route->setStartTimeOption(3);
            route->setStartTime(editDateBox->dateTime());
        }
        route->setStartFromBoat(startFromBoat->isChecked());

        if(parent->getBoats())
        {
            QListIterator<boatVLM*> i (*parent->getBoats());
            while(i.hasNext())
            {
                boatVLM * acc = i.next();
                if(acc->getBoatName()==editBoat->currentText())
                {
                    route->setBoat(acc);
                    break;
                }
            }
        }

        route->setFrozen(editFrozen->isChecked());
        route->setLive(editLive->isChecked());
        if(hidePois->isChecked()!=route->getHidePois())
            route->setHidePois(hidePois->isChecked());
    }
    if(result == QDialog::Rejected)
    {
    }
    QDialog::done(result);
}

//---------------------------------------
