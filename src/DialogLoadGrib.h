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

#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QTime>
#include <QProgressBar>

//#define HAS_TEMP

#include "class_list.h"

class DialogLoadGrib : public QDialog
{ Q_OBJECT
    public:
        DialogLoadGrib();
        ~DialogLoadGrib();

        void setZone(float x0, float y0, float x1, float y1);

    public slots:
        void slotBtOK();
        void slotBtCancel();
        void slotGribDataReceived(QByteArray *content, QString fileName);
        void slotGribFileError(QString error);
        void slotGribMessage(QString msg);
        void slotGribStartLoadData();
        void slotParameterUpdated();
        void slotAltitudeAll();

    signals:
        void signalGribFileReceived(QString fileName);

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
        QCheckBox *chkSnowDepth;
        QCheckBox *chkCAPEsfc;

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

        float   xmin,ymin,xmax,ymax,resolution;
        int     interval,days;
        bool    rain, cloud, pressure, wind, temp, humid, isotherm0;
        bool	tempPot, tempMin, tempMax, snowDepth, snowCateg, frzRainCateg;
        bool 	CAPEsfc;

        void    updateParameters();
        void    addSeparator (QLayout *layout, char orientation);	// 'H' or 'V'


};


#endif
