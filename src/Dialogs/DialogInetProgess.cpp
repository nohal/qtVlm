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

#include "DialogInetProgess.h"

/*********************************************
  Progress bar
  *******************************************/

DialogInetProgess::DialogInetProgess(QWidget * parent) : QDialog(parent)
{
    setupUi(this);
}

void DialogInetProgess::showDialog(QString name)
{
    fileName->setText(name);
    progress->reset();
    progress_txt->setText("0/0 Kb");
    setWindowModality(Qt::ApplicationModal);
    show();
}
void DialogInetProgess::hideDialog()
{
    if(this->isVisible()) hide();
}

void DialogInetProgess::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QString type;
    QString str;
    double received,total;
    if(bytesTotal<1024)
    {
        type="b";
        received=((double)bytesReceived);
        total=((double)bytesTotal);
    }
    else if(bytesTotal<1024*1024)
    {
        type="Kb";
        received=((double)bytesReceived)/1024.0;
        total=((double)bytesTotal)/1024.0;
    }
    else
    {
        type="Mb";
        received=((double)bytesReceived)/1024.0/1024.0;
        total=((double)bytesTotal)/1024.0/1024.0;
    }

    progress->setMinimum(0);
    progress->setMaximum((int)bytesTotal);
    progress->setValue((int)bytesReceived);
    str.sprintf("%.2f/%.2f ",received,total);
    progress_txt->setText(str+type);
}
