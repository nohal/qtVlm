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

#include <QtGui>

#include "paramVLM.h"
#include "Util.h"

paramVLM::paramVLM(QWidget * parent) : QDialog(parent)
{
    setupUi(this);

    /* adding list of supported NMEA sentences*/
    chk_activateEmulation->setCheckState(
         Util::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Util::getSetting("serialName", "COM2").toString());
    estimeLen->setValue(Util::getSetting("estimeLen",100).toInt());
    chk_gribZoomOnLoad->setCheckState(Util::getSetting("gribZoomOnLoad",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    chk_forceUserAgent->setCheckState(Util::getSetting("forceUserAgent",0).toInt()==1?Qt::Checked:Qt::Unchecked);
    userAgent->setText(Util::getSetting("userAgent", "").toString());
    userAgent->setEnabled(Util::getSetting("forceUserAgent",0).toInt()==1);
}

void paramVLM::done(int result)
{
    if(result == QDialog::Accepted)
    {
        Util::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("serialName", serialName->text());
        Util::setSetting("estimeLen", QString().setNum(estimeLen->value()));
        Util::setSetting("forceUserAgent",chk_forceUserAgent->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("userAgent",userAgent->text());
        emit paramVLMChanged();
    }
    QDialog::done(result);
}

void paramVLM::forceUserAgent_changed(int newVal)
{
    qWarning("New val %d",newVal);
    userAgent->setEnabled(newVal==Qt::Checked);
}
