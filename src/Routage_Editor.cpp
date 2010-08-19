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

#include "Routage_Editor.h"
#include "Util.h"
#include "POI.h"
#include "MainWindow.h"
#include "DialogGraphicsParams.h"
#include "mycentralwidget.h"
#include "routage.h"
#include "boatAccount.h"


//-------------------------------------------------------
// ROUTAGE_Editor: Constructor for edit an existing ROUTAGE
//-------------------------------------------------------
ROUTAGE_Editor::ROUTAGE_Editor(ROUTAGE *routage,myCentralWidget *parent)
    : QDialog(parent)
{
    this->routage=routage;
    this->parent=parent;
    setupUi(this);
    inputTraceColor =new InputLineParams(routage->getWidth(),routage->getColor(),1.6,  QColor(Qt::red),this,0.1,5);
    verticalLayout_2->addWidget( inputTraceColor);
    setWindowTitle(tr("Parametres Routage"));
    editName->setText(routage->getName());
    editDateBox->setDateTime(routage->getStartTime());
    editDateBox->setEnabled(true);
    QListIterator<boatAccount*> i (parent->getBoats());
    int n=0;
    while(i.hasNext())
    {
        boatAccount * acc = i.next();
        if(acc->getStatus())
        {
            if(acc->getAliasState())
                editBoat->addItem(acc->getAlias() + "(" + acc->getLogin() + ")");
            else
                editBoat->addItem(acc->getLogin());
            if(acc==routage->getBoat()) editBoat->setCurrentIndex(n);
            n++;
        }
    }
    QListIterator<POI*> j (parent->getPois());
    n=0;
    while(j.hasNext())
    {
        POI * poi = j.next();
        if(poi->getRoute()==NULL)
        {
            fromPOI->addItem(poi->getName());
            if(poi==routage->getFromPOI()) fromPOI->setCurrentIndex(n);
            toPOI->addItem(poi->getName());
            if(poi==routage->getToPOI()) toPOI->setCurrentIndex(n);
            n++;
        }
    }
    this->range->setValue(routage->getAngleRange());
    this->step->setValue(routage->getAngleStep());
    this->duree->setValue(routage->getTimeStep());
    this->windForced->setChecked(routage->getWindIsForced());
    this->showIso->setChecked(routage->getShowIso());
    this->explo->setValue(routage->getExplo());
    if(routage->getWindIsForced())
    {
        this->TWD->setValue(routage->getWindAngle());
        this->TWS->setValue(routage->getWindSpeed());
    }
    if(routage->isDone())
    {
        this->editName->setDisabled(true);
        this->editBoat->setDisabled(true);
        this->editDateBox->setDisabled(true);
        this->fromPOI->setDisabled(true);
        this->toPOI->setDisabled(true);
        this->duree->setDisabled(true);
        this->range->setDisabled(true);
        this->step->setDisabled(true);
        this->explo->setDisabled(true);
        this->windForced->setDisabled(true);
        this->TWD->setDisabled(true);
        this->TWS->setDisabled(true);
        if(routage->isConverted())
            this->convRoute->setDisabled(true);
    }
}
ROUTAGE_Editor::~ROUTAGE_Editor()
{
}
//---------------------------------------
void ROUTAGE_Editor::done(int result)
{
    if(result == QDialog::Accepted)
    {
        if (!parent->freeRoutageName((editName->text()).trimmed(),routage))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Ce nom est deja utilise, choisissez en un autre"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return;
        }
        routage->setName((editName->text()).trimmed());
        routage->setWidth(inputTraceColor->getLineWidth());
        routage->setColor(inputTraceColor->getLineColor());
        routage->setStartTime(editDateBox->dateTime());
        QListIterator<boatAccount*> i (parent->getBoats());
        while(i.hasNext())
        {
            boatAccount * acc = i.next();
            if(acc->getLogin()==editBoat->currentText())
            {
                 routage->setBoat(acc);
                 break;
            }
        }
        QListIterator<POI*> j (parent->getPois());
        while(j.hasNext())
        {
            POI * poi = j.next();
            if(poi->getName()==fromPOI->currentText())
            {
                 routage->setFromPOI(poi);
            }
            if(poi->getName()==toPOI->currentText())
            {
                 routage->setToPOI(poi);
            }
        }
        routage->setWindIsForced(windForced->isChecked());
        routage->setWind(TWD->value(),TWS->value());
        routage->setAngleRange(this->range->value());
        routage->setAngleStep(this->step->value());
        routage->setTimeStep(this->duree->value());
        routage->setExplo(this->explo->value());
        routage->setShowIso(this->showIso->isChecked());
        if(this->convRoute->isChecked())
        {
            if(!routage->isConverted())
            {
                if(routage->isDone())
                    routage->convertToRoute();
                else
                    routage->setConverted();
            }
        }
    }
    if(result == QDialog::Rejected)
    {
    }
    QDialog::done(result);
}

//---------------------------------------

void ROUTAGE_Editor::on_windForced_toggled(bool checked)
{
   TWD->setEnabled(checked);
   TWS->setEnabled(checked);
}

