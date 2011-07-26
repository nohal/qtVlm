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

#include <QDebug>
#include <QFile>
#include <QDir>

#include "DialogRealBoatConfig.h"

#include "settings.h"
#include "boatReal.h"
#include "Player.h"
#include "Util.h"

DialogRealBoatConfig::DialogRealBoatConfig(myCentralWidget *parent) : QDialog(parent)
{
    this->parent=parent;
    setupUi(this);
    Util::setFontDialog(this);
}

void DialogRealBoatConfig::launch(boatReal * boat)
{
    curBoat=boat;
    if(curBoat)
        curBoat->stopRead();
    /* init dialog */
    serialName->setText(Settings::getSetting("gpsPortName","COM1").toString());
    baudRate->setCurrentIndex(Settings::getSetting("gpsBaudRate",BAUD4800).toInt());
    this->displayNMEA->setChecked(boat->getDisplayNMEA());
    QDir polarDir = QDir("polar");
    QStringList extStr;
    extStr.append("*.pol");
    extStr.append("*.POL");
    extStr.append("*.csv");
    extStr.append("*.CSV");
    QFileInfoList fileList=polarDir.entryInfoList(extStr,QDir::Files);

    /* default value */
    polarList->addItem(tr("<Aucun>"));
    if(!fileList.isEmpty())
    {
        QListIterator<QFileInfo> j (fileList);

        while(j.hasNext())
        {
            QFileInfo finfo = j.next();
            polarList->addItem(finfo.fileName());
        }
    }

    if(curBoat)
    {
        QString polarStr=curBoat->getPolarName();
        polarList->setCurrentIndex(polarStr.isEmpty()?0:polarList->findText(polarStr));
        //qWarning() << "Init dialog with polar: " << polarStr;
    }

    exec();
}

void DialogRealBoatConfig::done(int result)
{
    if(result == QDialog::Accepted)
    {
        Settings::setSetting("gpsPortName",serialName->text());
        Settings::setSetting("gpsBaudRate",QString().setNum(baudRate->currentIndex()));
        if(curBoat)
        {
            //qWarning() << "Saving polar in boat: " << polarList->currentText();

            curBoat->setPolar(polarList->currentIndex()==0?QString():polarList->currentText());
        }
    }
    curBoat->setDisplayNMEA(this->displayNMEA->isChecked());
    QDialog::done(result);
}

