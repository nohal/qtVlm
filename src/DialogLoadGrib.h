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
#include <QProgressBar>
#include <QFileDialog>
#include <QTime>

//#define HAS_TEMP

#include "LoadGribFile.h"

class DialogLoadGrib : public QDialog
{ Q_OBJECT
    public:
        DialogLoadGrib();
        ~DialogLoadGrib();

        void setZone(float x0, float y0, float x1, float y1);

    public slots:
        void slotBtOK();
        void slotBtCancel();
        void slotBtServerStatus();
        void slotGribDataReceived(QByteArray *content, QString fileName);
        void slotGribReadProgress(int step, int done, int total);
        void slotGribFileError(QString error);
        void slotGribMessage(QString msg);
        void slotGribStartLoadData();
        void slotParameterUpdated();

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
#ifdef HAS_TEMP
        QCheckBox *chkTemp;
#endif

        QPushButton *btOK;
        QPushButton *btCancel;
        QPushButton *btServerStatus;

        QProgressBar *progressBar;
        QLabel       *labelMsg;

        QFrame *createFrameButtonsZone(QWidget *parent);

        float   xmin,ymin,xmax,ymax,resolution;
        int     interval,days;
        bool    pressure, wind;
#ifdef HAS_TEMP
	bool    temp;
#endif
        void    updateParameters();


};


#endif
