/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - maitai
*/
#ifdef QT_V5
#include <QtWidgets/QMessageBox>
#else
#include <QMessageBox>
#endif
#include <cmath>
#include <cassert>

#include "DialogZyGrib.h"
#include "settings.h"
#include "Util.h"
#include "LoadGribFile.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include <QScrollArea>
#include <QScrollBar>
#include <QFileDialog>
#include <QScreen>



//-------------------------------------------------------------------------------
DialogZygrib::DialogZygrib(MainWindow *main) : QDialog(main)
{
    this->main=main;
    setupUi(this);
    cbResolution->clear();
    cbResolution->addItems(QStringList()<< "0.5"+tr("deg") << "1"+tr("deg") << "2"+tr("deg"));
    cbInterval->clear();
    cbInterval->addItems(QStringList()<< "3 "+tr("heures") << "6 "+tr("heures") << "12 "+tr("heures") << "24 "+tr("heures"));
    cbDays->clear();
    cbDays->addItems(QStringList()<< "1 "+tr("jour")<<"2 "+tr("jours")<<"3 "+tr("jours")<<"4 "+tr("jours")<<"5 "+tr("jours")<<"6 "+tr("jours")<<"7 "+tr("jours")<<"8 "+tr("jours"));
    cbResolution->setItemData(0,0.5);
    cbResolution->setItemData(1,1);
    cbResolution->setItemData(2,2);
    cbInterval->setItemData(0,3);
    cbInterval->setItemData(1,6);
    cbInterval->setItemData(2,12);
    cbInterval->setItemData(3,24);
    cbDays->setItemData(0,1);
    cbDays->setItemData(1,2);
    cbDays->setItemData(2,3);
    cbDays->setItemData(3,4);
    cbDays->setItemData(4,5);
    cbDays->setItemData(5,6);
    cbDays->setItemData(6,7);
    cbDays->setItemData(7,8);

    loadgrib = new LoadGribFile();
    this->setObjectName("dialogZygrib");
    connect (loadgrib,SIGNAL(ungrayButtons()),this,SLOT(slotUngrayButtons()));
    connect (loadgrib,SIGNAL(clearSelection()),this,SIGNAL(clearSelection()));
    assert(loadgrib);

    setWindowTitle(tr("Telechargement"));
    loadInProgress = false;
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
    snowCateg = true;
    CAPEsfc = true;
    CINsfc = true;


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
    connect(chkCAPEsfc, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkCINsfc,  SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));

    connect(chkAltitude200, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude300, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude500, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude700, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitude850, SIGNAL(stateChanged(int)), 	this,  SLOT(slotParameterUpdated()));
    connect(chkAltitudeAll, SIGNAL(stateChanged(int)), 	this,  SLOT(slotAltitudeAll()));
    this->setParent(main);
    loadSettings();
    Util::setFontDialog(this,false);
}
void DialogZygrib::loadSettings()
{
    int ind = Settings::getSetting(downloadGribResolution).toInt();
    ind = Util::inRange(ind, 0, cbResolution->count()-1);
    cbResolution->setCurrentIndex(ind);
    ind = Settings::getSetting(downloadGribInterval).toInt();
    ind = Util::inRange(ind, 0, cbInterval->count()-1);
    cbInterval->setCurrentIndex(ind);
    ind = Settings::getSetting(downloadGribNbDays).toInt();
    ind = Util::inRange(ind, 0, cbDays->count()-1);
    cbDays->setCurrentIndex(ind);
    chkWind->setChecked    (Settings::getSetting(downloadWind).toBool());
    chkPressure->setChecked(Settings::getSetting(downloadPressure).toBool());
    chkRain->setChecked    (Settings::getSetting(downloadRain).toBool());
    chkCloud->setChecked   (Settings::getSetting(downloadCloud).toBool());
    chkTemp->setChecked    (Settings::getSetting(downloadTemp).toBool());
    chkHumid->setChecked   (Settings::getSetting(downloadHumid).toBool());
    chkIsotherm0->setChecked  (Settings::getSetting(downloadIsotherm0).toBool());
    chkTempPot->setChecked    (Settings::getSetting(downloadTempPot).toBool());
    chkTempPot->setChecked    (false);
    chkTempMin->setChecked    (Settings::getSetting(downloadTempMin).toBool());
    chkTempMax->setChecked    (Settings::getSetting(downloadTempMax).toBool());
    chkSnowCateg->setChecked  (Settings::getSetting(downloadSnowCateg).toBool());
    chkCAPEsfc->setChecked  (Settings::getSetting(downloadCAPEsfc).toBool());
    chkCINsfc->setChecked  (Settings::getSetting(downloadCINsfc).toBool());
    chkFrzRainCateg->setChecked  (Settings::getSetting(downloadFrzRainCateg).toBool());
    chkAltitude850->setChecked  (Settings::getSetting(downloadAltitudeData850).toBool());
    chkAltitude700->setChecked  (Settings::getSetting(downloadAltitudeData700).toBool());
    chkAltitude500->setChecked  (Settings::getSetting(downloadAltitudeData500).toBool());
    chkAltitude300->setChecked  (Settings::getSetting(downloadAltitudeData300).toBool());
    chkAltitude200->setChecked  (Settings::getSetting(downloadAltitudeData200).toBool());
    chkAltitudeAll->setChecked  (chkAltitude200->isChecked() && chkAltitude300->isChecked()
               && chkAltitude500->isChecked() && chkAltitude700->isChecked()
               && chkAltitude850->isChecked());
}

void DialogZygrib::slot_screenResize()
{
}

//-------------------------------------------------------------------------------
DialogZygrib::~DialogZygrib()
{
    //Settings::saveGeometry(this);
    if (loadgrib != NULL)
        delete loadgrib;
}
void DialogZygrib::slotProgress(qint64 i1, qint64 i2)
{
    this->progressBar->setRange(0,i2);
    this->progressBar->setValue(i1);
}

//----------------------------------------------------
void DialogZygrib::slotGribMessage(QString msg)
{
    labelMsg->setText(msg);
}

//----------------------------------------------------
void DialogZygrib::slotGribDataReceived(QByteArray *content, QString file)
{
    QString gribPath=Settings::getSetting(edtGribFolder).toString();
    QDir dirGrib(gribPath);
    if(!dirGrib.exists())
    {
        gribPath=appFolder.value("grib");
        Settings::setSetting(askGribFolder,1);
        Settings::setSetting(edtGribFolder,gribPath);
    }
    fileName=gribPath+"/"+file;
    if(Settings::getSetting(askGribFolder)==1)
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
            saveFile->close();
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
void DialogZygrib::slotGribFileError(QString error)
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
void DialogZygrib::slotGribStartLoadData()
{
    timeLoad.start();
}

//-------------------------------------------------------------------------------
void DialogZygrib::updateParameters()
{
    double tmp;

    cbResolution->currentData().toDouble();
    resolution = cbResolution->currentData().toDouble();
    interval   = cbInterval->currentData().toInt();
    days       = cbDays->currentData().toInt();

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
    snowCateg   = chkSnowCateg->isChecked();
    frzRainCateg = chkFrzRainCateg->isChecked();
    CAPEsfc      = chkCAPEsfc->isChecked();
    CINsfc      = chkCINsfc->isChecked();

    Settings::setSetting(downloadGribResolution, cbResolution->currentIndex());
    Settings::setSetting(downloadGribInterval,  cbInterval->currentIndex());
    Settings::setSetting(downloadGribNbDays,  cbDays->currentIndex());

    Settings::setSetting(downloadWind,  wind);
    Settings::setSetting(downloadPressure, pressure);
    Settings::setSetting(downloadRain,  rain);
    Settings::setSetting(downloadCloud, cloud);
    Settings::setSetting(downloadTemp,  temp);
    Settings::setSetting(downloadHumid, humid);
    Settings::setSetting(downloadIsotherm0,isotherm0);
    Settings::setSetting(downloadTempPot,  tempPot);
    Settings::setSetting(downloadTempMin,  tempMin);
    Settings::setSetting(downloadTempMax,  tempMax);
    Settings::setSetting(downloadSnowCateg, snowCateg);
    Settings::setSetting(downloadFrzRainCateg, frzRainCateg);
    Settings::setSetting(downloadCAPEsfc, CAPEsfc);
    Settings::setSetting(downloadCINsfc, CINsfc);

    Settings::setSetting(downloadAltitudeData200,  chkAltitude200->isChecked());
    Settings::setSetting(downloadAltitudeData300,  chkAltitude300->isChecked());
    Settings::setSetting(downloadAltitudeData500,  chkAltitude500->isChecked());
    Settings::setSetting(downloadAltitudeData700,  chkAltitude700->isChecked());
    Settings::setSetting(downloadAltitudeData850,  chkAltitude850->isChecked());
}

//-------------------------------------------------------------------------------
void DialogZygrib::slotParameterUpdated()
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
    int nbSnowCateg  = snowCateg ?  nbrec-1  : 0;
    int nbFrzRainCateg = frzRainCateg ?  nbrec-1  : 0;
    int nbCAPEsfc  = CAPEsfc ?  nbrec : 0;
    int nbCINsfc  = CINsfc ?  nbrec : 0;

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
    estime += nbSnowCateg*(head+(nbits*npts)/8+2 );
    estime += nbFrzRainCateg*(head+(nbits*npts)/8+2 );
    nbits = 10;
    estime += nbHumid*(head+(nbits*npts)/8+2 );
    nbits = 15;
    estime += nbIsotherm0*(head+(nbits*npts)/8+2 );
    nbits = 5;
    estime += nbCAPEsfc*(head+(nbits*npts)/8+2 );
    estime += nbCINsfc*(head+(nbits*npts)/8+2 );

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

void DialogZygrib::slotAltitudeAll ()
{
        bool check = chkAltitudeAll->isChecked ();
        chkAltitude200->setChecked (check);
        chkAltitude300->setChecked (check);
        chkAltitude500->setChecked (check);
        chkAltitude700->setChecked (check);
        chkAltitude850->setChecked (check);
}

//-------------------------------------------------------------------------------
void DialogZygrib::slotBtOK()
{
    btCancel->setText(tr("Stop"));
    Settings::saveGeometry(this);

    loadInProgress = true;
    btOK->setEnabled(false);
    loadgrib->getGribFile(
                          xmin,ymin, xmax,ymax,
                          resolution, interval, days,
                          wind, pressure, rain, cloud, temp, humid, isotherm0,
                          tempPot, tempMin, tempMax, snowCateg, frzRainCateg,
                          CAPEsfc,CINsfc,
                          chkAltitude200->isChecked(),
                          chkAltitude300->isChecked(),
                          chkAltitude500->isChecked(),
                          chkAltitude700->isChecked(),
                          chkAltitude850->isChecked()
                          );
}
void DialogZygrib::slotServerStatus()
{
    this->btCancel->setEnabled(false);
    this->btOK->setEnabled(false);
    this->btServerStatus->setEnabled(false);
    loadgrib->getServerStatus();
}
void DialogZygrib::slotUngrayButtons()
{
    this->btCancel->setEnabled(true);
    this->btOK->setEnabled(true);
    this->btServerStatus->setEnabled(true);
}

//-------------------------------------------------------------------------------
void DialogZygrib::slotBtCancel()
{
    if (loadInProgress)
    {
        loadInProgress = false;
        loadgrib->stop();
        btCancel->setText(tr("Annuler"));
        slotParameterUpdated();
    }
    else {
        Settings::saveGeometry(this);
        reject();
    }
}

//-------------------------------------------------------------------------------
void DialogZygrib::setZone(double x0, double y0, double x1, double y1)
{
    loadSettings();
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
