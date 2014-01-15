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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/
#ifdef QT_V5
#include <QtWidgets/QMessageBox>
#else
#include <QMessageBox>
#endif
#include <cmath>
#include <cassert>

#include "DialogUnits.h"
#include "settings.h"
#include "Util.h"


//-------------------------------------------------------------------------------
DialogUnits::DialogUnits() : QDialog()
{
    setWindowTitle(tr("Unites"));
    QFrame *ftmp;
    QLabel *label;
    frameGui = createFrameGui(this);
    
    layout = new QGridLayout(this);
    int lig=0;
    //-------------------------
    lig ++;
    QFont font;
    font.setBold(true);
    label = new QLabel(tr("Unites"), this);
    label->setFont(font);
    layout->addWidget( label,    lig,0, 1,-1, Qt::AlignCenter);
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    layout->addWidget( frameGui,  lig,0,   1, 2);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); layout->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    btOK     = new QPushButton(tr("Valider"), this);
    btCancel = new QPushButton(tr("Annuler"), this);
    layout->addWidget( btOK,    lig,0);
    layout->addWidget( btCancel, lig,1);
    Util::setFontDialog(this);
    Util::setFontDialog(ftmp);

    //===============================================================
    // restaure les valeurs enregistrées dans les settings
    QString tunit;
    int ind;


    tunit = Settings::getSetting(unitsPosition).toString();
    ind = (tunit=="") ? 0 : cbPositionUnit->findData(tunit);
    cbPositionUnit->setCurrentIndex( ind );
    
    tunit = Settings::getSetting(unitsDistance).toString();
    ind = (tunit=="") ? 0 : cbDistanceUnit->findData(tunit);
    cbDistanceUnit->setCurrentIndex( ind );
    
    tunit = Settings::getSetting(unitsLongitude).toString();
    ind = (tunit=="") ? 0 : cbLongitude->findData(tunit);
    cbLongitude->setCurrentIndex( ind );
    
    tunit = Settings::getSetting(unitsLatitude).toString();
    ind = (tunit=="") ? 0 : cbLatitude->findData(tunit);
    cbLatitude->setCurrentIndex( ind );
    
    //===============================================================
    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
}

//-------------------------------------------------------------------------------
void DialogUnits::slotBtOK()
{
    Settings::saveGeometry(this);
    QComboBox *cb;

    cb = cbPositionUnit;
    Settings::setSetting(unitsPosition,  cb->itemData(cb->currentIndex()).toString());
    cb = cbDistanceUnit;
    Settings::setSetting(unitsDistance,  cb->itemData(cb->currentIndex()).toString());

    cb = cbLongitude;
    Settings::setSetting(unitsLongitude,  cb->itemData(cb->currentIndex()).toString());
    cb = cbLatitude;
    Settings::setSetting(unitsLatitude,  cb->itemData(cb->currentIndex()).toString());
    accept();
}
//-------------------------------------------------------------------------------
void DialogUnits::slotBtCancel()
{
    Settings::saveGeometry(this);
    reject();
}

//=============================================================================
// GUI
//=============================================================================
QFrame *DialogUnits::createFrameGui(QWidget *parent)
{
    QFrame * frm = new QFrame(parent);
    QLabel * label;
    QGridLayout  *lay = new QGridLayout(frm);
    int lig=0;
    int sizemin = 160;
    //-------------------------
    lig ++;
    label = new QLabel(tr("Vitesse du vent :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
//    cbWindSpeedUnit = new QComboBox(this);
//    cbWindSpeedUnit->addItem( tr("m/s") ,  "m/s");
//    cbWindSpeedUnit->addItem( tr("km/h"),  "km/h");
//    cbWindSpeedUnit->addItem( tr("noeuds"), "noeuds");
//    cbWindSpeedUnit->setMinimumWidth (sizemin);
//    lay->addWidget( cbWindSpeedUnit, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Distances :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    cbDistanceUnit = new QComboBox(this);
    cbDistanceUnit->addItem( tr("mille marin"), "mille marin");
    cbDistanceUnit->addItem( tr("km"), "km");
    cbDistanceUnit->setMinimumWidth (sizemin);
    lay->addWidget( cbDistanceUnit, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Coordonnees :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    cbPositionUnit = new QComboBox(this);
    cbPositionUnit->addItem( tr("dddegmm'ss\""), "dddegmm'ss");
    cbPositionUnit->addItem( tr("dddegmm,mm'"), "dddegmm,mm'");
    cbPositionUnit->addItem( tr("dd,dddeg"), "dd,dddeg");
    cbPositionUnit->setMinimumWidth (sizemin);
    lay->addWidget( cbPositionUnit, lig,1, Qt::AlignLeft);
    
    //-------------------------
    lig ++;
    label = new QLabel(tr("Longitudes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    cbLongitude = new QComboBox(this);
    cbLongitude->addItem( tr("Auto"), "Auto");
    cbLongitude->addItem( tr("Est positive"), "Est positive");
    cbLongitude->addItem( tr("Ouest positive"), "Ouest positive");
    cbLongitude->setMinimumWidth (sizemin);
    lay->addWidget( cbLongitude, lig,1, Qt::AlignLeft);
    //-------------------------
    lig ++;
    label = new QLabel(tr("Latitudes :"), frm);
    lay->addWidget( label,    lig,0, Qt::AlignRight);
    cbLatitude = new QComboBox(this);
    cbLatitude->addItem( tr("Auto"), "Auto");
    cbLatitude->addItem( tr("Nord positive"), "Nord positive");
    cbLatitude->addItem( tr("Sud positive"), "Sud positive");
    cbLatitude->setMinimumWidth (sizemin);
    lay->addWidget( cbLatitude, lig,1, Qt::AlignLeft);
    
    return frm;
}











