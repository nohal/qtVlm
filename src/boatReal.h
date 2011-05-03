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

#ifndef BOATREAL_H
#define BOATREAL_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QGraphicsWidget>

#include <qextserialport.h>
#include <nmea.h>

#include "boat.h"
#include "vlmLine.h"
#include "class_list.h"
#include "mycentralwidget.h"

class ReceiverThread : public QThread
{ Q_OBJECT
    public:
        ReceiverThread(boatReal * parent);
        ~ReceiverThread(void);
        void run();

        bool initPort(void);
    public slots:
        void terminate();

    signals:
        void decodeData(QByteArray data);
        void updateBoat(boat * boat,bool b1, bool b2);

    private:
        QextSerialPort * port;
        boatReal *parent;
        bool stop;

};

class boatReal : public boat
{ Q_OBJECT
    public:
        boatReal(QString pseudo, bool activated,
                 Projection * proj,MainWindow * main,myCentralWidget * parent);
        ~boatReal();
        void stopRead();
        void startRead();
        void unSelectBoat(bool needUpdate);

        void updateAll(void);

        void paramUpdated();

        bool gpsIsRunning();
        void setPolar(QString polarName);

        void setPosition(double lat, double lon);

        /* event propagé par la scene */
        bool tryMoving(int x, int y);
        time_t getLastUpdateTime(){return this->lastUpdateTime;}
        int getFix(){return fix;}
        int getSig(){return sig;}
        double getPdop(){return pdop;}
        time_t getEta(){return eta;}
        void setWp(float lat, float lon, float wph);

    public slots:
        void decodeData(QByteArray data);
        void slot_selectBoat(void) { boat::slot_selectBoat(); }
        void slot_threadStartedOrFinished(void);
        void slot_chgPos(void);

    signals:
        void terminateThread();

    protected:
        void  mousePressEvent(QGraphicsSceneMouseEvent * e);
        void  mouseReleaseEvent(QGraphicsSceneMouseEvent * e);

    private:
        ReceiverThread * gpsReader;
        int cnt;

        nmeaINFO info;
        nmeaPARSER parser;

        QString parseMask(int mask);

        void updateBoatString(void);
        void updateHint(void);
        vlmLine *trace;
        double previousLon;
        double previousLat;
        bool gotPosition;

        void myCreatePopUpMenu(void);
        QAction * ac_chgPos;

        bool isMoving;
        int mouse_x,mouse_y;
        time_t lastUpdateTime;
        int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
        int     fix;        /**< Operating mode, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */
        double  pdop;
        time_t eta;
};






#endif // BOATREAL_H
