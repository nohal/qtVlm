/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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



#include "DialogRealBoatPosition.h"
#include "Util.h"
#include "boatReal.h"
#include "settings.h"
#include "mycentralwidget.h"
#define POI_EDT_LAT 1
#define POI_EDT_LON 2

DialogRealBoatPosition::DialogRealBoatPosition(myCentralWidget * parent) : QDialog(parent->getMainWindow())
{
    setupUi(this);
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
    connect (buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
    connect (buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
    QString tunit = Settings::getSetting(unitsPosition).toString();
    QString unit = (tunit=="") ? "dddegmm'ss" : tunit;
    formatWithSeconds=unit=="dddegmm'ss";
    formatLatLon();
    connect(this->lat_sig,SIGNAL(currentIndexChanged(int)),this,SLOT(latLonSignChg(int)));
    connect(this->lon_sig,SIGNAL(currentIndexChanged(int)),this,SLOT(latLonSignChg(int)));
    connect(this->lat_deg,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
    connect(this->lat_min,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
    connect(this->lat_sec,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
    connect(this->lon_deg,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
    connect(this->lon_min,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
    connect(this->lon_sec,SIGNAL(valueChanged(double)),this,SLOT(latLonChg(double)));
}
void DialogRealBoatPosition::slot_screenResize()
{
    Util::setWidgetSize(this);
}
void DialogRealBoatPosition::formatLatLon()
{
    if(formatWithSeconds)
    {
        this->lat_sec->show();
        this->lon_sec->show();
        this->lat_min->setDecimals(0);
        this->lon_min->setDecimals(0);
    }
    else
    {
        this->lat_sec->hide();
        this->lon_sec->hide();
        this->lat_min->setDecimals(3);
        this->lon_min->setDecimals(3);
    }
    lat_sec->setValue(0);
    lon_sec->setValue(0);
}

void DialogRealBoatPosition::showDialog(boatReal * boat)
{
    if(!boat) return;

    currentBoat=boat;

    setValue(POI_EDT_LON,boat->getLon());
    setValue(POI_EDT_LAT,boat->getLat());

    exec();
}

void DialogRealBoatPosition::done(int result)
{
    if(result==QDialog::Accepted)
    {
        Settings::saveGeometry(this);
        currentBoat->setPosition(getValue(POI_EDT_LAT),
                                 getValue(POI_EDT_LON));
        currentBoat->emitMoveBoat();
    }
    QDialog::done(result);
}

double DialogRealBoatPosition::getValue(int type)
{
    double res;
    double deg = (type==POI_EDT_LAT?lat_deg->value():lon_deg->value());
    double min = (type==POI_EDT_LAT?lat_min->value():lon_min->value())/60.0;
    double sec = (type==POI_EDT_LAT?lat_sec->value():lon_sec->value())/3600.0;
    double sig;
    if(type==POI_EDT_LAT)
        sig=lat_sig->currentIndex()==0?1.0:-1.0;
    else
        sig=lon_sig->currentIndex()==0?1.0:-1.0;
    res=sig*(deg+min+sec);
    return res;
}

void DialogRealBoatPosition::latLonSignChg(int i)
{
    if(this->lat_sig->hasFocus())
    {
        lat_val->blockSignals(true);
        if((i==0 && lat_val->value()<0) || (i==1 && lat_val->value()>0))
            lat_val->setValue(-lat_val->value());
        lat_val->blockSignals(false);
    }
    else if(this->lon_sig->hasFocus())
    {
        lon_val->blockSignals(true);
        if((i==0 && lon_val->value()<0) || (i==1 && lon_val->value()>0))
            lon_val->setValue(-lon_val->value());
        lon_val->blockSignals(false);
    }
}
void DialogRealBoatPosition::latLonChg(double)
{
    double a,b,c,sec;
    int s;
    if(this->lat_deg->hasFocus() || this->lat_min->hasFocus() || this->lat_sec->hasFocus())
    {
        a=lat_deg->value();
        b=lat_min->value();
        sec=lat_sec->value();
        s=lat_sig->currentIndex();
        c=a+b/60.0+sec/3600.0;
        if(s==1) c=-c;
        lat_val->blockSignals(true);
        lat_val->setValue(c);
        lat_val->blockSignals(false);
    }
    else if(this->lon_deg->hasFocus() || this->lon_min->hasFocus() || this->lon_sec->hasFocus())
    {
        a=lon_deg->value();
        b=lon_min->value();
        sec=lon_sec->value();
        s=lon_sig->currentIndex();
        c=a+b/60.0+sec/3600.0;
        if(s==1) c=-c;
        lon_val->blockSignals(true);
        lon_val->setValue(c);
        lon_val->blockSignals(false);
    }
    else if(this->lat_val->hasFocus())
    {
        a=lat_val->value();
        if(a<0)
        {
            a=qAbs(a);
            s=1;
        }
        else
            s=0;
        b=(int) a;
        c=60.0*fabs(a-b);
        lat_deg->blockSignals(true);
        lat_min->blockSignals(true);
        lat_sec->blockSignals(true);
        lat_sig->blockSignals(true);
        lat_deg->setValue(b);
        if(formatWithSeconds)
        {
            sec=c;
            c=(int) sec;
            sec=60.0*fabs(sec-c);
            lat_min->setValue(c);
            lat_sec->setValue(sec);
        }
        else
        {
            lat_min->setValue(c);
            lat_sec->setValue(0);
        }
        lat_sig->setCurrentIndex(s);
        lat_deg->blockSignals(false);
        lat_min->blockSignals(false);
        lat_sec->blockSignals(false);
        lat_sig->blockSignals(false);
    }
    else if(this->lon_val->hasFocus())
    {
        a=lon_val->value();
        if(a<0)
        {
            a=qAbs(a);
            s=1;
        }
        else
            s=0;
        b=(int) a;
        c=60.0*fabs(a-b);
        lon_deg->blockSignals(true);
        lon_min->blockSignals(true);
        lon_sec->blockSignals(true);
        lon_sig->blockSignals(true);
        lon_deg->setValue(b);
        if(formatWithSeconds)
        {
            sec=c;
            c=(int) sec;
            sec=60.0*fabs(sec-c);
            lon_min->setValue(c);
            lon_sec->setValue(sec);
        }
        else
        {
            lon_min->setValue(c);
            lon_sec->setValue(0);
        }
        lon_sig->setCurrentIndex(s);
        lon_deg->blockSignals(false);
        lon_min->blockSignals(false);
        lon_sec->blockSignals(false);
        lon_sig->blockSignals(false);
    }
}
void DialogRealBoatPosition::setValue(int type,double val)
{
    lat_sig->blockSignals(true);
    lat_deg->blockSignals(true);
    lat_min->blockSignals(true);
    lat_sec->blockSignals(true);
    lon_sig->blockSignals(true);
    lon_deg->blockSignals(true);
    lon_min->blockSignals(true);
    lon_sec->blockSignals(true);
    lat_val->blockSignals(true);
    lon_val->blockSignals(true);
    int sig=val<0?1:0;
    val=fabs(val);
    int   deg = (int) val;
    double min = 60.0*fabs(val-deg);
    double sec=0;
    if(formatWithSeconds)
    {
        double Min=min;
        min=(int) Min;
        sec=60.0*fabs(Min-min);
    }
    if(type==POI_EDT_LAT)
    {
        lat_deg->setValue(deg);
        lat_min->setValue(min);
        lat_sec->setValue(sec);
        lat_sig->setCurrentIndex(sig);
        if(sig==1)
            lat_val->setValue(-val);
        else
            lat_val->setValue(val);
    }
    else
    {
        lon_deg->setValue(deg);
        lon_min->setValue(min);
        lon_sec->setValue(sec);
        lon_sig->setCurrentIndex(sig);
        if(sig==1)
            lon_val->setValue(-val);
        else
            lon_val->setValue(val);
    }
    lat_sig->blockSignals(false);
    lat_deg->blockSignals(false);
    lat_min->blockSignals(false);
    lat_sec->blockSignals(false);
    lon_sig->blockSignals(false);
    lon_deg->blockSignals(false);
    lon_min->blockSignals(false);
    lon_sec->blockSignals(false);
    lat_val->blockSignals(false);
    lon_val->blockSignals(false);
}
