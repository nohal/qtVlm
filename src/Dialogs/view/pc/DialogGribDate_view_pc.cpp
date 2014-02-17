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

#include <QDateTime>
#include <QDebug>

#include "DialogGribDate_view_pc.h"
#include "Util.h"

DialogGribDate_view_pc::DialogGribDate_view_pc(myCentralWidget * centralWidget, DialogGribDate_ctrl *ctrl):
    Dialog_view_pc(centralWidget),
    DialogGribDate_view(centralWidget,ctrl)
{
    INIT_DIALOG
    dateParam->setTimeSpec(Qt::UTC);
}

void DialogGribDate_view_pc::initData(time_t current,std::set<time_t>  * listGrib) {
    initialTime=current;
    time_t min=-1,max=-1;
    // clear current data
    dateList->blockSignals(true); /*to avoid a qt bug with win64 compile*/
    while (dateList->count() > 0)
        dateList->removeItem(0);
    dateList->blockSignals(false);
    dateList->setCurrentIndex(-1);
    QApplication::processEvents();

    // init time
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(current);
    dateParam->setDateTime(dt);

    // init list
    std::set<time_t>::iterator it;
    int index=-1;
    int i=0;
    dateList->addItem(tr("None"),0);
    for (it=listGrib->begin(); it!=listGrib->end(); ++it)
    {
        time_t tps = *it;
        QString str = Util::formatDateTimeLong(tps);
        dt.setTime_t(tps);
        dateList->addItem(str,dt);
        if(tps==current)
            index=i;
        i++;
        if(min==-1) min=tps;
        else if(min>tps) min=tps;
        if(max==-1) max=tps;
        else if(max<tps) max=tps;
    }

    dateList->blockSignals(true);
    if(index!=-1)
        dateList->setCurrentIndex(index+1);
    else
        dateList->setCurrentIndex(0);
    dateList->blockSignals(false);

    //dateParam->setDateTime(dt);
    dt.setTime_t(min);
    dateParam->setMinimumDateTime(dt);
    dt.setTime_t(max);
    dateParam->setMaximumDateTime(dt);

}

time_t DialogGribDate_view_pc::launchDialog(void) {
    if(exec()==QDialog::Accepted) {
        return dateParam->dateTime().toTime_t();
    }
    else
        return initialTime;
}

void DialogGribDate_view_pc::slot_listChanged(int index) {
    if(index!=0)
    {
        dateParam->blockSignals(true);
        dateParam->setDateTime(dateList->itemData(index).toDateTime());
        dateParam->blockSignals(false);
    }
}

void DialogGribDate_view_pc::slot_timeChanged(QDateTime ) {
    dateList->setCurrentIndex(0);
}


