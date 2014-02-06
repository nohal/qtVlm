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

#include <QDebug>

#include "Util.h"
#include "DialogGribDate.h"
#include <QScroller>
#include <mycentralwidget.h>
DialogGribDate::DialogGribDate(myCentralWidget * parent) : QDialog(parent->getMainWindow())
{
    setupUi(this);
    QScroller::grabGesture(this->scrollArea->viewport());
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    result=NULL;
    startTime=0;
    listGribDates.clear();
    dateParam->setTimeSpec(Qt::UTC);
}
void DialogGribDate::slot_screenResize()
{
    Util::setWidgetSize(this,this->sizeHint());
}

void DialogGribDate::showDialog(time_t current,std::set<time_t>  * listGrib,time_t * result)
{

    if(!result)
        return;
    this->result=result;
    startTime=current;

    time_t min=-1,max=-1;

    /* clear current status*/
    listGribDates.clear();
    dateList->blockSignals(true); /*to avoid a qt bug with win64 compile*/
    while (dateList->count() > 0)
    {
        dateList->removeItem(0);
    }
    dateList->blockSignals(false);
    dateList->setCurrentIndex(-1);
    QApplication::processEvents();

    /* init with new data*/
    /* init internal list*/
    std::set<time_t>::iterator its;
    for (its=listGrib->begin(); its!=listGrib->end(); ++its)
        listGribDates.push_back(*its);

    QApplication::processEvents();
    /* init dialog*/
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(current);
    //qWarning() << dt;
    dateParam->setDateTime(dt);
    std::vector<time_t>::iterator it;
    int index=-1;
    int i=0;
    dateList->addItem("");

    for (it=listGribDates.begin(); it!=listGribDates.end(); ++it)
    {
        time_t tps = *it;
        QString str = Util::formatDateTimeLong(tps);
        dateList->addItem(str);
        //qWarning() << i << " - tps " << tps;
        if(tps==current)
            index=i;
        i++;
        if(min==-1) min=tps;
        else if(min>tps) min=tps;
        if(max==-1) max=tps;
        else if(max<tps) max=tps;
    }
    QApplication::processEvents();
    listIsChanging=true;
    if(index!=-1)
        dateList->setCurrentIndex(index+1);
    else
        dateList->setCurrentIndex(0);

    dateParam->setDateTime(dt);
    listIsChanging=false;
    dt.setTime_t(min);
    dateParam->setMinimumDateTime(dt);
    dt.setTime_t(max);
    dateParam->setMaximumDateTime(dt);

    exec();
}

void DialogGribDate::done(int res)
{
    if(res == QDialog::Accepted)
        *result=dateParam->dateTime().toTime_t();
    else
        *result=startTime;
    QDialog::done(res);
}

void DialogGribDate::listChanged(int index)
{
    if(index!=0)
    {
        index--;
        QDateTime dt;
        dt.setTimeSpec(Qt::UTC);
        dt.setTime_t(listGribDates[index]);
        listIsChanging=true;
        dateParam->setDateTime(dt);
        listIsChanging=false;
    }
}

void DialogGribDate::paramChanged(QDateTime)
{
    if(!listIsChanging)
        dateList->setCurrentIndex(0);
}
