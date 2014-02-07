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
#include <QFileDialog>

#include "DialogRealBoatConfig.h"

#include "settings.h"
#include "boatReal.h"
#include "GpsReceiver.h"
#include "Player.h"
#include "Util.h"
#include <QScroller>

DialogRealBoatConfig::DialogRealBoatConfig(myCentralWidget *parent) : QDialog(parent)
{
    this->parent=parent;
    connect(parent,SIGNAL(geometryChanged()),this,SLOT(slot_screenResize()));
    setupUi(this);
    QScroller::grabGesture(this->scrollArea->viewport());
    Util::setFontDialog(this);
}
void DialogRealBoatConfig::slot_screenResize()
{
    Util::setWidgetSize(this,this->sizeHint());
}

void DialogRealBoatConfig::launch(boatReal * boat)
{
    curBoat=boat;
    if(!curBoat) return;
    /* init dialog */

    //*** GPS ****

    // Serial part
    serialName->setText(Settings::getSetting(gpsSerial_portName).toString());
    int idx = baudRate->findText(Settings::getSetting(gpsDerial_baudRate).toString(),Qt::MatchExactly);
    if(idx==-1) idx=baudRate->findText("4800",Qt::MatchExactly);
    if(idx==-1) idx=0;
    baudRate->setCurrentIndex(idx);

    // File part
    gpsFileName=Settings::getSetting(gpsFake_fileName).toString();
    lb_fileName->setText(gpsFileName);

    // GPSd part

    // GPS source
    // first disable all GPS widget - the needed one will be activate by setCheck
    serialName->setEnabled(false);
    baudRate->setEnabled(false);
    btnFile->setEnabled(false);

#ifndef __UNIX_QTVLM
    rdGPSd->setChecked(false);
#endif

    gpsSource = Settings::getSetting(deviceType).toInt();
    switch(gpsSource) {
        case GPS_GPSD:
#ifdef __UNIX_QTVLM
        rdGPSd->setChecked(true);
        break;
#endif
        case GPS_SERIAL:
            rdSerial->setChecked(true);
            break;
        case GPS_FILE:
            rdFile->setChecked(true);
            break;
    }

    this->displayNMEA->setChecked(boat->getDisplayNMEA());

    //*** Polar ****
    QDir polarDir = QDir(appFolder.value("polar"));
    QStringList extStr;
    extStr.append("*.pol");
    extStr.append("*.POL");
    extStr.append("*.csv");
    extStr.append("*.CSV");
    extStr.append("*.xml");
    extStr.append("*.XML");
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
    }

    spn_polarEfficiency->setValue(Settings::getSetting(polar_efficiency).toInt());

    this->declinaison->setValue(Settings::getSetting(boat_declinaison).toDouble());
    this->minSpeedForEngine->setValue(Settings::getSetting(boat_minSpeedForEngine).toDouble());
    this->speedWithEngine->setValue(Settings::getSetting(boat_speedWithEngine).toDouble());

    exec();
}

void DialogRealBoatConfig::done(int result)
{
    Settings::saveGeometry(this);
    if(result == QDialog::Accepted)
    {
        /* GPS settings */
        // Serial
        Settings::setSetting(gpsSerial_portName,serialName->text());
        Settings::setSetting(gpsDerial_baudRate,baudRate->currentText());

        // File
        Settings::setSetting(gpsFake_fileName,gpsFileName);
        // GPSd

        // GPS mode
        Settings::setSetting(deviceType,gpsSource);

        curBoat->setDisplayNMEA(this->displayNMEA->isChecked());

        if(curBoat->gpsIsRunning())
            curBoat->restartGPS();

        Settings::setSetting(polar_efficiency,spn_polarEfficiency->value());
        Settings::setSetting(boat_minSpeedForEngine,minSpeedForEngine->value());
        Settings::setSetting(boat_speedWithEngine,speedWithEngine->value());
        if(curBoat)
        {
            qWarning() << "Saving polar in boat: " << polarList->currentText();

            curBoat->setPolar(polarList->currentIndex()==0?QString():polarList->currentText());
        }
        Settings::setSetting(boat_declinaison,QString().sprintf("%.1f",this->declinaison->value()));

        curBoat->setDeclinaison(declinaison->value());

        curBoat->setMinSpeedForEngine(minSpeedForEngine->value());
        curBoat->setSpeedWithEngine(speedWithEngine->value());
        this->parent->emitUpdateRoute(curBoat);
    }
    QDialog::done(result);
}

void DialogRealBoatConfig::slot_selectFile(void) {
    QString fileName = QFileDialog::getOpenFileName(this,
                         tr("Choisir un fichier"));
    if (fileName != "")
    {
        gpsFileName=fileName;
        lb_fileName->setText(fileName);
    }
}

void DialogRealBoatConfig::slot_serial(bool st) {
    if(st) {
        serialName->setEnabled(true);
        baudRate->setEnabled(true);
        gpsSource=GPS_SERIAL;
    }
    else {
        serialName->setEnabled(false);
        baudRate->setEnabled(false);
    }

}

void DialogRealBoatConfig::slot_file(bool st) {
    if(st) {
        btnFile->setEnabled(true);
        gpsSource=GPS_FILE;
    }
    else {
        btnFile->setEnabled(false);
    }
}

void DialogRealBoatConfig::slot_gpsd(bool st) {
    if(st) {
        gpsSource=GPS_GPSD;
    }
}

