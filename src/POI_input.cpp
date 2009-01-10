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

#include "POI_input.h"
#include "Util.h"

POI_input::POI_input(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
}



void POI_input::txtHasChanged(void)
{
    if(POI_list->toPlainText().isEmpty())
    {
        nbValid->setText("0/0 valid WP");
        return;
    }
    
    QStringList lsbuf,lsval1,lsval2;
    int nbOk=0;
    lsbuf = POI_list->toPlainText().split("\n");
    for (int i=0; i < lsbuf.size(); i++)
    {
        lsval1 = lsbuf.at(i).split(",");
        if(lsval1.size()==2 || lsval1.size()==3)
        {
            lsval2 = lsval1[1].split("@");
            if(lsval2.size()==2)
                nbOk++;
        }
    }
    nbValid->setText(QString("%1/%2 valid WP").arg(nbOk).arg(lsbuf.size()));
}


void POI_input::done(int result)
{
    if(result == QDialog::Accepted)
    {
        QStringList lsbuf,lsval1,lsval2;
        lsbuf = POI_list->toPlainText().split("\n");
        for (int i=0; i < lsbuf.size(); i++)
        {
            lsval1 = lsbuf.at(i).split(",");
            if(lsval1.size()==2 || lsval1.size()==3)
            {
                lsval2 = lsval1[1].split("@");
                if(lsval2.size()==2)
                {
                    if(lsval1.size()==2)
                        emit addPOI(lsval1[0].toFloat(),lsval2[0].toFloat(),lsval2[1].toFloat(),-1,false);
                    else
                        emit addPOI(lsval1[0].toFloat(),lsval2[0].toFloat(),lsval2[1].toFloat(),lsval1[2].toInt(),true);
                }
                    
            }
        }
    }
    POI_list->clear();
    QDialog::done(result);
}
