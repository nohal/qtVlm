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

#include "POI_editor.h"
#include "Util.h"
#include "MainWindow.h"

#define POI_EDT_LAT 1
#define POI_EDT_LON 2

//-------------------------------------------------------
// POI_Editor: Constructor for edit an existing POI
//-------------------------------------------------------
POI_Editor::POI_Editor(QWidget *ownerMeteotable, QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    this->poi = NULL;
    this->parent=parent;
    this->ownerMeteotable=ownerMeteotable;
    modeCreation=false;

    connect(this,SIGNAL(addPOI_list(POI*)),ownerMeteotable,SLOT(addPOI_list(POI*)));
    connect(this,SIGNAL(delPOI_list(POI*)),ownerMeteotable,SLOT(delPOI_list(POI*)));

    connect(ownerMeteotable, SIGNAL(editPOI(POI*)),this, SLOT(editPOI(POI*)));
    connect(ownerMeteotable, SIGNAL(newPOI(float,float,Projection *)),
            this, SLOT(newPOI(float,float,Projection *)));
}

void POI_Editor::editPOI(POI * poi_)
{
    //=> set name
    modeCreation = false;
    this->poi = poi_;
    initPOI();
    setWindowTitle(tr("Point d'Intérêt : ")+poi->getName());
    btDelete->setEnabled(true);
    exec();
}

void POI_Editor::newPOI(float lon, float lat,Projection *proj)
{
    //=> set name
    modeCreation = true;
    this->poi = new POI(tr("POI"), lat, lon, proj, ownerMeteotable,
                        parent, -1,-1,false);
    initPOI();
    setWindowTitle(tr("Nouveau Point d'Intérêt"));
    btDelete->setEnabled(false);
    exec();
}

void POI_Editor::initPOI(void)
{
    editName->setText(poi->getName());

    if(poi->getWph()==-1)
        editWph->setText(QString());
    else
         editWph->setText(QString().setNum(poi->getWph()));

    btSaveWP->setEnabled(!((MainWindow*)ownerMeteotable)->getBoatLockStatus());

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
}

//---------------------------------------
void POI_Editor::done(int result)
{
    if(result == QDialog::Accepted)
    {
        QDateTime tm = editTStamp->dateTime();

        tm.setTimeSpec(Qt::UTC);
        poi->setTimeStamp(tm.toTime_t());
        poi->setUseTimeStamp(chk_tstamp->checkState()==Qt::Checked);

        poi->setName((editName->text()).trimmed() );
        poi->setLongitude(getValue(POI_EDT_LON));
        poi->setLatitude (getValue(POI_EDT_LAT));

        if(editWph->text().isEmpty())
            poi->setWph(-1);
        else
            poi->setWph(editWph->text().toFloat());
        poi->updateProjection();

        if (modeCreation) {
            poi->show();
            emit addPOI_list(poi);
        }
    }

    if(result == QDialog::Rejected)
    {
        if (modeCreation)
                delete poi;
    }
    QDialog::done(result);
}

//---------------------------------------
void POI_Editor::btDeleteClicked()
{
    if (! modeCreation) {
        int rep = QMessageBox::question (this,
            tr("Détruire le POI : %1").arg(poi->getName()),
            tr("La destruction d'un point d'intérêt est définitive.\n\nEtes-vous sûr ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (rep == QMessageBox::Yes)
        {
            delPOI_list(poi);
            poi->close();
            QDialog::done(QDialog::Accepted);
        }
    }
}

void POI_Editor::btPasteClicked()
{
    float lat,lon,wph;
    int tstamp;
    if(!Util::getWPClipboard(&lat,&lon,&wph,&tstamp))
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

    if(wph<0 || wph > 360)
        editWph->setText(QString());
    else
        editWph->setText(QString().setNum(wph));
}

void POI_Editor::btCopyClicked()
{
    Util::setWPClipboard(getValue(POI_EDT_LAT),getValue(POI_EDT_LON),
        editWph->text().isEmpty()?-1:editWph->text().toFloat());
}

void POI_Editor::btSaveWPClicked()
{
    float wph;
    if(editWph->text().isEmpty())
        wph=-1;
    else
        wph=editWph->text().toFloat();
    poi->doChgWP(getValue(POI_EDT_LAT),getValue(POI_EDT_LON),wph);
    done(QDialog::Accepted);
}

void POI_Editor::chkTStamp_chg(int state)
{
    editTStamp->setEnabled(state==Qt::Checked);
    editName->setEnabled(state!=Qt::Checked);
}

void POI_Editor::nameHasChanged(QString newName)
{
    setWindowTitle(tr("Point d'Intérêt : ")+newName);
}

float POI_Editor::getValue(int type)
{
    float deg = (type==POI_EDT_LAT?lat_deg->value():lon_deg->value());
    float min = (type==POI_EDT_LAT?lat_min->value():lon_min->value())/60.0;
    if (deg < 0)
        return deg - min;
    else
        return deg + min;
}

void POI_Editor::setValue(int type,float val)
{
    int   deg = (int) trunc(val);
    float min = 60.0*fabs(val-trunc(val));
    if(type==POI_EDT_LAT)
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
