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
#ifdef QT_V5
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif
#include "Util.h"
#include "DialogPoiDelete.h"
#include "settings.h"

DialogPoiDelete::DialogPoiDelete(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
    Util::setFontDialog(this);
    QMap<QWidget *,QFont> exceptions;
    QFont wfont=QApplication::font();
    wfont.setBold(true);
    exceptions.insert(label,wfont);
    wfont=QApplication::font();
    wfont.setUnderline(true);
    exceptions.insert(lnk_all,wfont);
    exceptions.insert(lnk_none,wfont);
    Util::setSpecificFont(exceptions);
    mask=-1;
}

void DialogPoiDelete::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    Settings::setSetting(this->objectName()+".positionx",this->pos().x());
    Settings::setSetting(this->objectName()+".positiony",this->pos().y());
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

void DialogPoiDelete::do_chkAll(QString)
{
    chk_POI->setCheckState(Qt::Checked);
    chk_WP->setCheckState(Qt::Checked);
    chk_Balise->setCheckState(Qt::Checked);
}

void DialogPoiDelete::do_chkNone(QString)
{
    chk_POI->setCheckState(Qt::Unchecked);
    chk_WP->setCheckState(Qt::Unchecked);
    chk_Balise->setCheckState(Qt::Unchecked);
}
