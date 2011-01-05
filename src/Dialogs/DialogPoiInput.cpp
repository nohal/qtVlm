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

#include "DialogPoiInput.h"
#include "Util.h"
#include "mycentralwidget.h"

DialogPoiInput::DialogPoiInput(myCentralWidget * parent) : QDialog(parent)
{
    setupUi(this);
    connect(this,SIGNAL(addPOI(QString,int,float,float,float,int,bool,boat*)),
            parent,SLOT(slot_addPOI(QString,int,float,float,float,int,bool,boat*)));
}

void DialogPoiInput::slot_showPOI_input(void)
{
    exec();
}

void DialogPoiInput::txtHasChanged(void)
{
    if(POI_list->toPlainText().isEmpty())
    {
        nbValid->setText("0/0 valid WP");
        return;
    }
    
    QStringList lsbuf,lsval1,lsval2,lsval3;
    int nbOk=0;
    lsbuf = POI_list->toPlainText().split("\n");
    for (int i=0; i < lsbuf.size(); i++)
    {
        lsval1 = lsbuf.at(i).split("@");
        if(lsval1.size()==2)
        {
            lsval2 = lsval1[0].split(",");
            lsval3 = lsval1[1].split(",");
            if((lsval2.size()== 2 || lsval2.size()== 3) && (lsval3.size()== 1 || lsval3.size()== 2))
                nbOk++;
        }
    }
    nbValid->setText(QString("%1/%2 valid WP").arg(nbOk).arg(lsbuf.size()));
}


void DialogPoiInput::done(int result)
{
    if(result == QDialog::Accepted)
    {
        QStringList lsbuf,lsval1,lsval2,lsval3;
        QString name;
        float lat,lon,wph;
        int tstamp;
        lsbuf = POI_list->toPlainText().split("\n");
        for (int i=0; i < lsbuf.size(); i++)
        {
            if(Util::convertPOI(lsbuf.at(i),&name,&lat,&lon,&wph,&tstamp,type->currentIndex()))
                emit addPOI(name,type->currentIndex(),lat,lon,wph,tstamp,tstamp!=-1,NULL);
        }
    }
    POI_list->clear();
    QDialog::done(result);
}
