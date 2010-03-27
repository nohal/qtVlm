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

#include <cmath>

#include "Pilototo_param.h"
#include "Util.h"
#include "Pilototo.h"
#include "POI.h"

#define EDT_LAT 1
#define EDT_LON 2

Pilototo_param::Pilototo_param(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    instruction=NULL;
    //btn_selectPOI->setEnabled(false);
    
    mode->addItem(tr("Cap constant (1)"));
    mode->addItem(tr("Angle du vent (2)"));
    mode->addItem(tr("Pilote ortho (3)"));
    mode->addItem(tr("Meilleur VMG (4)"));
    mode->addItem(tr("VBVMG (5)"));
}

void Pilototo_param::editInstruction(Pilototo_instruction * instruction)
{
    float val;
    
    this->instruction=instruction;

    qWarning() << "Edit instruction std";

    mode->setCurrentIndex(instruction->getMode());
    angle->setValue(instruction->getAngle());
    
    setValue(EDT_LAT,instruction->getLat());
    setValue(EDT_LON,instruction->getLon());

    val=instruction->getWph();
    if(val==-1)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(val));
    exec();
}

void Pilototo_param::editInstructionPOI(Pilototo_instruction * instruction,POI * poi)
{
    float val;

    this->instruction=instruction;

    //qWarning() << "Edit instruction with POI";

    /*mode->setCurrentIndex(instruction->getMode());
    angle->setValue(instruction->getAngle());*/

    if(poi)
    {
        setValue(EDT_LAT,poi->getLatitude());
        setValue(EDT_LON,poi->getLongitude());

        val=poi->getWph();
        if(val==-1)
            editWph->setText(QString());
        else
            editWph->setText(QString().setNum(val));
    }
    exec();
}

void Pilototo_param::done(int result)
{
    if(result==QDialog::Accepted)
    {
        int index=mode->currentIndex();
        instruction->setMode(index);
        if(index == 0 || index == 1)
        {
            instruction->setAngle(angle->value());
        }
        else
        {
            instruction->setLat(getValue(EDT_LAT));
            instruction->setLon(getValue(EDT_LON));
            if(editWph->text().isEmpty())
                instruction->setWph(-1);
            else
                instruction->setWph(editWph->text().toFloat());
        }
    }
    QDialog::done(result);
}

void Pilototo_param::modeChanged(int mode)
{
    if(mode == 0 || mode == 1)
    {
        angle->setEnabled(true);
        position_grp->setEnabled(false);
        if(mode==0)
        {
            angle->setMinimum(0);
            angle->setMaximum(360);
        }
        else
        {
            angle->setMinimum(-180);
            angle->setMaximum(180);
        }
    }
    else
    {
        angle->setEnabled(false);
        position_grp->setEnabled(true);
    }
}

void Pilototo_param::pastePOI(void)
{
    float lat,lon,wph;
    
    if(!Util::getWPClipboard(NULL,&lat,&lon,&wph,NULL))
        return;
    
    setValue(EDT_LON,lon);
    setValue(EDT_LAT,lat);


    if(wph<0 || wph > 360)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(wph));
}

void Pilototo_param::copyPOI(void)
{
    float lat=getValue(EDT_LAT);
    float lon=getValue(EDT_LON);
    float wph;
    if(editWph->text().isEmpty())
        wph=-1;
    else
        wph=editWph->text().toFloat();
    Util::setWPClipboard(lat,lon,wph);
}

void Pilototo_param::selectPOI(void)
{
    emit doSelectPOI(instruction);
    QDialog::done(QDialog::Rejected);
}

float Pilototo_param::getValue(int type)
{
    float deg = (type==EDT_LAT?lat_deg->value():lon_deg->value());
    float min = (type==EDT_LAT?lat_min->value():lon_min->value())/60.0;
    float res;
    /* if min < 0 or deg < 0 the whole value is < 0 */
    if (deg < 0)
    {
        if(min<0)
            res = deg + min;
        else
            res = deg - min;
    }
    else
    {
        if(min<0)
            res=-(deg-min);
        else
            res = deg + min;
    }

    return res;
}

void Pilototo_param::setValue(int type,float val)
{
    int   deg = (int) trunc(val);
    float min = 60.0*fabs(val-trunc(val));

    if(deg==0 && val < 0)
        min=-min;

    if(type==EDT_LAT)
    {
        lat_deg->setValue(deg);
        lat_min->setValue(min);
    }
    else
    {
        lon_deg->setValue(deg);
        lon_min->setValue(min);
    }
}
