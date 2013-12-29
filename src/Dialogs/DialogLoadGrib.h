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

#ifndef DIALOGLOADGRIB_H
#define DIALOGLOADGRIB_H
#ifdef QT_V5
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>
#else
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressBar>
#endif
#include "LoadGribFile.h"
#include <QTime>

//#define HAS_TEMP

#include "class_list.h"

class DialogLoadGrib : public QDialog
{ Q_OBJECT
    public:
        DialogLoadGrib(MainWindow * main);
        ~DialogLoadGrib();

        void setZone(double x0, double y0, double x1, double y1);
        void checkQtvlmVersion(){loadgrib->checkQtvlmVersion();}
    public slots:
        void slotBtOK();
        void slotBtCancel();
        void slotServerStatus();
        void slotGribDataReceived(QByteArray *content, QString fileName);
        void slotGribFileError(QString error);
        void slotGribMessage(QString msg);
        void slotGribStartLoadData();
        void slotParameterUpdated();
        void slotAltitudeAll();
        void slotUngrayButtons();
        void slotProgress(qint64,qint64);

    signals:
        void signalGribFileReceived(QString fileName);
        void clearSelection();

    private:
        LoadGribFile    *loadgrib;
        bool     loadInProgress;
        QTime    timeLoad;

        QDoubleSpinBox *sbNorth;
        QDoubleSpinBox *sbSouth;
        QDoubleSpinBox *sbWest;
        QDoubleSpinBox *sbEast;

        QComboBox *cbResolution;
        QComboBox *cbInterval;
        QComboBox *cbDays;

        QCheckBox *chkWind;
        QCheckBox *chkPressure;
        QCheckBox *chkRain;
        QCheckBox *chkCloud;
        QCheckBox *chkTemp;
        QCheckBox *chkHumid;
        QCheckBox *chkIsotherm0;
        QCheckBox *chkTempPot;
        QCheckBox *chkTempMin;
        QCheckBox *chkTempMax;
        QCheckBox *chkSnowCateg;
        QCheckBox *chkFrzRainCateg;
        QCheckBox *chkCAPEsfc;
        QCheckBox *chkCINsfc;

        QCheckBox *chkAltitudeAll;
        QCheckBox *chkAltitude200;
        QCheckBox *chkAltitude300;
        QCheckBox *chkAltitude500;
        QCheckBox *chkAltitude700;
        QCheckBox *chkAltitude850;

        QPushButton *btOK;
        QPushButton *btCancel;
        QPushButton *btServerStatus;

        QProgressBar *progressBar;

        QLabel       *labelMsg;

        QFrame *createFrameButtonsZone(QWidget *parent);

        double   xmin,ymin,xmax,ymax,resolution;
        int     interval,days;
        bool    rain, cloud, pressure, wind, temp, humid, isotherm0;
        bool	tempPot, tempMin, tempMax, snowCateg, frzRainCateg;
        bool 	CAPEsfc, CINsfc;

        void    updateParameters();
        void    addSeparator (QLayout *layout, char orientation);	// 'H' or 'V'


};


#endif
