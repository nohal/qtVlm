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

#include "poi_delete.h"

POI_delete::POI_delete(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
    mask=-1;
}

void POI_delete::done(int result)
{
    if(result == QDialog::Accepted)
    {
        mask=0;
        if(chk_POI->isChecked())
            mask|=1;
        if(chk_WP->isChecked())
            mask|=2;
        if(chk_Balise->isChecked())
            mask|=4;
    }
    else
        mask=-1;

    QDialog::done(result);
}

void POI_delete::do_chkAll(void)
{
    chk_POI->setCheckState(Qt::Checked);
    chk_WP->setCheckState(Qt::Checked);
    chk_Balise->setCheckState(Qt::Checked);
}

void POI_delete::do_chkNone(void)
{
    chk_POI->setCheckState(Qt::Unchecked);
    chk_WP->setCheckState(Qt::Unchecked);
    chk_Balise->setCheckState(Qt::Unchecked);
}
