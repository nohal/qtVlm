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
#include <QDebug>

#include "mycentralwidget.h"
#include "dataDef.h"
#include "route.h"
#include "settings.h"
#include "Util.h"
#ifdef QT_V5
#include <QScroller>
#endif
#include "DialogRemoveRoute.h"

DialogRemoveRoute::DialogRemoveRoute(QWidget * parent,myCentralWidget * centralWidget): QDialog(parent) {
    setupUi(this);
#ifdef QT_V5
    QScroller::grabGesture(this->scrollArea->viewport());
#endif
    connect(centralWidget,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setPointSizeF(9.0);
    exceptions.insert(ls_poiList,wfont);
    Util::setSpecificFont(exceptions);
    this->setWindowTitle(tr("Supprimer des routes"));
    this->centralWidget=centralWidget;
    routeList=centralWidget->getRouteList();
    /* init list*/
    ls_poiList->clear();

    for(int i=0;i<routeList.count();++i) {
        if(routeList.at(i)->getBoat()!=centralWidget->getSelectedBoat()) continue;
        QPixmap iconI(20,10);
        iconI.fill(routeList.at(i)->getColor());
        QIcon icon(iconI);
        QListWidgetItem * item = new QListWidgetItem(icon,routeList.at(i)->getName());
        item->setData(Qt::UserRole,qVariantFromValue((void*)routeList.at(i)));
        ls_poiList->addItem(item);
    }

    updateNbSelected();
}
void DialogRemoveRoute::slot_screenResize()
{
    Util::setWidgetSize(this,this->sizeHint());
}

void DialogRemoveRoute::updateNbSelected(void) {
    QString myText;
    myText = QString().setNum(ls_poiList->selectedItems().count()) + " / ";
    myText +=  QString().setNum(ls_poiList->count())+ " ";
    myText += tr("selected Route");

    nbSelected->setText(myText);
}

void DialogRemoveRoute::slot_all(void) {
    ls_poiList->selectAll();
}

void DialogRemoveRoute::slot_none(void) {
    ls_poiList->clearSelection();
}

void DialogRemoveRoute::slot_remove(void) {
    QList<QListWidgetItem*> selectedItems=ls_poiList->selectedItems();
    if(selectedItems.count()==0) return ;
    int rep=QMessageBox::question(this,tr("Removing Routes"),
                          QString(tr("Are you sure to remove %1 routes?")).arg(selectedItems.count()),QMessageBox::Yes | QMessageBox::No);
    if(rep==QMessageBox::Yes)

    {
        qWarning()<<"selected items"<<selectedItems.count();
        for(int i=0;i<selectedItems.count();++i) {
            ROUTE * route = (ROUTE*) selectedItems.at(i)->data(Qt::UserRole).value<void*>();
            if(route)
            {
                centralWidget->myDeleteRoute(route,true);
            }
        }
        Settings::saveGeometry(this);
        accept();
    }
}

void DialogRemoveRoute::slot_itemSelectionChange(void) {
    updateNbSelected();
}
