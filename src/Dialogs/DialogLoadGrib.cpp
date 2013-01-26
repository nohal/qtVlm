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

#include <QtWidgets/QMessageBox>
#include <cmath>
#include <cassert>

#include "DialogLoadGrib.h"
#include "settings.h"
#include "Util.h"
#include "LoadGribFile.h"



//-------------------------------------------------------------------------------
DialogLoadGrib::DialogLoadGrib() : QDialog()
{
    loadgrib = new LoadGribFile();
    connect (loadgrib,SIGNAL(ungrayButtons()),this,SLOT(slotUngrayButtons()));
    assert(loadgrib);

    setWindowTitle(tr("Telechargement"));
    loadInProgress = false;
    QFrame * frameButtonsZone = createFrameButtonsZone(this);
    Util::setFontDialog(frameButtonsZone);
    rain  = true;
    cloud = true;
    pressure = true;
    wind  = true;
    temp  = true;
    humid = true;
    isotherm0 = true;
    tempPot = true;
    tempMin = true;
    tempMax = true;
    snowDepth = true;
    snowCateg = true;
    CAPEsfc = true;

    QGridLayout  *lay = new QGridLayout(this);
    assert(lay);
    lay->addWidget( frameButtonsZone,1,0, Qt::AlignLeft);

    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));
    connect(this->btServerStatus,SIGNAL(clicked()),this,SLOT(slotServerStatus()));

    connect(loadgrib, SIGNAL(signalGribDataReceived(QByteArray *, QString)),
            this,  SLOT(slotGribDataReceived(QByteArray *, QString)));
    connect(loadgrib, SIGNAL(signalGribLoadError(QString)),
            this,  SLOT(slotGribFileError(QString)));
    connect(loadgrib, SIGNAL(signalGribSendMessage(QString)),
            this,  SLOT(slotGribMessage(QString)));
    connect(loadgrib, SIGNAL(signalGribStartLoadData()),
            this,  SLOT(slotGribStartLoadData()));
    connect(loadgrib,SIGNAL(progress(qint64,qint64)),this,(SLOT(slotProgress(qint64,qint64))));

    //------------------------------------------------------
    connect(sbNorth, SIGNAL(valueChanged(double)),
            this,  SLOT(slotParameterUpdated()));
    connect(sbSouth, SIGNAL(valueChanged(double)),
            this,  SLOT(slotParameterUpdated()));
    connect(sbWest, SIGNAL(valueChanged(double)),
            this,  SLOT(slotParameterUpdated()));
    connect(sbEast, SIGNAL(valueChanged(double)),
            this,  SLOT(slotParameterUpdated()));

    connect(cbResolution, SIGNAL(activated(int)),
            this,  SLOT(slotParameterUpdated()));
    connect(cbInterval, SIGNAL(activated(int)),
            this,  SLOT(slotParameterUpdated()));
    connect(cbDays, SIGNAL(activated(int)),
            this,  SLOT(slotParameterUpdated()));

    connect(chkWind, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkPressure, SIGNAL(stateChanged(int)), this,  SLOT(slotParameterUpdated()));
    connect(chkRain, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkCloud, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkTemp, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkHumid, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkIsotherm0, SIGNAL(stateChanged(int)), this,  SLOT(slotParameterUpdated()));

    connect(chkTempPot, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkTempMin, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkTempMax, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkSnowCateg, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkFrzRainCateg, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkSnowDepth, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkCAPEsfc, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));

    connect(chkAltitude200, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude300, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude500, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude700, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude850, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitudeAll, SIGNAL(stateChanged(int)), 	this,  SLOT(slotAltitudeAll()));
    Util::setFontDialog(this);
}

//-------------------------------------------------------------------------------
DialogLoadGrib::~DialogLoadGrib()
{
    Settings::setSetting(this->objectName()+".height",this->height());
    Settings::setSetting(this->objectName()+".width",this->width());
    if (loadgrib != NULL)
        delete loadgrib;
}
void DialogLoadGrib::slotProgress(qint64 i1, qint64 i2)
{
    this->progressBar->setRange(0,i2);
    this->progressBar->setValue(i1);
}

//----------------------------------------------------
void DialogLoadGrib::slotGribMessage(QString msg)
{
    labelMsg->setText(msg);
}

//----------------------------------------------------
void DialogLoadGrib::slotGribDataReceived(QByteArray *content, QString fileName)
{
    QString gribPath=Settings::getSetting("edtGribFolder",appFolder.value("grib")).toString();
    QDir dirGrib(gribPath);
    if(!dirGrib.exists())
    {
        gribPath=appFolder.value("grib");
        Settings::setSetting("askGribFolder",1);
        Settings::setSetting("edtGribFolder",gribPath);
    }
    fileName=gribPath+"/"+fileName;
    if(Settings::getSetting("askGribFolder",1)==1)
    {
        fileName = QFileDialog::getSaveFileName(this,
                         tr("Sauvegarde du fichier GRIB"), fileName, "Grib (*.grb)");
    }

    if (fileName != "")
    {
        QFile *saveFile = new QFile(fileName);
        assert(saveFile);
        bool ok;
        qint64 nb = 0;
        ok = saveFile->open(QIODevice::WriteOnly);
        if (ok) {
            nb = saveFile->write(*content);
            if (nb > 0) {
                saveFile->close();
            }
        }
        if (ok && nb>0) {
            emit signalGribFileReceived(fileName);
            loadInProgress = false;
            btCancel->setText(tr("Annuler"));
            btOK->setEnabled(true);
            accept();
        }
        else {
            QMessageBox::critical (this,
                    tr("Erreur"),
                    tr("Ecriture du fichier impossible."));
        }
    }
    else {
        QMessageBox::critical (this,
                tr("Erreur"),
                tr("Operation abandonnee."));
        loadInProgress = false;
        btCancel->setText(tr("Annuler"));
        btOK->setEnabled(true);
    }
}

//----------------------------------------------------
void DialogLoadGrib::slotGribFileError(QString error)
{
    if (! loadInProgress)
        return;

    QString s;
    QMessageBox::critical (this,
                    tr("Erreur"),
                    tr("Erreur : ") + error );

    loadInProgress = false;
    btCancel->setText(tr("Annuler"));
    btOK->setEnabled(true);
    labelMsg->setText("");
}

//----------------------------------------------------
void DialogLoadGrib::slotGribStartLoadData()
{
    timeLoad.start();
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::updateParameters()
{
    double tmp;

//    xmin = sbWest->cleanText().toDouble();
//    xmax = sbEast->cleanText().toDouble();
//    ymin = sbNorth->cleanText().toDouble();
//    ymax = sbSouth->cleanText().toDouble();

    resolution = cbResolution->currentText().toDouble();
    interval   = cbInterval->currentText().toInt();
    days       = cbDays->currentText().toInt();

    if (xmin > xmax) {
        tmp = xmin;   xmin = xmax;   xmax = tmp;
    }
    if (ymin < ymax) {    // echelle Y inversee (90=nord)
        tmp = ymin;   ymin = ymax;   ymax = tmp;
    }
    // trop grand ?
    if (fabs(xmax-xmin) >=360)
        xmax = xmin+359.9;
    if (fabs(ymin-ymax) >=180)
        ymin = ymax+179.9;

    // trop petit ?
    if (fabs(xmax-xmin) < 2*resolution) {
        const double xm = (xmin+xmax)/2;
        xmin = xm - 2*resolution;
        xmax = xm + 2*resolution;
    }
    if (fabs(ymin-ymax) < 2*resolution) {
        const double ym = (ymin+ymax)/2;
        ymin = ym + 2*resolution;
        ymax = ym - 2*resolution;
    }

    wind     = chkWind->isChecked();
    pressure = chkPressure->isChecked();
    rain     = chkRain->isChecked();
    cloud    = chkCloud->isChecked();
    temp     = chkTemp->isChecked();
    humid    = chkHumid->isChecked();
    isotherm0    = chkIsotherm0->isChecked();

    tempPot     = chkTempPot->isChecked();
    tempMin     = chkTempMin->isChecked();
    tempMax     = chkTempMax->isChecked();
    snowDepth   = chkSnowDepth->isChecked();
    snowCateg   = chkSnowCateg->isChecked();
    frzRainCateg = chkFrzRainCateg->isChecked();
    CAPEsfc      = chkCAPEsfc->isChecked();

    Settings::setSetting("downloadGribResolution", cbResolution->currentIndex());
    Settings::setSetting("downloadGribInterval",  cbInterval->currentIndex());
    Settings::setSetting("downloadGribNbDays",  cbDays->currentIndex());

    Settings::setSetting("downloadWind",  wind);
    Settings::setSetting("downloadPressure", pressure);
    Settings::setSetting("downloadRain",  rain);
    Settings::setSetting("downloadCloud", cloud);
    Settings::setSetting("downloadTemp",  temp);
    Settings::setSetting("downloadHumid", humid);
    Settings::setSetting("downloadIsotherm0", isotherm0);

    Settings::setSetting("downloadTempPot",  tempPot);
    Settings::setSetting("downloadTempMin",  tempMin);
    Settings::setSetting("downloadTempMax",  tempMax);
    Settings::setSetting("downloadSnowDepth", snowDepth);
    Settings::setSetting("downloadSnowCateg", snowCateg);
    Settings::setSetting("downloadFrzRainCateg", frzRainCateg);
    Settings::setSetting("downloadCAPEsfc", CAPEsfc);

    Settings::setSetting("downloadAltitudeData200",  chkAltitude200->isChecked());
    Settings::setSetting("downloadAltitudeData300",  chkAltitude300->isChecked());
    Settings::setSetting("downloadAltitudeData500",  chkAltitude500->isChecked());
    Settings::setSetting("downloadAltitudeData700",  chkAltitude700->isChecked());
    Settings::setSetting("downloadAltitudeData850",  chkAltitude850->isChecked());
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::slotParameterUpdated()
{
    updateParameters();

    int npts = (int) (  ceil(fabs(xmax-xmin)/resolution)
                       * ceil(fabs(ymax-ymin)/resolution) );

    // Nombre de GribRecords
    int nbrec = (int) days*24/interval +1;
    int nbPress = pressure ?  nbrec   : 0;
    int nbWind  = wind     ?  2*nbrec : 0;
    int nbRain  = rain     ?  nbrec-1 : 0;
    int nbCloud = cloud    ?  nbrec-1 : 0;
    int nbTemp  = temp     ?  nbrec   : 0;
    int nbHumid = humid    ?  nbrec   : 0;
    int nbIsotherm0 = isotherm0    ?  nbrec   : 0;

    int nbTempPot  = tempPot ?  nbrec   : 0;
    int nbTempMin  = tempMin ?  nbrec-1  : 0;
    int nbTempMax  = tempMax ?  nbrec-1  : 0;
    int nbSnowDepth  = snowDepth ?  nbrec  : 0;
    int nbSnowCateg  = snowCateg ?  nbrec-1  : 0;
    int nbFrzRainCateg = frzRainCateg ?  nbrec-1  : 0;
    int nbCAPEsfc  = CAPEsfc ?  nbrec : 0;

    int head = 84;
    int estime = 0;
    int nbits;

    nbits = 13;
    estime += nbWind*(head+(nbits*npts)/8+2 );

    nbits = 11;
    estime += nbTemp*(head+(nbits*npts)/8+2 );
    estime += nbTempPot*(head+(nbits*npts)/8+2 );
    estime += nbTempMin*(head+(nbits*npts)/8+2 );
    estime += nbTempMax*(head+(nbits*npts)/8+2 );
    nbits = 4;
    estime += nbRain*(head+(nbits*npts)/8+2 );
    nbits = 15;
    estime += nbPress*(head+(nbits*npts)/8+2 );
    nbits = 4;
    estime += nbCloud*(head+(nbits*npts)/8+2 );
    nbits = 1;
    estime += nbSnowDepth*(head+(nbits*npts)/8+2 );
    estime += nbSnowCateg*(head+(nbits*npts)/8+2 );
    estime += nbFrzRainCateg*(head+(nbits*npts)/8+2 );
    nbits = 10;
    estime += nbHumid*(head+(nbits*npts)/8+2 );
    nbits = 15;
    estime += nbIsotherm0*(head+(nbits*npts)/8+2 );
    nbits = 5;
    estime += nbCAPEsfc*(head+(nbits*npts)/8+2 );

    int nbalt = 0;
    if (chkAltitude200->isChecked()) nbalt++;
    if (chkAltitude300->isChecked()) nbalt++;
    if (chkAltitude500->isChecked()) nbalt++;
    if (chkAltitude700->isChecked()) nbalt++;
    if (chkAltitude850->isChecked()) nbalt++;
    nbits = 12;
    estime += nbrec*nbalt*5*(head+(nbits*npts)/8+2 );

    estime = estime/1024;
    slotGribMessage(tr("Taille estimee : environ %1 ko").arg(estime) );

    if (estime == 0)
        btOK->setEnabled(false);
    else
        btOK->setEnabled(true);

}

void DialogLoadGrib::slotAltitudeAll ()
{
        bool check = chkAltitudeAll->isChecked ();
        chkAltitude200->setChecked (check);
        chkAltitude300->setChecked (check);
        chkAltitude500->setChecked (check);
        chkAltitude700->setChecked (check);
        chkAltitude850->setChecked (check);
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::slotBtOK()
{
    btCancel->setText(tr("Stop"));

    loadInProgress = true;
    btOK->setEnabled(false);
    loadgrib->getGribFile(
                          xmin,ymin, xmax,ymax,
                          resolution, interval, days,
                          wind, pressure, rain, cloud, temp, humid, isotherm0,
                          tempPot, tempMin, tempMax, snowDepth, snowCateg, frzRainCateg,
                          CAPEsfc,
                          chkAltitude200->isChecked(),
                          chkAltitude300->isChecked(),
                          chkAltitude500->isChecked(),
                          chkAltitude700->isChecked(),
                          chkAltitude850->isChecked()
                          );
}
void DialogLoadGrib::slotServerStatus()
{
    this->btCancel->setEnabled(false);
    this->btOK->setEnabled(false);
    this->btServerStatus->setEnabled(false);
    loadgrib->getServerStatus();
}
void DialogLoadGrib::slotUngrayButtons()
{
    this->btCancel->setEnabled(true);
    this->btOK->setEnabled(true);
    this->btServerStatus->setEnabled(true);
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::slotBtCancel()
{
    if (loadInProgress)
    {
        loadInProgress = false;
        loadgrib->stop();
        btCancel->setText(tr("Annuler"));
        slotParameterUpdated();
    }
    else {
        reject();
    }
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::setZone(double x0, double y0, double x1, double y1)
{
    this->progressBar->setRange(0,100);
    this->progressBar->setValue(0);
    x0=floor(x0);
    x1=ceil(x1);
    y0=ceil(y0);
    y1=floor(y1);
    xmin=x0;
    xmax=x1;
    ymin=y0;
    ymax=y1;
    if(y0<0)
    {
        sbNorth->setValue(-y0);
        sbNorth->setSuffix(tr(" degS"));
    }
    else
    {
        sbNorth->setValue(y0);
        sbNorth->setSuffix(tr(" degN"));
    }
    if(y1<0)
    {
        sbSouth->setValue(-y1);
        sbSouth->setSuffix(tr(" degS"));
    }
    else
    {
        sbSouth->setValue(y1);
        sbSouth->setSuffix(tr(" degN"));
    }
    if(x0<0)
    {
        sbWest->setValue(-x0);
        sbWest->setSuffix(tr(" degW"));
    }
    else if (x0>180)
    {
        sbWest->setValue(360-x0);
        sbWest->setSuffix(tr(" degW"));
    }
    else
    {
        sbWest->setValue(x0);
        sbWest->setSuffix(tr(" degE"));
    }
    if(x0<1)
    {
        sbEast->setValue(-x1);
        sbEast->setSuffix(tr(" degW"));
    }
    else if (x1>180)
    {
        sbEast->setValue(360-x1);
        sbEast->setSuffix(tr(" degW"));
    }
    else
    {
        sbEast->setValue(x1);
        sbEast->setSuffix(tr(" degE"));
    }
    slotParameterUpdated();
}

//-------------------------------------------------------------------------------
QFrame *DialogLoadGrib::createFrameButtonsZone(QWidget *parent)
{
    QFrame * ftmp;
    QFrame * frm = new QFrame(parent);
    assert(frm);
    QVBoxLayout  *lay = new QVBoxLayout(frm);
    assert(lay);
        lay->setContentsMargins (0,0,0,0);
        lay->setSpacing (3);

    int ind, lig,col;
        //------------------------------------------------
        // Geographic area
        //------------------------------------------------
    int sizemin = 0;
    sbNorth = new QDoubleSpinBox(this);
    assert(sbNorth);
    sbNorth->setDecimals(0);
    sbNorth->setMinimum(-90);
    sbNorth->setMaximum(90);
    sbNorth->setSuffix(tr(" degN"));
    sbNorth->setReadOnly(true);

    sbSouth = new QDoubleSpinBox(this);
    assert(sbSouth);
    sbSouth->setDecimals(0);
    sbSouth->setMinimum(-90);
    sbSouth->setMaximum(90);
    sbSouth->setSuffix(tr(" degN"));
    sbSouth->setReadOnly(true);

    sbWest = new QDoubleSpinBox(this);
    assert(sbWest);
    sbWest->setDecimals(0);
    sbWest->setMinimum(-360);
    sbWest->setMaximum(360);
    sbWest->setSuffix(tr(" degE"));
    sbWest->setReadOnly(true);

    sbEast = new QDoubleSpinBox(this);
    assert(sbEast);
    sbEast->setDecimals(0);
    sbEast->setMinimum(-360);
    sbEast->setMaximum(360);
    sbEast->setSuffix(tr(" degE"));
    sbEast->setReadOnly(true);

        //------------------------------------------------
        // Resolution, intervalle, duree
        //------------------------------------------------
    cbResolution = new QComboBox(this);
    assert(cbResolution);
    cbResolution->addItems(QStringList()<< "0.5" << "1" << "2");
    cbResolution->setMinimumWidth (sizemin);
        ind = Settings::getSetting("downloadGribResolution", 0).toInt();
        ind = Util::inRange(ind, 0, cbResolution->count()-1);
    cbResolution->setCurrentIndex(ind);

    cbInterval = new QComboBox(this);
    assert(cbInterval);
    cbInterval->addItems(QStringList()<< "3" << "6" << "12" << "24");
    cbInterval->setMinimumWidth (sizemin);
        ind = Settings::getSetting("downloadGribInterval", 0).toInt();
        ind = Util::inRange(ind, 0, cbInterval->count()-1);
    cbInterval->setCurrentIndex(ind);

    cbDays = new QComboBox(this);
    assert(cbDays);
    cbDays->addItems(QStringList()<< "1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8");
    cbDays->setMinimumWidth (sizemin);
        ind = Settings::getSetting("downloadGribNbDays", 7).toInt();
        ind = Util::inRange(ind, 0, cbDays->count()-1);
    cbDays->setCurrentIndex(ind);

        //------------------------------------------------
        // Choix des donnees meteo
        //------------------------------------------------
    chkWind     = new QCheckBox(tr("Wind (10 m)"));
    assert(chkWind);
    chkPressure = new QCheckBox(tr("Mean sea level pressure"));
    assert(chkPressure);
    chkRain     = new QCheckBox(tr("Total precipitation"));
    assert(chkRain);
    chkCloud     = new QCheckBox(tr("Cloud cover"));
    assert(chkCloud);
    chkTemp     = new QCheckBox(tr("Temperature (2 m)"));
    assert(chkTemp);
    chkHumid    = new QCheckBox(tr("Relative humidity (2 m)"));
    assert(chkHumid);
    chkIsotherm0    = new QCheckBox(tr("Isotherm 0degC"));
    assert(chkIsotherm0);

    chkTempPot     = new QCheckBox(tr("Potential temperature (sigma 995)"));
    assert(chkTempPot);
    chkTempMin     = new QCheckBox(tr("Temperature min (2 m)"));
    assert(chkTempMin);
    chkTempMax     = new QCheckBox(tr("Temperature max (2 m)"));
    assert(chkTempMax);
    chkSnowCateg     = new QCheckBox(tr("Snow (snowfall possible)"));
    assert(chkSnowCateg);
    chkFrzRainCateg     = new QCheckBox(tr("Frozen rain (rainfall possible)"));
    assert(chkFrzRainCateg);
    chkSnowDepth     = new QCheckBox(tr("Snow (depth)"));
    assert(chkSnowDepth);
    chkCAPEsfc     = new QCheckBox(tr("CAPE (surface)"));
    assert(chkCAPEsfc);


    //--------------------------------------------------------------------------------
    chkWind->setChecked    (Settings::getSetting("downloadWind", true).toBool());
    chkPressure->setChecked(Settings::getSetting("downloadPressure", true).toBool());
    chkRain->setChecked    (Settings::getSetting("downloadRain", true).toBool());
    chkCloud->setChecked   (Settings::getSetting("downloadCloud", true).toBool());
    chkTemp->setChecked    (Settings::getSetting("downloadTemp", true).toBool());
    chkHumid->setChecked   (Settings::getSetting("downloadHumid", true).toBool());
    chkIsotherm0->setChecked  (Settings::getSetting("downloadIsotherm0", true).toBool());

    chkTempPot->setChecked    (Settings::getSetting("downloadTempPot", false).toBool());
    chkTempPot->setChecked    (false);

    chkTempMin->setChecked    (Settings::getSetting("downloadTempMin", false).toBool());
    chkTempMax->setChecked    (Settings::getSetting("downloadTempMax", false).toBool());
    chkSnowDepth->setChecked  (Settings::getSetting("downloadSnowDepth", true).toBool());
    chkSnowCateg->setChecked  (Settings::getSetting("downloadSnowCateg", true).toBool());
    chkCAPEsfc->setChecked  (Settings::getSetting("downloadCAPEsfc", true).toBool());
    chkFrzRainCateg->setChecked  (Settings::getSetting("downloadFrzRainCateg", true).toBool());

        //----------------------------------------------------------------
    chkAltitude850  = new QCheckBox ("850 "+tr("hPa"));
    assert (chkAltitude850);
    chkAltitude850->setChecked  (Settings::getSetting("downloadAltitudeData850", false).toBool());
    chkAltitude700  = new QCheckBox ("700 "+tr("hPa"));
    assert (chkAltitude700);
    chkAltitude700->setChecked  (Settings::getSetting("downloadAltitudeData700", false).toBool());
    chkAltitude500  = new QCheckBox ("500 "+tr("hPa"));
    assert (chkAltitude500);
    chkAltitude500->setChecked  (Settings::getSetting("downloadAltitudeData500", false).toBool());
    chkAltitude300  = new QCheckBox ("300 "+tr("hPa"));
    assert (chkAltitude300);
    chkAltitude300->setChecked  (Settings::getSetting("downloadAltitudeData300", false).toBool());
    chkAltitude200  = new QCheckBox ("200 "+tr("hPa"));
    assert (chkAltitude200);
    chkAltitude200->setChecked  (Settings::getSetting("downloadAltitudeData200", false).toBool());

    chkAltitudeAll = new QCheckBox (tr("All"));
    assert (chkAltitudeAll);
        chkAltitudeAll->setChecked  (
                      chkAltitude200->isChecked() && chkAltitude300->isChecked()
                   && chkAltitude500->isChecked() && chkAltitude700->isChecked()
                   && chkAltitude850->isChecked()
                );

        //----------------------------------------------------------------
    btOK     = new QPushButton(tr("Download GRIB file"), this);
    assert(btOK);
    btCancel = new QPushButton(tr("Cancel"), this);
    assert(btCancel);
    btServerStatus = new QPushButton(tr("Server status"), this);
    assert(btServerStatus);

    progressBar = new QProgressBar();
    assert(progressBar);

    QLayout  *tlay;
    QGridLayout  *tgrid;
        //------------------------------------------------
        // Disposition des widgets
        //------------------------------------------------

        ftmp = new QFrame(this);
    tgrid = new QGridLayout(ftmp);
    assert(tgrid);
        tgrid->setContentsMargins (0,0,0,0);
                tgrid->addWidget( new QLabel(tr("Latitude min :")), 0, 0, Qt::AlignRight);
                tgrid->addWidget( sbNorth, 0, 1);
                tgrid->addWidget( new QLabel(tr("Latitude max :")), 0, 2, Qt::AlignRight);
                tgrid->addWidget( sbSouth, 0, 3);

                tgrid->addWidget( new QLabel(tr("Longitude min :")), 1, 0, Qt::AlignRight);
                tgrid->addWidget( sbWest, 1, 1);
                tgrid->addWidget( new QLabel(tr("Longitude max :")), 1, 2, Qt::AlignRight);
                tgrid->addWidget( sbEast, 1, 3);

    lay->addWidget( ftmp);

    //-------------------------
    addSeparator (lay, 'H');
    //-------------------------

        ftmp = new QFrame(this);
    tlay = new QHBoxLayout(ftmp);
    assert(tlay);
        tlay->setContentsMargins (0,0,0,0);

                tlay->addWidget( new QLabel(tr("Resolution :")));
                tlay->addWidget( cbResolution);
                tlay->addWidget( new QLabel(tr(" deg")));
                //-------------------------
                addSeparator (tlay, 'V');
                //-------------------------
                tlay->addWidget( new QLabel(tr("Interval :")));
                tlay->addWidget( cbInterval);
                tlay->addWidget( new QLabel(tr(" hours")));
                //-------------------------
                addSeparator (tlay, 'V');
                //-------------------------
                tlay->addWidget( new QLabel(tr("Period :")));
                tlay->addWidget( cbDays);
                tlay->addWidget( new QLabel(tr(" days")));
    lay->addWidget( ftmp);

    //-------------------------
    addSeparator (lay, 'H');
    //-------------------------
        ftmp = new QFrame(this);
    tgrid = new QGridLayout(ftmp);
    assert(tgrid);
        tgrid->setContentsMargins (0,0,0,0);
        tgrid->setSpacing (0);
        //-----------------------------
        // Colonne 1
        //-----------------------------
        col = 0;
        lig = 0;
                tgrid->addWidget( chkWind ,       lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkPressure ,   lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkTemp ,      lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkTempMin ,   lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkTempMax ,   lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkTempPot , lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkIsotherm0 , lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkCAPEsfc ,  lig++, col, Qt::AlignLeft);
        //-----------------------------
        // Colonne 2
        //-----------------------------
        col = 1;
        lig = 0;
                tgrid->addWidget( chkCloud ,   lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkHumid ,   lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkRain ,    lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkSnowCateg ,  lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkSnowDepth ,  lig++, col, Qt::AlignLeft);
                tgrid->addWidget( chkFrzRainCateg , lig++, col, Qt::AlignLeft);
    lay->addWidget( ftmp);

    //-------------------------
    addSeparator (lay, 'H');
    //-------------------------
        lay->addWidget (new QLabel (
                        tr("Atmosphere: geopotential altitude, wind, temperature, theta-e, relative humidity.")
                        +"\n"+
                        tr("Warning : these data increase strongly the size of the GRIB file.")
                ));
        //lay->addWidget( chkAltitudeData );
        ftmp = new QFrame(this);
        assert (ftmp);
                tlay = new QHBoxLayout(ftmp);
                assert (tlay);
                tlay->setContentsMargins (0,0,0,0);
                tlay->addWidget (chkAltitude850);
                tlay->addWidget (chkAltitude700);
                tlay->addWidget (chkAltitude500);
                tlay->addWidget (chkAltitude300);
                tlay->addWidget (chkAltitude200);
                addSeparator (tlay, 'V');
                addSeparator (tlay, 'V');
                tlay->addWidget (chkAltitudeAll);
        lay->addWidget (ftmp);

    //-------------------------
    addSeparator (lay, 'H');
    //-------------------------
    lay->addWidget( progressBar );
    //-------------------------
    labelMsg = new QLabel();
    lay->addWidget( labelMsg );
    //lay->addWidget(new QLabel(tr("File size max: 20000 ko.")));
    lay->addWidget(new QLabel(tr("File size max: ")+ "51200 Ko (50 Mo)"));
    //-------------------------
    addSeparator (lay, 'H');
    //-------------------------
        ftmp = new QFrame(this);
    tlay = new QHBoxLayout(ftmp);
    assert(tlay);
        tlay->setContentsMargins (0,0,0,0);
                tlay->addWidget( btOK );
                tlay->addWidget( btServerStatus );
                tlay->addWidget( btCancel );
    lay->addWidget( ftmp);

    return frm;
}
//----------------------------------------------------------------------------
void DialogLoadGrib::addSeparator (QLayout *layout, char orientation)
{
        QFrame *ftmp;
        ftmp = new QFrame ();
        assert (ftmp);
        if (orientation == 'H') {
                ftmp->setFrameShape(QFrame::HLine);
        }
        else {
                ftmp->setFrameShape(QFrame::VLine);
        }
        ftmp->setStyleSheet ("color:#AAAAAA");
        layout->addWidget(ftmp);
}











