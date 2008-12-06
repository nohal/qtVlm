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
    lst_trame->addItem("GLL");

    chk_activateEmulation->setCheckState(
         Util::getSetting("gpsEmulEnable", "0").toString()=="1"?Qt::Checked:Qt::Unchecked);
    serialName->setText(Util::getSetting("serialName", "COM2").toString());
    lst_trame->setCurrentIndex(lst_trame->findText(Util::getSetting("nmeaType", "GLL").toString()));
}

void paramVLM::done(int result)
{
    if(result == QDialog::Accepted)
    {
        Util::setSetting("gpsEmulEnable",chk_activateEmulation->checkState()==Qt::Checked?"1":"0");
        Util::setSetting("serialName", serialName->text());
        Util::setSetting("nmeaType",lst_trame->currentText());
        emit paramVLMChanged();
    }
    QDialog::done(result);
}
