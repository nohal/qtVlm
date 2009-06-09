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

#include <QMessageBox>
#include <cmath>
#include <cassert>

#include "DialogLoadGrib.h"
#include "Util.h"



//-------------------------------------------------------------------------------
DialogLoadGrib::DialogLoadGrib() : QDialog()
{
    loadgrib = new LoadGribFile();
    assert(loadgrib);

    setWindowTitle(tr("Téléchargement"));
    loadInProgress = false;
    QFrame * frameButtonsZone = createFrameButtonsZone(this);

    QGridLayout  *lay = new QGridLayout(this);
    assert(lay);
    lay->addWidget( frameButtonsZone,1,0, Qt::AlignLeft);

    connect(btCancel, SIGNAL(clicked()), this, SLOT(slotBtCancel()));
    connect(btOK, SIGNAL(clicked()), this, SLOT(slotBtOK()));

    connect(loadgrib, SIGNAL(signalGribDataReceived(QByteArray *, QString)),
            this,  SLOT(slotGribDataReceived(QByteArray *, QString)));
    connect(loadgrib, SIGNAL(signalGribLoadError(QString)),
            this,  SLOT(slotGribFileError(QString)));
    connect(loadgrib, SIGNAL(signalGribSendMessage(QString)),
            this,  SLOT(slotGribMessage(QString)));
    connect(loadgrib, SIGNAL(signalGribStartLoadData()),
            this,  SLOT(slotGribStartLoadData()));

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
}

//-------------------------------------------------------------------------------
DialogLoadGrib::~DialogLoadGrib()
{
    if (loadgrib != NULL)
        delete loadgrib;
}

//----------------------------------------------------
void DialogLoadGrib::slotGribMessage(QString msg)
{
    labelMsg->setText(msg);
}

//----------------------------------------------------
void DialogLoadGrib::slotGribDataReceived(QByteArray *content, QString fileName)
{
    fileName = QFileDialog::getSaveFileName(this,
                 tr("Sauvegarde du fichier GRIB"), "grib/"+fileName, "");

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
                tr("Opération abandonnée."));
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
    float tmp, xm, ym;

    xmin = sbWest->cleanText().toFloat();
    xmax = sbEast->cleanText().toFloat();
    ymin = sbNorth->cleanText().toFloat();
    ymax = sbSouth->cleanText().toFloat();

    resolution = cbResolution->currentText().toFloat();
    interval   = cbInterval->currentText().toInt();
    days       = cbDays->currentText().toInt();

    if (xmin > xmax) {
        tmp = xmin;   xmin = xmax;   xmax = tmp;
    }
    if (ymin < ymax) {    // échelle Y inversée (90=nord)
        tmp = ymin;   ymin = ymax;   ymax = tmp;
    }
    // trop grand ?
    if (fabs(xmax-xmin) >=360)
        xmax = xmin+359.9;
    if (fabs(ymin-ymax) >=180)
        ymin = ymax+179.9;

    // trop petit ?
    if (fabs(xmax-xmin) < 2*resolution) {
        xm = (xmin+xmax)/2;
        xmin = xm - 2*resolution;
        xmax = xm + 2*resolution;
    }
    if (fabs(ymin-ymax) < 2*resolution) {
        ym = (ymin+ymax)/2;
        ymin = ym + 2*resolution;
        ymax = ym - 2*resolution;
    }

        Util::setSetting("downloadIndResolution", cbResolution->currentIndex());
        Util::setSetting("downloadIndInterval",  cbInterval->currentIndex());
        Util::setSetting("downloadIndNbDays",  cbDays->currentIndex());
}

//-------------------------------------------------------------------------------
void DialogLoadGrib::slotParameterUpdated()
{
    updateParameters();

    int npts = (int) (  ceil(fabs(xmax-xmin)/resolution)
                       * ceil(fabs(ymax-ymin)/resolution) );

    // Nombre de GribRecords
    int nbrec = (int) days*24/interval +1;
    int nbWind  = 2*nbrec;

    int head = 84;
    int estime = 0;
    int nbits;

    nbits = 13;
    estime += nbWind*(head+(nbits*npts)/8+2 );

    estime = estime/1024;
    slotGribMessage(tr("Taille estimée : environ %1 ko").arg(estime) );

    if (estime == 0)
        btOK->setEnabled(false);
    else
        btOK->setEnabled(true);

}

//-------------------------------------------------------------------------------
void DialogLoadGrib::slotBtOK()
{
    btCancel->setText(tr("Stop"));

    loadInProgress = true;
    btOK->setEnabled(false);
    loadgrib->getGribFile(xmin,ymin,xmax,ymax,resolution,interval,days);
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
void DialogLoadGrib::setZone(float x0, float y0, float x1, float y1)
{
    sbNorth->setValue(y0);
    sbSouth->setValue(y1);
    sbWest->setValue(x0);
    sbEast->setValue(x1);
    slotParameterUpdated();
}

//-------------------------------------------------------------------------------
QFrame *DialogLoadGrib::createFrameButtonsZone(QWidget *parent)
{
    QFrame * frm = new QFrame(parent);
    assert(frm);
    QGridLayout  *lay = new QGridLayout(frm);
    assert(lay);
    int ind, lig;
    QFrame *ftmp;
    int sizemin = 80;
    sbNorth = new QDoubleSpinBox(this);
    assert(sbNorth);
    sbNorth->setDecimals(0);
    sbNorth->setMinimum(-90);
    sbNorth->setMaximum(90);
    sbNorth->setSuffix(tr(" °N"));
    sbNorth->setMinimumWidth (sizemin);

    sbSouth = new QDoubleSpinBox(this);
    assert(sbSouth);
    sbSouth->setDecimals(0);
    sbSouth->setMinimum(-90);
    sbSouth->setMaximum(90);
    sbSouth->setSuffix(tr(" °N"));
    sbSouth->setMinimumWidth (sizemin);

    sbWest = new QDoubleSpinBox(this);
    assert(sbWest);
    sbWest->setDecimals(0);
    sbWest->setMinimum(-360);
    sbWest->setMaximum(360);
    sbWest->setSuffix(tr(" °E"));
    sbWest->setMinimumWidth (sizemin);

    sbEast = new QDoubleSpinBox(this);
    assert(sbEast);
    sbEast->setDecimals(0);
    sbEast->setMinimum(-360);
    sbEast->setMaximum(360);
    sbEast->setSuffix(tr(" °E"));
    sbEast->setMinimumWidth (sizemin);

    cbResolution = new QComboBox(this);
    assert(cbResolution);
    cbResolution->addItems(QStringList()<< "0.5" << "1" << "2");
    cbResolution->setMinimumWidth (sizemin);
        ind = Util::getSetting("downloadIndResolution", 1).toInt();
        ind = Util::inRange(ind, 0, cbResolution->count()-1);
    cbResolution->setCurrentIndex(ind);

    cbInterval = new QComboBox(this);
    assert(cbInterval);
    cbInterval->addItems(QStringList()<< "3" << "6" << "12" << "24");
    cbInterval->setMinimumWidth (sizemin);
        ind = Util::getSetting("downloadIndInterval", 1).toInt();
        ind = Util::inRange(ind, 0, cbInterval->count()-1);
    cbInterval->setCurrentIndex(ind);

    cbDays = new QComboBox(this);
    assert(cbDays);
    cbDays->addItems(QStringList()<< "1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7");
    cbDays->setMinimumWidth (sizemin);
        ind = Util::getSetting("downloadIndNbDays", 4).toInt();
        ind = Util::inRange(ind, 0, cbDays->count()-1);
    cbDays->setCurrentIndex(ind);

    btOK     = new QPushButton(tr("Télécharger le fichier GRIB"), this);
    assert(btOK);
    btCancel = new QPushButton(tr("Annuler"), this);
    assert(btCancel);

    lig = 0;
    lay->addWidget( new QLabel(tr("Latitude min :")), lig, 0,  Qt::AlignRight);
    lay->addWidget( sbNorth, lig, 1, Qt::AlignLeft );
    lay->addWidget( new QLabel(tr("Latitude max :")), lig, 2, Qt::AlignRight);
    lay->addWidget( sbSouth, lig, 3, Qt::AlignLeft );
    lig ++;
    lay->addWidget( new QLabel(tr("Longitude min :")), lig, 0, Qt::AlignRight);
    lay->addWidget( sbWest, lig, 1, Qt::AlignLeft );
    lay->addWidget( new QLabel(tr("Longitude max :")), lig, 2, Qt::AlignRight);
    lay->addWidget( sbEast, lig, 3, Qt::AlignLeft );
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); lay->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    lay->addWidget( new QLabel(tr("Résolution :")), lig,0, Qt::AlignRight);
    lay->addWidget( cbResolution, lig, 1, Qt::AlignLeft );
    lay->addWidget( new QLabel(tr(" °")), lig,2, Qt::AlignLeft);
    lig ++;
    lay->addWidget( new QLabel(tr("Intervalle :")), lig,0, Qt::AlignRight);
    lay->addWidget( cbInterval, lig, 1, Qt::AlignLeft );
    lay->addWidget( new QLabel(tr(" heures")), lig,2, Qt::AlignLeft);
    lig ++;
    lay->addWidget( new QLabel(tr("Durée :")), lig,0, Qt::AlignRight);
    lay->addWidget( cbDays, lig, 1, Qt::AlignLeft );
    lay->addWidget( new QLabel(tr(" jours")), lig,2, Qt::AlignLeft);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); lay->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    labelMsg = new QLabel();
    lay->addWidget( labelMsg, lig,0, 1, -1);
    lig ++;
    lay->addWidget(new QLabel(tr("La taille des fichiers est limitée �  20000 ko.")),lig,0,1,-1);
    //-------------------------
    lig ++;
    ftmp = new QFrame(this); ftmp->setFrameShape(QFrame::HLine); lay->addWidget( ftmp, lig,0, 1, -1);
    //-------------------------
    lig ++;
    lay->addWidget( btOK,    lig,0,   1, 2);
    lay->addWidget( btCancel, lig,2,  1, 2);


    return frm;
}











