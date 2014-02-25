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
#ifdef QT_V5
#include <QtWidgets/QMessageBox>
#include <QScroller>
#else
#include <QMessageBox>
#endif
#include <QDebug>

class POI_Editor;

#include "DialogPoi.h"
#include "Util.h"
#include "MainWindow.h"
#include "boatVLM.h"
#include <QDesktopWidget>
#include "settings.h"
#define POI_EDT_LAT 1
#define POI_EDT_LON 2

//-------------------------------------------------------
// POI_Editor: Constructor for edit an existing POI
//-------------------------------------------------------
DialogPoi::DialogPoi(MainWindow * main,myCentralWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
#ifdef QT_V5
    QScroller::grabGesture(this->scrollArea->viewport());
#endif
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    Util::setFontDialog(this);
//    int minSize=qMax(this->lat_sig->width(),this->lon_sig->width());
//    lat_sig->setMinimumWidth(minSize);
//    lon_sig->setMinimumWidth(minSize);
    this->poi = NULL;
    this->parent=parent;
    this->main=main;

    lock=true;
    modeCreation=false;

    connect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));

    connect(this,SIGNAL(doChgWP(double,double,double)),main,SLOT(slotChgWP(double,double,double)));
    QString tunit = Settings::getSetting(unitsPosition).toString();
    QString unit = (tunit=="") ? "dddegmm'ss" : tunit;
    formatWithSeconds=unit=="dddegmm'ss";
    navMode->addItem("VB-VMG");
    navMode->addItem("B-VMG");
    navMode->addItem("Ortho");
}
void DialogPoi::slot_screenResize()
{
    Util::setWidgetSize(this);
}
void DialogPoi::formatLatLon()
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

void DialogPoi::initPOI(POI * poi,const bool &creationMode)
{
    this->poi=poi;
    this->modeCreation=creationMode;
    if(modeCreation)
    {
        setWindowTitle(tr("Nouvelle marque"));
        btDelete->setEnabled(false);
    }
    else
    {
        setWindowTitle(tr("Marque : ")+poi->getName());
    }
    //this->setEnabled(true);
    QString tunit = Settings::getSetting(unitsPosition).toString();
    QString unit = (tunit=="") ? "dddegmm'ss" : tunit;
    formatWithSeconds=unit=="dddegmm'ss";
    this->formatLatLon();
    editName->setText(poi->getName());
    this->sequence->setValue(poi->getSequence());
    oldType=poi->getType();
    POI_type_liste->setCurrentIndex(oldType);
    QListIterator<ROUTE*> i (parent->getRouteList());
    cb_routeList->clear();
    cb_routeList->addItem("Not in a route");
    while(i.hasNext())
    {
        ROUTE * route=i.next();
        if(Settings::getSetting(autoHideRoute).toInt()==1 && (route->getBoat()==NULL || !route->getBoat()->getIsSelected())) continue;
        QPixmap iconI(20,10);
        iconI.fill(route->getColor());
        QIcon icon(iconI);
        cb_routeList->addItem(icon,route->getName());
    }
    if(poi->getRoute()!=NULL)
        cb_routeList->setCurrentIndex(cb_routeList->findText(((ROUTE*) poi->getRoute())->getName()));
    if(poi->getWph()==-1)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(poi->getWph()));

    btSaveWP->setEnabled(!main->getBoatLockStatus());

    setValue(POI_EDT_LON,poi->getLongitude());
    setValue(POI_EDT_LAT,poi->getLatitude());

    if(poi->getTimeStamp()!=-1)
    {
        QDateTime tm;
        tm.setTimeSpec(Qt::UTC);
        tm.setTime_t(poi->getTimeStamp());
        editTStamp->setDateTime(tm);
        editTStamp->setTimeSpec(Qt::UTC);
    }
    else
    {
        QDateTime tm = QDateTime::currentDateTime().toUTC();
        editTStamp->setDateTime(tm);
    }
    navMode->setCurrentIndex(poi->getNavMode());

    chk_tstamp->setCheckState(poi->getUseTimeStamp()?Qt::Checked:Qt::Unchecked);
    editTStamp->setEnabled(poi->getUseTimeStamp());
    editName->setEnabled(!poi->getUseTimeStamp());
    this->notSimplicable->setChecked(poi->getNotSimplificable());

    boat * ptr=parent->getSelectedBoat();
    if(ptr)
    {
        btSaveWP->setText(tr("Marque->WP\n(")+ptr->getBoatPseudo()+")");
        btSaveWP->setEnabled(true);
    }
    else
        btSaveWP->setEnabled(false);
    editName->setSelection(0,editName->text().length());
    editName->setFocus();
}

//---------------------------------------
void DialogPoi::done(int result)
{
    this->setEnabled(false);
    Settings::saveGeometry(this);
    if(result == QDialog::Accepted)
    {
        poi->setPartOfTwa(false);
        QDateTime tm = editTStamp->dateTime();
        poi->setNotSimplificable(this->notSimplicable->checkState()==Qt::Checked);
        tm.setTimeSpec(Qt::UTC);
        poi->setTimeStamp(tm.toTime_t());
        poi->setUseTimeStamp(chk_tstamp->checkState()==Qt::Checked);
        poi->setSequence(this->sequence->value());

        poi->setLongitude(getValue(POI_EDT_LON));
        poi->setLatitude (getValue(POI_EDT_LAT));
        poi->manageLineBetweenPois();
 //       int oldtype=poi->getType();
        poi->setType(POI_type_liste->currentIndex());
        poi->setName((editName->text()).trimmed() );
        if(editWph->text().isEmpty())
            poi->setWph(-1);
        else
            poi->setWph(editWph->text().toDouble());
        poi->slot_updateProjection();
        poi->setNavMode(navMode->currentIndex());

        if (modeCreation) {
            //poi->show();
            emit addPOI_list(poi);
        }
        if(cb_routeList->currentIndex()!=0)
        {
            for (int n=0;n<parent->getRouteList().size();++n)
            {
                if(parent->getRouteList().at(n)->getName()==cb_routeList->currentText())
                {
                    ROUTE * route=parent->getRouteList().at(n);
                    if(!route->getFrozen())
                    {
                        poi->setRoute(route);
                        break;
                    }
                    else
                        break;
                }
            }
        }
        else
            poi->setRoute(NULL);
        if(poi->getIsWp()) emit doChgWP(poi->getLatitude(),poi->getLongitude(),poi->getWph());
    }

    if(result == QDialog::Rejected)
    {
        if (modeCreation)
                poi->deleteLater();
    }
    QDialog::done(result);
}

//---------------------------------------
void DialogPoi::btDeleteClicked()
{
    this->btCancel->setEnabled(false);
    if (! modeCreation) {
        int rep = QMessageBox::question (this,
            tr("Detruire la marque : %1").arg(poi->getName()),
            tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes)
        {
            emit delPOI_list(poi);
            poi->deleteLater();
            QDialog::done(QDialog::Accepted);
        }
        else
            this->btCancel->setEnabled(true);
    }
}

void DialogPoi::btPasteClicked()
{
    double lat,lon,wph;
    QString name;
    int tstamp;
    if(!Util::getWPClipboard(&name,&lat,&lon,&wph,&tstamp))
        return;

    setValue(POI_EDT_LON,lon);
    setValue(POI_EDT_LAT,lat);

    if(tstamp==-1)
    {
        QDateTime tm = QDateTime::currentDateTime().toUTC();
        editTStamp->setDateTime(tm);
        chk_tstamp->setCheckState(Qt::Unchecked);
    }
    else
    {
        QDateTime tm;
        tm.setTimeSpec(Qt::UTC);
        tm.setTime_t(tstamp);
        editTStamp->setDateTime(tm);
        editTStamp->setTimeSpec(Qt::UTC);
        chk_tstamp->setCheckState(Qt::Checked);
    }

    if(name!="")
        editName->setText(name);

    if(wph<0 || wph > 360)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(wph));
}

void DialogPoi::btCopyClicked()
{
    Util::setWPClipboard(getValue(POI_EDT_LAT),getValue(POI_EDT_LON),
        editWph->text().isEmpty()?-1:editWph->text().toDouble());
}

void DialogPoi::btSaveWPClicked()
{
    double wph;
    if(editWph->text().isEmpty())
        wph=-1;
    else
        wph=editWph->text().toDouble();
    emit doChgWP(getValue(POI_EDT_LAT),getValue(POI_EDT_LON),wph);
    done(QDialog::Accepted);
}

void DialogPoi::chkTStamp_chg(int state)
{
    editTStamp->setEnabled(state==Qt::Checked);
    editName->setEnabled(state!=Qt::Checked);
}

void DialogPoi::nameHasChanged(QString newName)
{
    setWindowTitle(tr("Marque : ")+newName);
}

void DialogPoi::latLonChg(double /*value*/)
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
void DialogPoi::latLonSignChg(int i)
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


void DialogPoi::type_chg(int index)
{
    if(index==oldType)
        return;
    QString oldStr=POI::getTypeStr(oldType);
    QString editStr=editName->text();
    if(oldStr==editStr.left(oldStr.size()))
        editName->setText(POI::getTypeStr(index)+editStr.right(editStr.size()-oldStr.size()));
    oldType=index;
}


double DialogPoi::getValue(int type)
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

void DialogPoi::setValue(int type,double val)
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
