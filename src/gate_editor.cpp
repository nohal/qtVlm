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
#include <QtGui>

#include "gate_editor.h"

#define EDT_LAT 0
#define EDT_LON 1

gate_editor::gate_editor(MainWindow * main ,myCentralWidget * parent): QDialog(parent)
{
    setupUi(this);

    this->parent=parent;
    this->main=main;

    lat[0][0] = lat_deg_1;
    lat[0][1] = lat_min_1;
    lat[1][0] = lat_deg_2;
    lat[1][1] = lat_min_2;

    lon[0][0] = lon_deg_1;
    lon[0][1] = lon_min_1;
    lon[1][0] = lon_deg_2;
    lon[1][1] = lon_min_2;


    connect(main,SIGNAL(newGate(float,float,float,float,Projection*)),
            this,SLOT(newGate(float,float,float,float,Projection*)));

    connect(this,SIGNAL(addGate_list(gate*)),parent,SLOT(slot_addGate_list(gate*)));
    connect(this,SIGNAL(delGate_list(gate*)),parent,SLOT(slot_delGate_list(gate*)));

}

void gate_editor::editGate(gate * gate_)
{
    //=> set name
    modeCreation = false;
    curGate = gate_;
    initEditor();
    setWindowTitle(tr("Porte : ")+curGate->getName());
    btDelete->setEnabled(true);
    exec();
}

void gate_editor::newGate(float lat_1, float lon_1,float lat_2, float lon_2,Projection *proj)
{
    modeCreation = true;
    curGate = new gate(tr("Porte"), lat_1, lon_1, lat_2, lon_2,proj, main,parent);
    initEditor();
    setWindowTitle(tr("Nouvelle porte"));
    btDelete->setEnabled(false);
    exec();
}

void gate_editor::initEditor(void)
{
    editName->setText(curGate->getName());

    setValue(0,EDT_LON,curGate->getLon(0));
    setValue(0,EDT_LAT,curGate->getLat(0));
    setValue(1,EDT_LON,curGate->getLon(1));
    setValue(1,EDT_LAT,curGate->getLat(1));

    gate_color=curGate->getColor();
    QString style ="background-color: "+gate_color.name();
    color_previw->setStyleSheet(style);
}

void gate_editor::done(int result)
{
    if(result == QDialog::Accepted)
    {
        curGate->setName(editName->text());
        curGate->setLat(0,getValue(0,EDT_LAT));
        curGate->setLat(1,getValue(1,EDT_LAT));
        curGate->setLon(0,getValue(0,EDT_LON));
        curGate->setLon(1,getValue(1,EDT_LON));
        curGate->setColor(gate_color);
        curGate->updateProjection();
        if (modeCreation)
        {
            curGate->show();
            emit addGate_list(curGate);
        }
    }

    if(result == QDialog::Rejected)
    {
        if (modeCreation)
            delete curGate;
    }
    QDialog::done(result);
}

void gate_editor::gate_remove(void)
{

}

void gate_editor::gate_copy(void)
{

}

void gate_editor::gate_paste(void)
{

}

void gate_editor::gate_chgColor(void)
{
    QColor color = QColorDialog::getColor(gate_color, this);
    if(color.isValid())
    {
        QString style ="background-color: "+color.name();
        color_previw->setStyleSheet(style);
        gate_color=color;
    }
}

float gate_editor::getValue(int num,int type)
{
    float deg = (type==EDT_LAT?lat[num][0]->value():lon[num][0]->value());
    float min = (type==EDT_LAT?lat[num][1]->value():lon[num][1]->value())/60.0;
    if (deg < 0)
        return deg - min;
    else
        return deg + min;
}

void gate_editor::setValue(int num,int type,float val)
{
    int   deg = (int) trunc(val);
    float min = 60.0*fabs(val-trunc(val));

    //qWarning() << "Gate - setVal, num=" << num << ", type=" << type << ", val=" << val << "=> deg=" << deg << ",min=" << min;

    if(type==EDT_LAT)
    {
        lat[num][0]->setValue(deg);
        lat[num][1]->setValue(min);
    }
    else
    {
        lon[num][0]->setValue(deg);
        lon[num][1]->setValue(min);
    }
}

