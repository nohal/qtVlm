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
***********************************************************************/

#ifndef VLM_BOARD_H
#define VLM_BOARD_H

#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QMainWindow>

#include "boatAccount.h"
#include "ui_BoardVLM_part1.h"
#include "ui_BoardVLM_part2.h"
#include "Util.h"

#include "GribPlot.h"
#include "Util.h"

class boardVLM_part2: public QWidget , public Ui::boardVLM_part2_ui
{
    Q_OBJECT
    public:
        boardVLM_part2(QWidget * parent=0);
        void boatUpdate(boatAccount * boat);

        void showGribPointInfo(const GribPointInfo &pf);

    public slots:
        void doSaveWP();
        void doClearWP();
        void chgLat();
        void chgLon();
        void doPilotOrtho();
        void doVmg();
        void doPaste();
        void updateNxtVac();

    signals:
        void showMessage(QString msg);
        void sendCmd(int cmdNum,float,float,float);

    private:
        boatAccount * currentBoat;
        QTimer * timer;
        int nxtVac_cnt;
};

class boardVLM: public QWidget , public Ui::boardVLM_part1_ui
{ Q_OBJECT
    public:
        boardVLM(QMainWindow * mainWin,QWidget * parent=0);
        void boatUpdate(boatAccount * boat);
        void updateProxy(void);

        static QString pos2String(int type,float value);

        void showGribPointInfo(const GribPointInfo &pf);
        

    public slots:
        void chgHeading();
        void headingUpdated();
        void doVirer();
        void chgAngle();
        void angleUpdated();
        void doSynch();
        void synch_GPS();

        void requestFinished (QNetworkReply*);
        void sendCmd(int cmdNum,float val1,float val2, float val3);
        void chkResult(void);
        void setWP(float lat,float lon,float wph=-1);
        void paramChanged(void);

    signals:
        void showMessage(QString msg);

    private:
        QNetworkAccessManager *inetManager;
        QString host;

        int currentRequest;
        int currentCmdNum;
        float cmd_val1,cmd_val2,cmd_val3;
        int nbRetry;

        boatAccount * currentBoat;

        QTimer * timer;
        QTimer * GPS_timer;
		
        boardVLM_part2 * board2;

        /*GPS emul param*/
        QString COM;
};



#endif
