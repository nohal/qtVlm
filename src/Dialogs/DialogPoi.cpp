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
#include <QMessageBox>
#include <QDebug>

class POI_Editor;

#include "DialogPoi.h"
#include "Util.h"
#include "MainWindow.h"
#include "boatVLM.h"
#include <QDesktopWidget>

#define POI_EDT_LAT 1
#define POI_EDT_LON 2

//-------------------------------------------------------
// POI_Editor: Constructor for edit an existing POI
//-------------------------------------------------------
DialogPoi::DialogPoi(MainWindow * main,myCentralWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    this->resize(widget->width()+10,widget->height()+10);
//    int minSize=qMax(this->lat_sig->width(),this->lon_sig->width());
//    lat_sig->setMinimumWidth(minSize);
//    lon_sig->setMinimumWidth(minSize);
    widget->setParent(0);
    scroll=new QScrollArea(this);
    scroll->resize(widget->size());
    scroll->setWidget(widget);
    QSize mySize=QSize(widget->size().width()+20,widget->size().height()+20);
    QSize screenSize=QApplication::desktop()->screenGeometry().size()*.8;
    if(mySize.height() > screenSize.height())
    {
        mySize.setHeight(screenSize.height());
    }
    if(mySize.width() > screenSize.width())
    {
        mySize.setWidth(screenSize.width());
    }
    this->resize(mySize);
    scroll->resize(mySize);
    this->poi = NULL;
    this->parent=parent;
    this->main=main;

    lock=true;
    modeCreation=false;

    connect(this,SIGNAL(addPOI_list(POI*)),parent,SLOT(slot_addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),parent,SLOT(slot_delPOI_list(POI*)));

    connect(main, SIGNAL(newPOI(float,float,Projection *, boat *)),
            this, SLOT(newPOI(float,float,Projection *, boat*)));
    connect(this,SIGNAL(doChgWP(float,float,float)),main,SLOT(slotChgWP(float,float,float)));
}
void DialogPoi::resizeEvent ( QResizeEvent * /*event*/ )
{
    scroll->resize(this->size());
}

void DialogPoi::editPOI(POI * poi_)
{
    //=> set name
    modeCreation = false;
    this->poi = poi_;
    initPOI();
    setWindowTitle(tr("Marque : ")+poi->getName());
    btDelete->setEnabled(true);

    exec();
}

void DialogPoi::newPOI(float lon, float lat,Projection *proj, boat *boat)
{
    //=> set name
    modeCreation = true;
    this->poi = new POI(tr("POI"), POI_TYPE_POI,lat, lon, proj, main,
                        parent, -1,-1,false, boat);
    initPOI();
    setWindowTitle(tr("Nouvelle marque"));
    btDelete->setEnabled(false);
    exec();
}

void DialogPoi::initPOI(void)
{
    editName->setText(poi->getName());
    oldType=poi->getType();
    POI_type_liste->setCurrentIndex(oldType);
    QListIterator<ROUTE*> i (parent->getRouteList());
    cb_routeList->clear();
    cb_routeList->addItem("Not in a route");
    while(i.hasNext())
        cb_routeList->addItem(i.next()->getName());
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
}

//---------------------------------------
void DialogPoi::done(int result)
{
    if(result == QDialog::Accepted)
    {
        poi->setPartOfTwa(false);
        QDateTime tm = editTStamp->dateTime();
        poi->setNotSimplificable(this->notSimplicable->checkState()==Qt::Checked);
        tm.setTimeSpec(Qt::UTC);
        poi->setTimeStamp(tm.toTime_t());
        poi->setUseTimeStamp(chk_tstamp->checkState()==Qt::Checked);

        poi->setLongitude(getValue(POI_EDT_LON));
        poi->setLatitude (getValue(POI_EDT_LAT));
 //       int oldtype=poi->getType();
        poi->setType(POI_type_liste->currentIndex());
        poi->setName((editName->text()).trimmed() );
        if(editWph->text().isEmpty())
            poi->setWph(-1);
        else
            poi->setWph(editWph->text().toFloat());
        poi->slot_updateProjection();

        if (modeCreation) {
            //poi->show();
            emit addPOI_list(poi);
        }
        if(cb_routeList->currentIndex()!=0)
        {
            ROUTE * route=parent->getRouteList()[cb_routeList->currentIndex()-1];
            if(!route->getFrozen())
                poi->setRoute(route);
        }
        else
            poi->setRoute(NULL);
        if(poi->getIsWp()) emit doChgWP(poi->getLatitude(),poi->getLongitude(),poi->getWph());


    }

    if(result == QDialog::Rejected)
    {
        if (modeCreation)
                delete poi;
    }
    QDialog::done(result);
}

//---------------------------------------
void DialogPoi::btDeleteClicked()
{
    if (! modeCreation) {
        int rep = QMessageBox::question (this,
            tr("Detruire la marque : %1").arg(poi->getName()),
            tr("La destruction d'une marque est definitive.\n\nEtes-vous sur ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes)
        {
            emit delPOI_list(poi);
            poi->close();
            QDialog::done(QDialog::Accepted);
        }
    }
}

void DialogPoi::btPasteClicked()
{
    float lat,lon,wph;
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
        editWph->text().isEmpty()?-1:editWph->text().toFloat());
}

void DialogPoi::btSaveWPClicked()
{
    float wph;
    if(editWph->text().isEmpty())
        wph=-1;
    else
        wph=editWph->text().toFloat();
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
    double a,b,c;
    int s;
    if(this->lat_deg->hasFocus() || this->lat_min->hasFocus())
    {
        a=lat_deg->value();
        b=lat_min->value();
        s=lat_sig->currentIndex();
        c=a+b/60;
        if(s==1) c=-c;
        lat_val->blockSignals(true);
        lat_val->setValue(c);
        lat_val->blockSignals(false);
    }
    else if(this->lon_deg->hasFocus() || this->lon_min->hasFocus())
    {
        a=lon_deg->value();
        b=lon_min->value();
        s=lon_sig->currentIndex();
        c=a+b/60;
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
        b=(int) trunc(a);
        c=60.0*fabs(a-trunc(a));
        lat_deg->blockSignals(true);
        lat_min->blockSignals(true);
        lat_sig->blockSignals(true);
        lat_deg->setValue(b);
        lat_min->setValue(c);
        lat_sig->setCurrentIndex(s);
        lat_deg->blockSignals(false);
        lat_min->blockSignals(false);
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
        b=(int) trunc(a);
        c=60.0*fabs(a-trunc(a));
        lon_deg->blockSignals(true);
        lon_min->blockSignals(true);
        lon_sig->blockSignals(true);
        lon_deg->setValue(b);
        lon_min->setValue(c);
        lon_sig->setCurrentIndex(s);
        lon_deg->blockSignals(false);
        lon_min->blockSignals(false);
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


float DialogPoi::getValue(int type)
{
    float res;
    float deg = (type==POI_EDT_LAT?lat_deg->value():lon_deg->value());
    float min = (type==POI_EDT_LAT?lat_min->value():lon_min->value())/60.0;
    float sig;
    if(type==POI_EDT_LAT)
        sig=lat_sig->currentIndex()==0?1.0:-1.0;
    else
        sig=lon_sig->currentIndex()==0?1.0:-1.0;
    res=sig*(deg+min);
    return res;
}

void DialogPoi::setValue(int type,float val)
{
    int sig=val<0?1:0;
    val=fabs(val);
    int   deg = (int) trunc(val);
    float min = 60.0*fabs(val-trunc(val));
    if(type==POI_EDT_LAT)
    {
        lat_deg->setValue(deg);
        lat_min->setValue(min);
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
        lon_sig->setCurrentIndex(sig);
        if(sig==1)
            lon_val->setValue(-val);
        else
            lon_val->setValue(val);
    }
}
