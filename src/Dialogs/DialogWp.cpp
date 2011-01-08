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

#include "DialogWp.h"

#include "boatVLM.h"
#include "POI.h"
#include "Util.h"


/************************/
/* Dialog WP            */

DialogWp::DialogWp(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    currentBoat=NULL;

    WP_conv_lat->setText("");
    WP_conv_lon->setText("");
}

void DialogWp::show_WPdialog(boatVLM * boat)
{
    currentBoat=boat;

    initDialog(boat->getWPLat(),boat->getWPLon(),boat->getWPHd());

    exec();
}

void DialogWp::show_WPdialog(POI * poi)
{
    if(poi)
        initDialog(poi->getLatitude(),poi->getLongitude(),poi->getWph());
    exec();
}

void DialogWp::initDialog(float WPLat,float WPLon,float WPHd)
{
    if(WPLat == 0 && WPLon == 0)
    {
        WP_lat->setText("");
        WP_lon->setText("");
        WP_heading->setText("");
    }
    else
    {
        WP_lat->setText(QString().setNum(WPLat));
        WP_lon->setText(QString().setNum(WPLon));
        if(WPHd==-1)
            WP_heading->setText("");
        else
            WP_heading->setText(QString().setNum(WPHd));
    }
}

void DialogWp::done(int result)
{
    if(result == QDialog::Accepted)
    {
        if(WP_lat->text().isEmpty() && WP_lon->text().isEmpty())
            confirmAndSendCmd(tr("Confirmer le changement du WP"),tr("WP change"),VLM_CMD_WP,0,0,-1);
        else
            confirmAndSendCmd(tr("Confirmer le changement du WP"),tr("WP change"),
                              VLM_CMD_WP,WP_lat->text().toFloat(),WP_lon->text().toFloat(),
                    WP_heading->text().isEmpty()?-1:WP_heading->text().toFloat());
    }
    QDialog::done(result);
}

void DialogWp::doClearWP()
{
    WP_lat->setText("");
    WP_lon->setText("");
    WP_heading->setText("");
}

void DialogWp::chgLat()
{
    if(WP_lat->text().isEmpty())
        WP_conv_lat->setText("");
    else
    {
        float val = WP_lat->text().toFloat();
        WP_conv_lat->setText(Util::pos2String(TYPE_LAT,val));
    }
}

void DialogWp::chgLon()
{
    if(WP_lon->text().isEmpty())
        WP_conv_lon->setText("");
    else
    {
        float val = WP_lon->text().toFloat();
        WP_conv_lon->setText(Util::pos2String(TYPE_LON,val));
    }
}

void DialogWp::doPaste()
{
    float lat,lon,wph;
    if(!currentBoat)
        return;
    if(!Util::getWPClipboard(NULL,&lat,&lon,&wph,NULL)) /*no need to get timestamp*/
        return;
    WP_lat->setText(QString().setNum(lat));
    WP_lon->setText(QString().setNum(lon));
    WP_heading->setText(QString().setNum(wph));
}

void DialogWp::doCopy()
{
    if(!currentBoat)
        return;
    Util::setWPClipboard(WP_lat->text().toFloat(),WP_lon->text().toFloat(),
        WP_heading->text().toFloat());
    QDialog::done(QDialog::Rejected);

}

void DialogWp::doSelPOI()
{
    emit selectPOI();
    QDialog::done(QDialog::Rejected);
}
