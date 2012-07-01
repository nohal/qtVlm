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

#include "DialogPilototoParam.h"
#include "Util.h"
#include "DialogPilototo.h"
#include "POI.h"
#include "settings.h"

#define EDT_LAT 1
#define EDT_LON 2

DialogPilototoParam::DialogPilototoParam(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    instruction=NULL;
    //btn_selectPOI->setEnabled(false);
    
    mode->addItem(tr("Cap constant (1)"));
    mode->addItem(tr("Angle du vent (2)"));
    mode->addItem(tr("Pilote ortho (3)"));
    mode->addItem(tr("Meilleur VMG (4)"));
    mode->addItem(tr("VBVMG (5)"));

    btn_copyPOI->setIcon(QIcon("img/copy.png"));
    btn_pastePOI->setIcon(QIcon("img/paste.png"));
}

void DialogPilototoParam::editInstruction(DialogPilototoInstruction * instruction)
{
    double val;
    
    this->instruction=instruction;

    //qWarning() << "Edit instruction std";

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

void DialogPilototoParam::editInstructionPOI(DialogPilototoInstruction * instruction,POI * poi)
{
    double val;

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

void DialogPilototoParam::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
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
                instruction->setWph(editWph->text().toDouble());
        }
    }
    QDialog::done(result);
}

void DialogPilototoParam::modeChanged(int mode)
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

void DialogPilototoParam::pastePOI(void)
{
    double lat,lon,wph;
    
    if(!Util::getWPClipboard(NULL,&lat,&lon,&wph,NULL))
        return;
    
    setValue(EDT_LON,lon);
    setValue(EDT_LAT,lat);


    if(wph<0 || wph > 360)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(wph));
}

void DialogPilototoParam::copyPOI(void)
{
    double lat=getValue(EDT_LAT);
    double lon=getValue(EDT_LON);
    double wph;
    if(editWph->text().isEmpty())
        wph=-1;
    else
        wph=editWph->text().toDouble();
    Util::setWPClipboard(lat,lon,wph);
}

void DialogPilototoParam::selectPOI(void)
{
    emit doSelectPOI(instruction,2); /* type=editor*/
    QDialog::done(QDialog::Rejected);
}


double DialogPilototoParam::getValue(int type)
{
    double deg = (type==EDT_LAT?lat_deg->value():lon_deg->value());
    double min = (type==EDT_LAT?lat_min->value():lon_min->value())/60.0;
    double res;
    double sig;
    if(type==EDT_LAT)
        sig=lat_sig->currentIndex()==0?1.0:-1.0;
    else
        sig=lon_sig->currentIndex()==0?1.0:-1.0;

    /* if min < 0 or deg < 0 the whole value is < 0 */
    /*if (deg < 0)
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
    }*/

    res=sig*(deg+min);

    //qWarning() << (type==POI_EDT_LAT?"Lat ":"Lon ") << " set to " << res;

    return res;
}

void DialogPilototoParam::setValue(int type,double val)
{
    int sig=val<0?1:0;
    val=fabs(val);
    int   deg = (int) val;
    double min = 60.0*fabs(val-deg);

    /*if(deg==0 && val < 0)
        min=-min;*/

    if(type==EDT_LAT)
    {
        lat_deg->setValue(deg);
        lat_min->setValue(min);
        lat_sig->setCurrentIndex(sig);
    }
    else
    {
        lon_deg->setValue(deg);
        lon_min->setValue(min);
        lon_sig->setCurrentIndex(sig);
    }
}
