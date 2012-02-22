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

DialogRealBoatPosition::DialogRealBoatPosition(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    lat->setText(QString());
    lon->setText(QString());
}

void DialogRealBoatPosition::showDialog(boatReal * boat)
{
    if(!boat) return;

    currentBoat=boat;

    lat->setText(QString().setNum(boat->getLat()));
    lon->setText(QString().setNum(boat->getLon()));

    exec();
}

void DialogRealBoatPosition::done(int result)
{
    if(result == QDialog::Accepted)
    {
        double boatLat,boatLon;

        if(lat->text().isEmpty()) boatLat=0;
        else boatLat=lat->text().toDouble();
        if(lon->text().isEmpty()) boatLon=0;
        else boatLon=lon->text().toDouble();

        currentBoat->setPosition(boatLat,boatLon);
        currentBoat->emitMoveBoat();
    }
    QDialog::done(result);
}

void DialogRealBoatPosition::latChanged(QString txt)
{
    if(txt.isEmpty())
        conv_lat->setText("");
    else
    {
        double val = txt.toDouble();
        conv_lat->setText(Util::pos2String(TYPE_LAT,val));
    }
}

void DialogRealBoatPosition::lonChanged(QString txt)
{
    if(txt.isEmpty())
        conv_lon->setText("");
    else
    {
        double val = txt.toDouble();
        conv_lon->setText(Util::pos2String(TYPE_LON,val));
    }
}
