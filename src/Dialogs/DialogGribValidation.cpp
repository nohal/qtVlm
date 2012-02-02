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

#include <QDebug>
#include <QDateTime>

#include "DialogGribValidation.h"
#include "Grib.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "Util.h"
#include "dataDef.h"
extern int nbWarning;
DialogGribValidation::DialogGribValidation(myCentralWidget * my_centralWidget,MainWindow * mainWindow) :  QDialog(my_centralWidget)
{
    setupUi(this);
    Util::setFontDialog(this);
    this->my_centralWidget=my_centralWidget;
    this->mainWindow=mainWindow;
    tstamp->setText(QString().setNum(my_centralWidget->getGrib()->getCurrentDate()));
    this->label_date->setText(QDateTime().fromTime_t(tstamp->text().toInt()).toUTC().toString("dd MMM-hh:mm:ss"));
    latitude->setText("0");
    longitude->setText("0");
    this->latitude->blockSignals(true);
}

DialogGribValidation::~DialogGribValidation()
{

}

void DialogGribValidation::setMode(int mode)
{
    type->setCurrentIndex(mode-1);
    curMode=mode;
}

void DialogGribValidation::done(int result)
{
    int newMode;
    if(result == QDialog::Accepted)
        newMode=this->type->currentIndex()+1;
    else
        newMode=curMode;

    if(this->my_centralWidget->getGrib())
    {
        this->my_centralWidget->getGrib()->setInterpolationMode(newMode);
        qWarning() << "Setting interpolation mode to " << newMode;
    }

    hide();
    this->my_centralWidget->send_redrawAll();
    //QDialog::done(result);
}

void DialogGribValidation::doNow(void)
{
    tstamp->setText(QString().setNum(QDateTime::currentDateTime().toUTC().toTime_t()));
}

void DialogGribValidation::interpolationChanged(int newMode)
{
    if(this->my_centralWidget->getGrib())
    {
        newMode++;
        this->my_centralWidget->getGrib()->setInterpolationMode(newMode);
        qWarning() << "Setting interpolation mode to " << newMode;
    }

    this->my_centralWidget->send_redrawAll();
    inputChanged();
}

void DialogGribValidation::inputChanged(void)
{
    nbWarning=0;
    /* recompute interpolation */
    double lat,lon;
    int tstamp;
    bool ok;
    double vit,ang;

    lat=this->latitude->text().toDouble(&ok);
    if(!ok)
    {
        this->vitesse->setText("Err lat");
        return;
    }
    lon=this->longitude->text().toDouble(&ok);
    if(!ok)
    {
        this->vitesse->setText("Err lon");
        return;
    }
    tstamp=this->tstamp->text().toInt(&ok);
    if(!ok)
    {
        this->vitesse->setText("Err tstamp");
        this->label_date->setText("Err tstamp");
        return;
    }
    this->label_date->setText(QDateTime().fromTime_t(this->tstamp->text().toInt()).toUTC().toString("dd MMM-hh:mm:ss"));
    //qWarning() << "Get new param: " << lat << "," << lon << " - " << tstamp;

    if(!this->my_centralWidget->getGrib())
    {
        this->vitesse->setText("No grib");
        return;
    }

    ok=this->my_centralWidget->getGrib()->getInterpolatedValue_byDates(lon,lat,tstamp,&vit,&ang,
                                                      type->currentIndex()+1,this->chk_debug->checkState()==Qt::Checked);
    //qWarning() << "Interpolation: v=" << vit << ", ang=" << ang << "(ok=" << ok << ")";

    vitesse->setText(QString().setNum(vit));
    angle->setText(QString().setNum(radToDeg(ang)));

}
