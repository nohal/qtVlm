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
    int idx = baudRate->findText(Settings::getSetting("gpsBaudRate",BAUD4800).toString(),Qt::MatchExactly);
    //qWarning() << "idx=" << idx << ", string= " << Settings::getSetting("gpsBaudRate",BAUD4800).toString();
    if(idx==-1) idx=baudRate->findText("4800",Qt::MatchExactly);
    if(idx==-1) idx=0;
    baudRate->setCurrentIndex(idx);
    polarEfficiency->setValue(Settings::getSetting("polarEfficiency",100).toInt());
    this->displayNMEA->setChecked(boat->getDisplayNMEA());
    QDir polarDir = QDir(appFolder.value("polar"));
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
    this->declinaison->setValue(Settings::getSetting("declinaison",0).toDouble());
    this->minSpeedForEngine->setValue(Settings::getSetting("minSpeedForEngine",0).toDouble());
    this->speedWithEngine->setValue(Settings::getSetting("speedWithEngine",6).toDouble());

    exec();
}

void DialogRealBoatConfig::done(int result)
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if(result == QDialog::Accepted)
    {
        Settings::setSetting("gpsPortName",serialName->text());
        //qWarning() << "Set rate=" << baudRate->currentText();
        Settings::setSetting("gpsBaudRate",baudRate->currentText());
        Settings::setSetting("polarEfficiency",polarEfficiency->value());
        Settings::setSetting("minSpeedForEngine",minSpeedForEngine->value());
        Settings::setSetting("speedWithEngine",speedWithEngine->value());
        if(curBoat)
        {
            qWarning() << "Saving polar in boat: " << polarList->currentText();

            curBoat->setPolar(polarList->currentIndex()==0?QString():polarList->currentText());
        }
        Settings::setSetting("declinaison",QString().sprintf("%.1f",this->declinaison->value()));
        curBoat->setDisplayNMEA(this->displayNMEA->isChecked());
        curBoat->setDeclinaison(declinaison->value());
        if(!curBoat->gpsIsRunning())
            curBoat->emitMoveBoat();
        curBoat->setMinSpeedForEngine(minSpeedForEngine->value());
        curBoat->setSpeedWithEngine(speedWithEngine->value());
        this->parent->emitUpdateRoute(curBoat);
    }
    QDialog::done(result);
}

