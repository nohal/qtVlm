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

#include <qextserialport.h>
#include <nmea.h>

#include "boat.h"
#include "class_list.h"

class ReceiverThread : public QThread
{ Q_OBJECT
    public:
        ReceiverThread(void);
        ~ReceiverThread(void);
        void run();

    public slots:
        void terminate();

    signals:
        void decodeData(QString data);

    private:
        QextSerialPort * port;
        bool stop;
};

class boatReal : public boat
{ Q_OBJECT
    public:
        boatReal(QString name, bool activated,
                 Projection * proj,MainWindow * main,myCentralWidget * parent);
        ~boatReal();
        void readData();
        int getVacLen(void) {return 5; }
        void unSelectBoat(bool needUpdate) { boat::unSelectBoat(needUpdate); }

    public slots:
        void decodeData(QString data);
        void slot_selectBoat(void) { boat::slot_selectBoat(); }

    signals:
        void terminateThread();

    private:
        ReceiverThread * gpsReader;
        int cnt;

        nmeaINFO info;
        nmeaPARSER parser;

        QString parseMask(int mask);

        void updateBoatName(void);
        void updateHint(void);
};






#endif // BOATREAL_H
