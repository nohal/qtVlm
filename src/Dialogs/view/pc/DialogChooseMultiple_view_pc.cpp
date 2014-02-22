/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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

#include <QMap>
#include <QFont>
#include <QDebug>

#include "Util.h"
#include "settings.h"

#include "DialogChooseMultiple_view_pc.h"

DialogChooseMultiple_view_pc::DialogChooseMultiple_view_pc(myCentralWidget *centralWidget) :
    Dialog_view_pc(centralWidget),
    DialogChooseMultiple_view(centralWidget)
{
    INIT_DIALOG

    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setPointSizeF(9.0);
    exceptions.insert(lst,wfont);
    Util::setSpecificFont(exceptions);
}



QList<dataItem> DialogChooseMultiple_view_pc::launchDialog(QString dialogTitle,QString title,QList<dataItem> dataItemList) {

    setWindowTitle(dialogTitle);
    this->title->setText(title);

    lst->clear();

    for(int i=0;i<dataItemList.size();++i) {
        QListWidgetItem * item = new QListWidgetItem(dataItemList.at(i).str,lst);
        item->setData(Qt::UserRole,dataItemList.at(i).dataVariant);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(dataItemList.at(i).checked?Qt::Checked:Qt::Unchecked);
        lst->addItem(item);
    }

    QList<dataItem> result;

    if(exec()==QDialog::Accepted) {
        Settings::saveGeometry(this);
        for(int i=0;i<lst->count();++i) {
            QListWidgetItem * item = lst->item(i);
            dataItem it;
            it.dataVariant=item->data(Qt::UserRole);
            it.checked=item->checkState()==Qt::Checked;
            result.append(it);
        }
        return result;
    }
    else {
        Settings::saveGeometry(this);
        return result;
    }
}
