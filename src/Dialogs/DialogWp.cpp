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
#include "boatReal.h"
#include "POI.h"
#include "Util.h"
#include "settings.h"
#include <QMessageBox>


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
void DialogWp::setLocked(const bool &locked)
{
    this->WP_lat->setEnabled(locked);
    this->WP_lon->setEnabled(locked);
    this->btn_clear->setEnabled(locked);
    this->btn_paste->setEnabled(locked);
    this->btn_selectPOI->setEnabled(locked);
    this->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(locked);
}

void DialogWp::show_WPdialog(boat * boat)
{
    currentBoat=boat;

    initDialog(boat->getWPLat(),boat->getWPLon(),boat->getWPHd());

    exec();
}

void DialogWp::show_WPdialog(POI * poi, boat * boat)
{
    currentBoat=boat;
    if(poi)
        initDialog(poi->getLatitude(),poi->getLongitude(),poi->getWph());
    else
        initDialog(currentBoat->getWPLat(),currentBoat->getWPLon(),currentBoat->getWPHd());
    exec();
}

void DialogWp::initDialog(double WPLat,double WPLon,double WPHd)
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
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    Settings::setSetting(this->objectName()+".positionx",this->pos().x());
    Settings::setSetting(this->objectName()+".positiony",this->pos().y());
    if(result == QDialog::Accepted)
    {
        QPointF pos;
        double wphd;

        if(WP_lat->text().isEmpty() && WP_lon->text().isEmpty()) {
            pos = QPointF(0,0);
            wphd=-1;
        }
        else {
            pos=QPointF(WP_lon->text().toDouble(),WP_lat->text().toDouble());
            wphd=WP_heading->text().isEmpty()?-1:WP_heading->text().toDouble();
        }
        switch(currentBoat->get_boatType()) {
            case BOAT_VLM: {
                boatVLM * ptr=(boatVLM*)currentBoat;
                ptr->setWP(pos,wphd);
                break;
            }
            case BOAT_REAL: {
                boatReal * ptr=(boatReal*)currentBoat;
                ptr->setWP(pos,wphd);
                break;
            }
            default:
                break;
        }
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
        double val = WP_lat->text().toDouble();
        WP_conv_lat->setText(Util::pos2String(TYPE_LAT,val));
    }
}

void DialogWp::chgLon()
{
    if(WP_lon->text().isEmpty())
        WP_conv_lon->setText("");
    else
    {
        double val = WP_lon->text().toDouble();
        WP_conv_lon->setText(Util::pos2String(TYPE_LON,val));
    }
}

void DialogWp::doPaste()
{
    double lat,lon,wph;
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
    if(WP_heading->text().isEmpty())
        Util::setWPClipboard(WP_lat->text().toDouble(),WP_lon->text().toDouble(),
            -1);
    else
        Util::setWPClipboard(WP_lat->text().toDouble(),WP_lon->text().toDouble(),
            WP_heading->text().toDouble());
    QDialog::done(QDialog::Rejected);

}

void DialogWp::doSelPOI()
{
    emit selectPOI();
    QDialog::done(QDialog::Rejected);
}
