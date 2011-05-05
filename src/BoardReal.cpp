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

#include <QMessageBox>
#include <QMenu>
#include <QDebug>

#include "MainWindow.h"

#include "BoardReal.h"
#include "Board.h"
#include "settings.h"

#include "boatReal.h"

#include "dataDef.h"
#include "Util.h"

boardReal::boardReal(MainWindow * mainWin, board * parent) : QWidget(mainWin)
{
    setupUi(this);
    Util::setFontDialog(this);
    this->mainWin=mainWin;
    this->parent=parent;

    /* Contextual Menu */
    popup = new QMenu(this);
    ac_showHideCompass = new QAction(tr("Cacher compas"),popup);
    popup->addAction(ac_showHideCompass);
    connect(ac_showHideCompass,SIGNAL(triggered()),this,SLOT(slot_hideShowCompass()));

//    /* Etat du compass */
//    if(Settings::getSetting("boardCompassShown", "1").toInt()==1)
//        windAngle->show();
//    else
//        windAngle->hide();
}

boatReal * boardReal::currentBoat(void)
{
    if(parent && parent->currentBoat())
    {
        if(parent->currentBoat()->getType()==BOAT_REAL)
            return (boatReal*)parent->currentBoat();
        else
            return NULL;
    }
    else
        return NULL;
}

void boardReal::setWp(float lat,float lon,float wph)
{
    boatReal * myBoat=currentBoat();
    if(myBoat)
        myBoat->setWp(lat,lon,wph);
}
void boardReal::boatUpdated(void)
{
    boatReal * myBoat=currentBoat();

    if(!myBoat)
    {
        qWarning() << "No current real boat";
        return;
    }
    /* boat position */
    latitude->setText(Util::pos2String(TYPE_LAT,myBoat->getLat()));
    longitude->setText(Util::pos2String(TYPE_LON,myBoat->getLon()));

    /* boat heading */
    //windAngle->setValues(myBoat->getHeading(),0,myBoat->getWindSpeed(), -1, -1);
    this->dir->display(myBoat->getHeading());

    /* boat speed*/
    this->speed->display(myBoat->getSpeed());

    /*WP*/
    if(myBoat->getWPLat()!=0 && myBoat->getWPLat()!=0)
    {
        dnm_2->setText(QString().setNum(myBoat->getDnm()));
        ortho->setText(QString().setNum(myBoat->getOrtho()));
        vmg_2->setText(QString().setNum(myBoat->getVmg()));
        angle->setText(QString().setNum(myBoat->getLoxo()));
    }
    else
    {
        dnm_2->setText("---");
        ortho->setText("---");
        vmg_2->setText("---");
        angle->setText("---");
    }

    /* GPS status */
    QString status;
    if(myBoat->gpsIsRunning())
    {
        status="running\n";
        if(myBoat->getSig()==0)
            status=status+"no signal\n";
        else if(myBoat->getSig()==1)
            status=status+"Fix qualitys\n";
        else if (myBoat->getSig()==2)
            status=status+"Differential quality\n";
        else if (myBoat->getSig()==3)
            status=status+"Sensitive quality\n";
        if(myBoat->getFix()==1)
            status=status+"no position";
        else if(myBoat->getFix()==2)
            status=status+"2D position";
        else
            status=status+"3D position";
        if(myBoat->getFix()>1)
        {
            status=status+"\n";
            if(myBoat->getPdop()<=1)
                status=status+"Ideal accuracy";
            else if(myBoat->getPdop()<=2)
                status=status+"Excellent accuracy";
            else if(myBoat->getPdop()<=1)
                status=status+"Good accuracy";
            else if(myBoat->getPdop()<=5)
                status=status+"Moderate accuracy";
            else if(myBoat->getPdop()<=20)
                status=status+"Bad accuracy";
            else
                status=status+"Very bad accuracy";
        }
        gpsStatus->setText(status);
        this->startBtn->setEnabled(false);
        this->stopBtn->setEnabled(true);
    }
    else
    {
        gpsStatus->setText("Stopped");
        this->startBtn->setEnabled(true);
        this->stopBtn->setEnabled(false);
    }
}

void boardReal::setChangeStatus(bool /*status*/)
{

}

void boardReal::paramChanged()
{

}

void boardReal::disp_boatInfo()
{
    QMessageBox::information(this,tr("Information"),tr("Bientot des infos ici"));
}

void boardReal::chgBoatPosition(void)
{
    boatReal * myBoat=currentBoat();
    if(myBoat)
        myBoat->slot_chgPos();
}

void boardReal::startGPS(void)
{
    boatReal * myBoat=currentBoat();
    if(!myBoat)
    {
        qWarning() << "No real board to start GPS";
        return;
    }
    myBoat->startRead();
}

 void boardReal::stopGPS(void)
 {
     boatReal * myBoat=currentBoat();
     if(!myBoat)
     {
         qWarning() << "No real board to stop GPS";
         return;
     }
     myBoat->stopRead();
 }

void boardReal::slot_hideShowCompass()
{
    //setCompassVisible(~windAngle->isVisible());
}

void boardReal::setCompassVisible(bool status)
{
//    if(status)
//    {
//        Settings::setSetting("boardCompassShown",1);
//        windAngle->show();
//    }
//    else
//    {
//        Settings::setSetting("boardCompassShown",0);
//        windAngle->hide();
//    }
}

void boardReal::contextMenuEvent(QContextMenuEvent  *)
{
//    if(windAngle->isVisible())
//        ac_showHideCompass->setText(tr("Cacher le compas"));
//    else
//        ac_showHideCompass->setText(tr("Afficher le compas"));
//    popup->exec(QCursor::pos());
}
