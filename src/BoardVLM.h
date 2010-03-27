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

#include "class_list.h"

#include "inetClient.h"

#include "ui_BoardVLM.h"
#include "ui_WP_dialog.h"

class WP_dialog: public QDialog, public Ui::WP_dialog_ui
{ Q_OBJECT
    public:
        WP_dialog(QWidget * parent=0);
        void show_WPdialog(boatAccount * boat);

    public slots:
        void chgLat();
        void chgLon();
        void doPaste();
        void doCopy();
        void done(int result);
        void doClearWP();
        void doSelPOI();
        void show_WPdialog(POI * poi);        

    signals:
        void sendCmd(int cmdNum,float,float,float);
        void selectPOI(void);

    private:
        boatAccount * currentBoat;
        void initDialog(float WPLat,float WPLon,float WPHd);
};

class boardVLM: public QWidget , public Ui::boardVLM_ui, public inetClient
{ Q_OBJECT
    public:
        boardVLM(MainWindow * mainWin, inetConnexion * inet);
        void validationDone(bool ok);

        /* inetClient */
        void requestFinished(QByteArray res);

    public slots:
        void chgHeading();
        void headingUpdated(double heading);
        void chgAngle();
        void angleUpdated(double angle);
        void doSync();
        void doVirer();
        void doPilotOrtho();
        void doVmg();
        void doVbvmg();
        void doWP_edit();
        void disp_boatInfo();
        void synch_GPS();

        void sendCmd(int cmdNum,float val1,float val2, float val3);
        void setWP(float lat,float lon,float wph=-1);
        void paramChanged(void);
        void setChangeStatus(bool);

        void edtSpinBox_key(void);

        void boatUpdated(boatAccount * boat);

    signals:
        void VLM_Sync(void);
        void POI_selectAborted(POI*);

    protected:
        void keyPressEvent ( QKeyEvent * event );

    private:
        QMainWindow * mainWin;

        int currentCmdNum;
        int nbRetry;
        bool isWaiting;

        float cmd_val1,cmd_val2,cmd_val3;
        float computeWPdir(boatAccount * boat);
        void update_btnWP(void);

        boatAccount * currentBoat;

        QTimer * GPS_timer;

        WP_dialog * wpDialog;

        QString default_styleSheet;

        /*GPS emul param*/
        QString COM;
        char chkSum(QString data);

        /* heading /angle user update */
        bool isComputing;
};



#endif
