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

#include <QListWidget>
#include <QMessageBox>

#include "POI.h"
#include "mycentralwidget.h"
#include "dataDef.h"
#include "route.h"
#include "settings.h"

#include "DialogRemovePoi.h"

DialogRemovePoi::DialogRemovePoi(QWidget * parent,myCentralWidget * centralWidget): QDialog(parent) {
    setupUi(this);
    Util::setFontDialog(this);

    this->centralWidget=centralWidget;
    poiList=centralWidget->getPois();
    /* init list*/
    ls_poiList->clear();

    for(int i=0;i<poiList.count();++i) {
        if(poiList.at(i)->getRoute()!=NULL) continue;
        QListWidgetItem * item = new QListWidgetItem(poiList.at(i)->getName());
        item->setData(Qt::UserRole,VPtr<POI>::asQVariant(poiList.at(i)));
        ls_poiList->addItem(item);
    }

    updateNbSelected();
}

void DialogRemovePoi::updateNbSelected(void) {
    QString myText;
    myText = QString().setNum(ls_poiList->selectedItems().count()) + " / ";
    myText +=  QString().setNum(ls_poiList->count())+ " ";
    myText += tr("selected POI");

    nbSelected->setText(myText);
}

void DialogRemovePoi::slot_all(void) {
    ls_poiList->selectAll();
}

void DialogRemovePoi::slot_none(void) {
    ls_poiList->clearSelection();
}

void DialogRemovePoi::slot_remove(void) {
    QList<QListWidgetItem*> selectedItems=ls_poiList->selectedItems();
    if(selectedItems.count()==0) return ;
    if(QMessageBox::question(this,tr("Removing POI"),
                          QString(tr("Are you sure to remove %1 POI?")).arg(selectedItems.count())
                             ) == QMessageBox::Ok)
    {
        for(int i=0;i<selectedItems.count();++i) {
            POI * poi = VPtr<POI>::asPtr(selectedItems.at(i)->data(Qt::UserRole));
            if(poi)
            {
                if(poi->getRoute()!=NULL)
                {
                    if(poi->getRoute()->getFrozen()||poi->getRoute()->getHidden()||poi->getRoute()->isBusy()) continue;
                    poi->setRoute(NULL);
                }
                centralWidget->slot_delPOI_list(poi);
                poi->deleteLater();
            }
        }
        accept();
    }
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
}

void DialogRemovePoi::slot_itemSelectionChange(void) {
    updateNbSelected();
}
